#pragma once

/// @file argu/core/parser.hpp
/// @brief Argument parsing implementation

#include <argu/config/config_parser.hpp>
#include <argu/core/arg.hpp>
#include <argu/core/command.hpp>
#include <argu/core/error.hpp>
#include <argu/core/levenshtein.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace argu {

    /// Internal parser implementation
    class Parser {
      public:
        explicit Parser(Command &cmd) : m_cmd(cmd) {}

        /// Parse command line arguments
        ParseResult parse(int argc, char *argv[]) {
            std::vector<std::string> args;
            args.reserve(static_cast<std::size_t>(argc));

            // Skip program name (argv[0])
            for (int i = 1; i < argc; ++i) {
                args.emplace_back(argv[i]);
            }

            return parse(args);
        }

        /// Parse arguments from vector
        ParseResult parse(const std::vector<std::string> &args) {
            try {
                // Ensure help and version flags are added
                ensure_default_flags();

                // Load config file if specified
                apply_config_file(args);

                // Check environment variables first
                apply_env_defaults();

                // Parse the arguments
                parse_args(args);

                // Validate required arguments
                validate_required();

                // Validate constraints
                validate_constraints();

                // Validate argument groups
                validate_groups();

                // If in aggregate mode and we collected errors, throw them now
                if (m_cmd.get_error_mode() == ErrorMode::Aggregate && !m_aggregated_errors.empty()) {
                    throw m_aggregated_errors;
                }

                // Apply defaults for missing values
                apply_defaults();

                // Run callback if set
                if (m_cmd.m_callback) {
                    m_cmd.m_callback(m_cmd.m_matches);
                }

                return ParseResult();
            } catch (const HelpRequested &e) {
                return ParseResult(true, 0, e.message());
            } catch (const VersionRequested &e) {
                return ParseResult(true, 0, e.message());
            } catch (const CompletionRequested &e) {
                return ParseResult(true, 0, e.message());
            } catch (const AggregatedErrors &e) {
                return ParseResult(e);
            } catch (const Error &e) {
                return ParseResult(e);
            }
        }

      private:
        Command &m_cmd;
        std::size_t m_positional_index = 0;
        bool m_positionals_only = false;      // After seeing --
        AggregatedErrors m_aggregated_errors; // For ErrorMode::Aggregate

        /// Add an error (either throw immediately or collect for later)
        template <typename E> void add_error(E &&error) {
            if (m_cmd.get_error_mode() == ErrorMode::Aggregate) {
                m_aggregated_errors.add_error(std::forward<E>(error));
            } else {
                throw std::forward<E>(error);
            }
        }

        void ensure_default_flags() {
            // Check if help already exists
            bool has_help = false;
            bool has_version = false;

            for (const auto &arg : m_cmd.m_args) {
                if (arg.get_action() == ValueAction::Help)
                    has_help = true;
                if (arg.get_action() == ValueAction::Version)
                    has_version = true;
            }

            if (!has_help && !m_cmd.m_disable_help) {
                m_cmd.m_args.insert(m_cmd.m_args.begin(), Arg("help")
                                                              .short_name('h')
                                                              .long_name("help")
                                                              .help("Print help information")
                                                              .flag()
                                                              .action(ValueAction::Help));
            }

            if (!has_version && m_cmd.m_version && !m_cmd.m_disable_version) {
                m_cmd.m_args.insert(m_cmd.m_args.begin() + (has_help ? 0 : 1), Arg("version")
                                                                                   .short_name('V')
                                                                                   .long_name("version")
                                                                                   .help("Print version information")
                                                                                   .flag()
                                                                                   .action(ValueAction::Version));
            }
        }

        void apply_config_file(const std::vector<std::string> &args) {
            // First, apply layered config files (in order, lower priority first)
            for (const auto &layer : m_cmd.m_config_layers) {
                apply_single_config(layer.path, layer.required);
            }

            // Then apply CLI-specified or default config file (highest priority)
            if (!m_cmd.m_config_arg.empty()) {
                std::string config_path;

                // Look for config file argument
                for (std::size_t i = 0; i < args.size(); ++i) {
                    if (args[i] == "--" + m_cmd.m_config_arg && i + 1 < args.size()) {
                        config_path = args[i + 1];
                        break;
                    }
                    if (args[i].starts_with("--" + m_cmd.m_config_arg + "=")) {
                        config_path = args[i].substr(m_cmd.m_config_arg.size() + 3);
                        break;
                    }
                }

                // Use default if not specified
                if (config_path.empty() && !m_cmd.m_default_config_path.empty()) {
                    if (std::filesystem::exists(m_cmd.m_default_config_path)) {
                        config_path = m_cmd.m_default_config_path;
                    }
                }

                if (!config_path.empty()) {
                    apply_single_config(config_path, config_path != m_cmd.m_default_config_path);
                }
            }
        }

        void apply_single_config(const std::string &config_path, bool required) {
            if (!std::filesystem::exists(config_path)) {
                if (required) {
                    throw ConfigFileError(config_path, "file not found");
                }
                return;
            }

            try {
                auto config = ConfigParser::parse_file(config_path);

                for (const auto &arg : m_cmd.m_args) {
                    // Try arg name
                    std::optional<std::string> value = config.get(arg.name());

                    // Try long name
                    if (!value && arg.long_opt()) {
                        value = config.get(*arg.long_opt());
                    }

                    if (value) {
                        auto &match = m_cmd.m_matches.get_or_create_match(arg.name());
                        // Higher priority configs are processed last and override
                        match.values.clear();
                        match.values.push_back(*value);
                        match.occurrences = 1;
                        match.from_config = true;
                        match.source = ValueSource::ConfigFile;
                        arg.apply_value(*value);
                    }
                }
            } catch (const ConfigFileError &) {
                if (required) {
                    throw;
                }
            }
        }

        void apply_env_defaults() {
            for (const auto &arg : m_cmd.m_args) {
                // Try to get env var value from multiple sources (in priority order):
                // 1. Explicit env var name set on the argument
                // 2. Arg-level prefix + arg name
                // 3. Command-level prefix + arg name
                const char *env_val = nullptr;
                std::string env_var_used;

                // 1. Check explicit env var
                if (arg.get_env()) {
                    env_val = std::getenv(arg.get_env()->c_str());
                    if (env_val) {
                        env_var_used = *arg.get_env();
                    }
                }

                // 2. Check arg-level prefix + arg name (uppercase, underscores)
                if (!env_val && arg.get_env_prefix()) {
                    std::string auto_var = make_env_var_name(*arg.get_env_prefix(), arg.name());
                    env_val = std::getenv(auto_var.c_str());
                    if (env_val) {
                        env_var_used = auto_var;
                    }
                }

                // 3. Check command-level prefix + arg name
                if (!env_val && m_cmd.m_env_prefix) {
                    std::string auto_var = make_env_var_name(*m_cmd.m_env_prefix, arg.name());
                    env_val = std::getenv(auto_var.c_str());
                    if (env_val) {
                        env_var_used = auto_var;
                    }
                }

                if (env_val) {
                    auto &match = m_cmd.m_matches.get_or_create_match(arg.name());

                    // Handle value delimiters for multi-value args
                    if (arg.get_value_delimiter() && arg.get_action() == ValueAction::Append) {
                        std::vector<std::string> split_values = split_by_delimiter(env_val, *arg.get_value_delimiter());
                        for (const auto &val : split_values) {
                            std::string transformed = arg.apply_transformers(val);
                            match.values.push_back(transformed);
                            arg.append_value(transformed);
                        }
                        match.occurrences = split_values.size();
                    } else {
                        std::string transformed = arg.apply_transformers(env_val);
                        match.values.push_back(transformed);
                        match.occurrences = 1;
                        arg.apply_value(transformed);
                    }

                    match.from_env = true;
                    match.source = ValueSource::Environment;
                }
            }
        }

        /// Convert arg name to environment variable name
        /// E.g., "my-option" with prefix "MYAPP_" becomes "MYAPP_MY_OPTION"
        static std::string make_env_var_name(const std::string &prefix, const std::string &arg_name) {
            std::string result = prefix;
            for (char c : arg_name) {
                if (c == '-' || c == '.') {
                    result += '_';
                } else {
                    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                }
            }
            return result;
        }

        /// Find an argument by prefix match (for partial matching)
        /// Returns nullptr if no match or ambiguous
        const Arg *find_arg_by_prefix(const std::string &prefix) const {
            const Arg *match = nullptr;
            int match_count = 0;

            for (const auto &arg : m_cmd.m_args) {
                if (arg.long_opt() && arg.long_opt()->starts_with(prefix)) {
                    match = &arg;
                    ++match_count;
                }
                // Also check visible aliases
                for (const auto &alias : arg.get_visible_aliases()) {
                    if (alias.starts_with(prefix)) {
                        match = &arg;
                        ++match_count;
                    }
                }
            }

            // Return match only if unambiguous
            return (match_count == 1) ? match : nullptr;
        }

        /// Split a string by a delimiter
        static std::vector<std::string> split_by_delimiter(const std::string &str, char delim) {
            std::vector<std::string> result;
            std::string current;
            for (char c : str) {
                if (c == delim) {
                    if (!current.empty()) {
                        result.push_back(current);
                        current.clear();
                    }
                } else {
                    current += c;
                }
            }
            if (!current.empty()) {
                result.push_back(current);
            }
            return result;
        }

        void parse_args(const std::vector<std::string> &args) {
            for (std::size_t i = 0; i < args.size(); ++i) {
                const std::string &arg = args[i];

                if (m_positionals_only) {
                    handle_positional(arg);
                    continue;
                }

                if (arg == "--") {
                    m_positionals_only = true;
                    continue;
                }

                if (arg.starts_with("--")) {
                    i = handle_long_option(args, i);
                } else if (arg.starts_with("-") && arg.size() > 1) {
                    i = handle_short_option(args, i);
                } else {
                    // Check if it's a subcommand
                    if (auto *sub = m_cmd.find_subcommand(arg)) {
                        parse_subcommand(sub, args, i + 1);
                        return;
                    }

                    // Check if external subcommands are allowed
                    if (m_cmd.m_allow_external && !m_cmd.m_subcommands.empty()) {
                        // This might be an external subcommand - capture remaining args
                        handle_external_subcommand(arg, args, i);
                        return;
                    }

                    // Check for trailing var arg capture
                    if (m_cmd.m_trailing_var_arg) {
                        handle_trailing_args(args, i);
                        return;
                    }

                    handle_positional(arg);
                }
            }
        }

        std::size_t handle_long_option(const std::vector<std::string> &args, std::size_t i) {
            std::string arg = args[i].substr(2); // Remove --
            std::string value;
            bool has_inline_value = false;

            // Check for --option=value syntax
            auto eq_pos = arg.find('=');
            if (eq_pos != std::string::npos) {
                value = arg.substr(eq_pos + 1);
                arg = arg.substr(0, eq_pos);
                has_inline_value = true;
            }

            const Arg *opt = m_cmd.find_arg_by_long(arg);

            // Try partial matching if enabled and not found
            if (!opt && m_cmd.m_allow_partial_matching) {
                opt = find_arg_by_prefix(arg);
            }

            if (!opt) {
                // Try to suggest similar options
                auto suggestions =
                    detail::find_closest_matches(arg, m_cmd.get_long_options(), m_cmd.get_suggest_threshold());
                throw UnknownArgumentError("--" + arg, [&suggestions]() {
                    std::vector<std::string> result;
                    for (const auto &s : suggestions)
                        result.push_back("--" + s);
                    return result;
                }());
            }

            // Handle special actions
            if (opt->get_action() == ValueAction::Help) {
                throw HelpRequested(m_cmd.help());
            }
            if (opt->get_action() == ValueAction::Version) {
                throw VersionRequested(m_cmd.version_string());
            }

            auto &match = m_cmd.m_matches.get_or_create_match(opt->name());

            // Command line takes precedence - clear env/config values if "last wins" or first CLI occurrence
            bool should_override = (match.source != ValueSource::CommandLine) ||
                                   (opt->is_last_wins() || m_cmd.m_conflict_mode == ConflictMode::LastWins);

            if (should_override && match.source != ValueSource::CommandLine) {
                // First CLI value overrides env/config
                match.values.clear();
                match.occurrences = 0;
            }

            match.occurrences++;
            match.source = ValueSource::CommandLine;

            // Handle negatable flags
            if (opt->is_negatable() && opt->is_negated_match(arg)) {
                opt->apply_flag(false);
                return i;
            }

            if (opt->is_flag()) {
                opt->apply_flag(opt->get_action() == ValueAction::StoreTrue);
                return i;
            }

            if (opt->is_count()) {
                opt->apply_count(static_cast<int>(match.occurrences));
                return i;
            }

            // Get values
            if (has_inline_value) {
                // Handle value delimiter for multi-value in single arg
                if (opt->get_value_delimiter()) {
                    auto split_values = split_by_delimiter(value, *opt->get_value_delimiter());
                    for (const auto &v : split_values) {
                        std::string transformed = opt->apply_transformers(v);
                        match.values.push_back(transformed);
                        if (opt->get_action() == ValueAction::Append) {
                            opt->append_value(transformed);
                        }
                    }
                    if (!match.values.empty() && opt->get_action() != ValueAction::Append) {
                        opt->apply_value(match.values.back());
                    }
                } else {
                    std::string transformed = opt->apply_transformers(value);
                    match.values.push_back(transformed);
                    if (opt->get_action() == ValueAction::Append) {
                        opt->append_value(transformed);
                    } else {
                        opt->apply_value(transformed);
                    }
                }
            } else {
                // Collect required values
                std::size_t values_needed = opt->value_count().min;
                std::size_t values_max = opt->value_count().max;
                std::size_t collected = 0;

                while (collected < values_max && i + 1 < args.size()) {
                    const std::string &next = args[i + 1];
                    // Allow values starting with - if:
                    // 1. allow_negative_numbers is true (for -42)
                    // 2. allow_hyphen_values is true (for arbitrary - values)
                    if (next.starts_with("-") && !m_cmd.m_allow_negative && !m_cmd.m_allow_hyphen_values) {
                        break;
                    }
                    ++i;
                    std::string transformed = opt->apply_transformers(next);
                    match.values.push_back(transformed);
                    if (opt->get_action() == ValueAction::Append) {
                        opt->append_value(transformed);
                    }
                    ++collected;
                }

                // If no values collected but has implicit value, use it
                if (collected == 0 && opt->get_implicit_value()) {
                    std::string transformed = opt->apply_transformers(*opt->get_implicit_value());
                    match.values.push_back(transformed);
                    opt->apply_value(transformed);
                    return i;
                }

                if (collected < values_needed) {
                    throw MissingValueError(opt->name());
                }

                if (!match.values.empty() && opt->get_action() != ValueAction::Append) {
                    opt->apply_value(match.values.back());
                }
            }

            // Validate choices
            validate_choices(*opt, match.values);

            // Run validators
            validate_value(*opt, match.values);

            return i;
        }

        std::size_t handle_short_option(const std::vector<std::string> &args, std::size_t i) {
            const std::string &arg = args[i];

            // Handle combined short options like -abc
            for (std::size_t j = 1; j < arg.size(); ++j) {
                char c = arg[j];
                const Arg *opt = m_cmd.find_arg_by_short(c);

                if (!opt) {
                    // Try to suggest
                    auto suggestions = detail::find_closest_matches(std::string(1, c), m_cmd.get_short_options(), 1);
                    std::vector<std::string> formatted;
                    for (const auto &s : suggestions)
                        formatted.push_back("-" + s);
                    throw UnknownArgumentError(std::string("-") + c, formatted);
                }

                // Handle special actions
                if (opt->get_action() == ValueAction::Help) {
                    throw HelpRequested(m_cmd.help());
                }
                if (opt->get_action() == ValueAction::Version) {
                    throw VersionRequested(m_cmd.version_string());
                }

                auto &match = m_cmd.m_matches.get_or_create_match(opt->name());

                // Command line takes precedence
                if (match.source != ValueSource::CommandLine) {
                    match.values.clear();
                    match.occurrences = 0;
                }

                match.occurrences++;
                match.source = ValueSource::CommandLine;

                if (opt->is_flag()) {
                    opt->apply_flag(opt->get_action() == ValueAction::StoreTrue);
                    continue;
                }

                if (opt->is_count()) {
                    opt->apply_count(static_cast<int>(match.occurrences));
                    continue;
                }

                // Option takes a value
                std::string value;

                // Check if remaining chars are the value: -oVALUE
                if (j + 1 < arg.size()) {
                    value = arg.substr(j + 1);
                    j = arg.size(); // Exit loop
                } else if (i + 1 < args.size()) {
                    // Next arg is the value
                    ++i;
                    value = args[i];
                } else {
                    throw MissingValueError(opt->name());
                }

                std::string transformed = opt->apply_transformers(value);
                match.values.push_back(transformed);
                if (opt->get_action() == ValueAction::Append) {
                    opt->append_value(transformed);
                } else {
                    opt->apply_value(transformed);
                }

                validate_choices(*opt, match.values);
                validate_value(*opt, match.values);
            }

            return i;
        }

        void handle_positional(const std::string &value) {
            // Find positional arguments in order
            std::vector<const Arg *> positionals;
            for (const auto &arg : m_cmd.m_args) {
                if (arg.is_positional()) {
                    positionals.push_back(&arg);
                }
            }

            // Sort by index if specified
            std::sort(positionals.begin(), positionals.end(), [](const Arg *a, const Arg *b) {
                auto idx_a = a->get_index().value_or(SIZE_MAX);
                auto idx_b = b->get_index().value_or(SIZE_MAX);
                return idx_a < idx_b;
            });

            if (m_positional_index >= positionals.size()) {
                // Check if last positional takes multiple values
                if (!positionals.empty()) {
                    const Arg *last = positionals.back();
                    if (last->value_count().max > 1 || last->value_count().max == ValueCount::unlimited) {
                        auto &match = m_cmd.m_matches.get_or_create_match(last->name());
                        std::string transformed = last->apply_transformers(value);
                        match.values.push_back(transformed);
                        match.occurrences++;
                        if (last->get_action() == ValueAction::Append) {
                            last->append_value(transformed);
                        } else {
                            last->apply_value(transformed);
                        }
                        return;
                    }
                }

                // Check if it might be a mistyped subcommand
                auto subcommand_names = m_cmd.get_subcommand_names(false); // Exclude hidden
                auto suggestions = detail::find_closest_matches(value, subcommand_names, m_cmd.get_suggest_threshold());
                if (!suggestions.empty()) {
                    throw UnknownSubcommandError(value, suggestions);
                }

                throw TooManyArgumentsError(value);
            }

            const Arg *pos = positionals[m_positional_index];
            auto &match = m_cmd.m_matches.get_or_create_match(pos->name());
            std::string transformed = pos->apply_transformers(value);
            match.values.push_back(transformed);
            match.occurrences++;

            if (pos->get_action() == ValueAction::Append) {
                pos->append_value(transformed);
            } else {
                pos->apply_value(transformed);
            }

            validate_choices(*pos, match.values);
            validate_value(*pos, match.values);

            // Move to next positional if this one is satisfied
            if (match.values.size() >= pos->value_count().max) {
                ++m_positional_index;
            }
        }

        void parse_subcommand(Command *sub, const std::vector<std::string> &args, std::size_t start) {
            // Propagate version if enabled
            if (m_cmd.m_propagate_version && m_cmd.m_version && !sub->m_version) {
                sub->m_version = m_cmd.m_version;
            }

            // Copy global args
            for (const auto &arg : m_cmd.m_args) {
                if (arg.is_global()) {
                    sub->m_args.push_back(arg);
                }
            }

            // Create sub-args
            std::vector<std::string> sub_args(args.begin() + static_cast<std::ptrdiff_t>(start), args.end());

            // Parse subcommand
            Parser sub_parser(*sub);
            auto result = sub_parser.parse(sub_args);

            // Store subcommand info
            m_cmd.m_matches.m_subcommand = sub->name();
            m_cmd.m_matches.m_subcommand_matches = std::make_unique<Matches>(std::move(sub->m_matches));

            // Handle help/version from subcommand (exit_code 0 with message)
            if (result.exit_code() == 0 && !result.message().empty()) {
                throw HelpRequested(result.message());
            }

            if (!result.success()) {
                throw Error(result.message(), static_cast<ExitCode>(result.exit_code()));
            }
        }

        void handle_external_subcommand(const std::string &subcommand, const std::vector<std::string> &args,
                                        std::size_t start) {
            // Store the external subcommand name and remaining args
            m_cmd.m_matches.m_subcommand = subcommand;
            m_cmd.m_matches.m_external_subcommand = true;

            // Capture all remaining args as external args
            std::vector<std::string> external_args;
            for (std::size_t i = start + 1; i < args.size(); ++i) {
                external_args.push_back(args[i]);
            }
            m_cmd.m_matches.m_external_args = std::move(external_args);
        }

        void handle_trailing_args(const std::vector<std::string> &args, std::size_t start) {
            // Capture all remaining args as trailing args
            auto &match = m_cmd.m_matches.get_or_create_match(m_cmd.m_trailing_values_name);
            for (std::size_t i = start; i < args.size(); ++i) {
                match.values.push_back(args[i]);
                match.occurrences++;
            }
            match.source = ValueSource::CommandLine;
        }

        void validate_required() {
            for (const auto &arg : m_cmd.m_args) {
                bool arg_present = m_cmd.m_matches.contains(arg.name());
                bool arg_required = arg.is_required();

                // Check required_unless: arg is required unless one of the listed args is present
                if (!arg_present && !arg.get_required_unless().empty()) {
                    bool any_unless_present = false;
                    for (const auto &unless_arg : arg.get_required_unless()) {
                        if (m_cmd.m_matches.contains(unless_arg)) {
                            any_unless_present = true;
                            break;
                        }
                    }
                    if (!any_unless_present && !arg.default_val()) {
                        add_error(MissingRequiredError(arg.name()));
                        continue;
                    }
                }

                // Check required_if_eq: arg is required if another arg equals specific value
                if (!arg_present) {
                    for (const auto &[other_arg, other_value] : arg.get_required_if_eq()) {
                        auto match_value = m_cmd.m_matches.get_one(other_arg);
                        if (match_value && *match_value == other_value) {
                            if (!arg.default_val()) {
                                add_error(MissingRequiredError(arg.name()));
                                break;
                            }
                        }
                    }
                }

                // Check basic required
                if (arg_required && !arg_present) {
                    if (!arg.default_val()) {
                        add_error(MissingRequiredError(arg.name()));
                    }
                }
            }

            // Check if subcommand is required
            if (m_cmd.m_subcommand_required && !m_cmd.m_matches.m_subcommand && !m_cmd.m_subcommands.empty()) {
                std::vector<std::string> available;
                for (const auto &sub : m_cmd.m_subcommands) {
                    if (!sub->is_hidden()) {
                        available.push_back(sub->name());
                    }
                }
                add_error(MissingSubcommandError(available));
            }
        }

        void validate_constraints() {
            for (const auto &arg : m_cmd.m_args) {
                if (!m_cmd.m_matches.contains(arg.name()))
                    continue;

                // Check conflicts
                for (const auto &conflict : arg.get_conflicts()) {
                    if (m_cmd.m_matches.contains(conflict)) {
                        add_error(ConflictError(arg.name(), conflict));
                    }
                }

                // Check requires
                for (const auto &req : arg.get_requires()) {
                    if (!m_cmd.m_matches.contains(req)) {
                        add_error(DependencyError(arg.name(), req));
                    }
                }

                // Check requires_if: this arg requires another arg if it has specific value
                for (const auto &[req_arg, req_value] : arg.get_requires_if()) {
                    auto this_value = m_cmd.m_matches.get_one(arg.name());
                    if (this_value && *this_value == req_value) {
                        if (!m_cmd.m_matches.contains(req_arg)) {
                            add_error(DependencyError(arg.name(), req_arg));
                        }
                    }
                }
            }
        }

        void validate_groups() {
            for (const auto &group : m_cmd.m_groups) {
                std::vector<std::string> present;
                std::vector<std::string> missing;

                for (const auto &arg_name : group.get_args()) {
                    if (m_cmd.m_matches.contains(arg_name)) {
                        present.push_back(arg_name);
                    } else {
                        missing.push_back(arg_name);
                    }
                }

                switch (group.get_type()) {
                case GroupType::MutuallyExclusive:
                    if (present.size() > 1) {
                        add_error(MutexGroupError(group.get_name(), present));
                    }
                    break;

                case GroupType::RequiredTogether:
                    if (!present.empty() && !missing.empty()) {
                        add_error(RequiredTogetherError(group.get_name(), present, missing));
                    }
                    break;

                case GroupType::AtLeastOne:
                    if (group.is_required() && present.empty()) {
                        add_error(AtLeastOneRequiredError(group.get_name(), group.get_args()));
                    }
                    break;

                default:
                    break;
                }
            }
        }

        void validate_choices(const Arg &arg, const std::vector<std::string> &values) {
            const auto &choices = arg.get_choices();
            if (choices.empty())
                return;

            for (const auto &val : values) {
                if (std::find(choices.begin(), choices.end(), val) == choices.end()) {
                    std::string valid;
                    for (std::size_t i = 0; i < choices.size(); ++i) {
                        if (i > 0)
                            valid += ", ";
                        valid += choices[i];
                    }
                    throw InvalidValueError(arg.name(), val, "valid values are: " + valid);
                }
            }
        }

        void validate_value(const Arg &arg, const std::vector<std::string> &values) {
            for (const auto &validator : arg.get_validators()) {
                for (const auto &val : values) {
                    auto error = validator(val);
                    if (error) {
                        throw ValidationError(arg.name(), *error);
                    }
                }
            }
        }

        void apply_defaults() {
            for (const auto &arg : m_cmd.m_args) {
                if (m_cmd.m_matches.contains(arg.name()))
                    continue;

                // Check default_value_if: conditional default based on another arg's value
                bool conditional_default_applied = false;
                for (const auto &[other_arg, other_value, default_val] : arg.get_default_value_if()) {
                    auto match_value = m_cmd.m_matches.get_one(other_arg);
                    if (match_value && *match_value == other_value) {
                        auto &match = m_cmd.m_matches.get_or_create_match(arg.name());
                        match.values.push_back(default_val);
                        match.occurrences = 1;
                        match.source = ValueSource::Default;
                        arg.apply_value(default_val);
                        conditional_default_applied = true;
                        break;
                    }
                }

                // Apply regular default if no conditional default was applied
                if (!conditional_default_applied && arg.default_val()) {
                    arg.apply_default();
                    auto &match = m_cmd.m_matches.get_or_create_match(arg.name());
                    match.values.push_back(*arg.default_val());
                    match.occurrences = 1;
                }
            }
        }
    };

    // Implementation of Command::parse methods
    inline ParseResult Command::parse(int argc, char *argv[]) {
        Parser parser(*this);
        return parser.parse(argc, argv);
    }

    inline ParseResult Command::parse(const std::vector<std::string> &args) {
        Parser parser(*this);
        return parser.parse(args);
    }

} // namespace argu
