#pragma once

/// @file parser.hpp
/// @brief Argument parsing implementation

#include <argu/arg.hpp>
#include <argu/command.hpp>
#include <argu/error.hpp>

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

                // Check environment variables first
                apply_env_defaults();

                // Parse the arguments
                parse_args(args);

                // Validate required arguments
                validate_required();

                // Validate constraints
                validate_constraints();

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
            } catch (const Error &e) {
                return ParseResult(e);
            }
        }

      private:
        Command &m_cmd;
        std::size_t m_positional_index = 0;
        bool m_positionals_only = false; // After seeing --

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

        void apply_env_defaults() {
            for (const auto &arg : m_cmd.m_args) {
                if (arg.get_env()) {
                    const char *env_val = std::getenv(arg.get_env()->c_str());
                    if (env_val) {
                        auto &match = m_cmd.m_matches.get_or_create_match(arg.name());
                        match.values.push_back(env_val);
                        match.occurrences = 1;
                        arg.apply_value(env_val);
                    }
                }
            }
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
            if (!opt) {
                throw UnknownArgumentError("--" + arg);
            }

            // Handle special actions
            if (opt->get_action() == ValueAction::Help) {
                throw HelpRequested(m_cmd.help());
            }
            if (opt->get_action() == ValueAction::Version) {
                throw VersionRequested(m_cmd.version_string());
            }

            auto &match = m_cmd.m_matches.get_or_create_match(opt->name());
            match.occurrences++;

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
                match.values.push_back(value);
                if (opt->get_action() == ValueAction::Append) {
                    opt->append_value(value);
                } else {
                    opt->apply_value(value);
                }
            } else {
                // Collect required values
                std::size_t values_needed = opt->value_count().min;
                std::size_t values_max = opt->value_count().max;
                std::size_t collected = 0;

                while (collected < values_max && i + 1 < args.size()) {
                    const std::string &next = args[i + 1];
                    if (next.starts_with("-") && !m_cmd.m_allow_negative) {
                        break;
                    }
                    ++i;
                    match.values.push_back(next);
                    if (opt->get_action() == ValueAction::Append) {
                        opt->append_value(next);
                    }
                    ++collected;
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
                    throw UnknownArgumentError(std::string("-") + c);
                }

                // Handle special actions
                if (opt->get_action() == ValueAction::Help) {
                    throw HelpRequested(m_cmd.help());
                }
                if (opt->get_action() == ValueAction::Version) {
                    throw VersionRequested(m_cmd.version_string());
                }

                auto &match = m_cmd.m_matches.get_or_create_match(opt->name());
                match.occurrences++;

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

                match.values.push_back(value);
                if (opt->get_action() == ValueAction::Append) {
                    opt->append_value(value);
                } else {
                    opt->apply_value(value);
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
                    if (last->value_count().max > 1 || last->value_count().max == SIZE_MAX) {
                        auto &match = m_cmd.m_matches.get_or_create_match(last->name());
                        match.values.push_back(value);
                        match.occurrences++;
                        if (last->get_action() == ValueAction::Append) {
                            last->append_value(value);
                        } else {
                            last->apply_value(value);
                        }
                        return;
                    }
                }
                throw TooManyArgumentsError(value);
            }

            const Arg *pos = positionals[m_positional_index];
            auto &match = m_cmd.m_matches.get_or_create_match(pos->name());
            match.values.push_back(value);
            match.occurrences++;

            if (pos->get_action() == ValueAction::Append) {
                pos->append_value(value);
            } else {
                pos->apply_value(value);
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

        void validate_required() {
            for (const auto &arg : m_cmd.m_args) {
                if (arg.is_required() && !m_cmd.m_matches.contains(arg.name())) {
                    // Check if default was applied
                    if (!arg.default_val()) {
                        throw MissingRequiredError(arg.name());
                    }
                }
            }

            // Check if subcommand is required
            if (m_cmd.m_subcommand_required && !m_cmd.m_matches.m_subcommand && !m_cmd.m_subcommands.empty()) {
                throw MissingSubcommandError();
            }
        }

        void validate_constraints() {
            for (const auto &arg : m_cmd.m_args) {
                if (!m_cmd.m_matches.contains(arg.name()))
                    continue;

                // Check conflicts
                for (const auto &conflict : arg.get_conflicts()) {
                    if (m_cmd.m_matches.contains(conflict)) {
                        throw ConflictError(arg.name(), conflict);
                    }
                }

                // Check requires
                for (const auto &req : arg.get_requires()) {
                    if (!m_cmd.m_matches.contains(req)) {
                        throw MissingRequiredError(req + " (required by " + arg.name() + ")");
                    }
                }
            }
        }

        void validate_choices(const Arg &arg, const std::vector<std::string> &values) {
            const auto &choices = arg.get_choices();
            if (choices.empty())
                return;

            for (const auto &val : values) {
                if (std::find(choices.begin(), choices.end(), val) == choices.end()) {
                    std::string valid = "";
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
                if (!m_cmd.m_matches.contains(arg.name()) && arg.default_val()) {
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
