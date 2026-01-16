#pragma once

/// @file argu/core/command.hpp
/// @brief Command and subcommand definitions (CLAP-style)

#include <argu/core/arg.hpp>
#include <argu/core/error.hpp>
#include <argu/core/group.hpp>
#include <argu/core/types.hpp>
#include <argu/style/colors.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace argu {

    /// Forward declaration
    class Parser;

    /// Parse result that can be used for easy error handling
    class ParseResult {
      public:
        ParseResult() : m_success(true), m_exit_code(0) {}
        explicit ParseResult(const Error &error)
            : m_success(false), m_exit_code(error.exit_code_int()), m_message(error.message()) {}
        ParseResult(bool success, int code, std::string msg = "")
            : m_success(success), m_exit_code(code), m_message(std::move(msg)) {}

        /// Check if parsing succeeded
        explicit operator bool() const { return m_success; }
        bool success() const { return m_success; }

        /// Get exit code (0 for success, non-zero for errors)
        int exit_code() const { return m_exit_code; }

        /// Get error message
        const std::string &message() const { return m_message; }

        /// Print error to stderr and return exit code
        int exit() const {
            if (!m_success && !m_message.empty()) {
                std::cerr << m_message << std::endl;
            } else if (m_success && !m_message.empty()) {
                // Help or version output goes to stdout
                std::cout << m_message << std::endl;
            }
            return m_exit_code;
        }

      private:
        bool m_success;
        int m_exit_code;
        std::string m_message;
    };

    /// Matches - result of parsing containing matched arguments
    class Matches {
        friend class Parser;

      public:
        Matches() = default;
        ~Matches() = default;

        // Move constructor and assignment
        Matches(Matches &&other) noexcept = default;
        Matches &operator=(Matches &&other) noexcept = default;

        // Copy constructor - deep copies subcommand matches
        Matches(const Matches &other)
            : m_matches(other.m_matches), m_subcommand(other.m_subcommand),
              m_external_subcommand(other.m_external_subcommand), m_external_args(other.m_external_args) {
            if (other.m_subcommand_matches) {
                m_subcommand_matches = std::make_unique<Matches>(*other.m_subcommand_matches);
            }
        }

        // Copy assignment
        Matches &operator=(const Matches &other) {
            if (this != &other) {
                m_matches = other.m_matches;
                m_subcommand = other.m_subcommand;
                m_external_subcommand = other.m_external_subcommand;
                m_external_args = other.m_external_args;
                if (other.m_subcommand_matches) {
                    m_subcommand_matches = std::make_unique<Matches>(*other.m_subcommand_matches);
                } else {
                    m_subcommand_matches.reset();
                }
            }
            return *this;
        }

        /// Check if an argument was present
        bool contains(const std::string &name) const {
            return std::any_of(m_matches.begin(), m_matches.end(),
                               [&](const ArgMatch &m) { return m.name == name && m.occurrences > 0; });
        }

        /// Get number of occurrences of an argument
        std::size_t occurrences(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            return it != m_matches.end() ? it->occurrences : 0;
        }

        /// Get single value as string
        std::optional<std::string> get_one(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            if (it != m_matches.end() && !it->values.empty()) {
                return it->values.front();
            }
            return std::nullopt;
        }

        /// Get all values as strings
        std::vector<std::string> get_many(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            if (it != m_matches.end()) {
                return it->values;
            }
            return {};
        }

        /// Get value converted to type T
        template <typename T> std::optional<T> get(const std::string &name) const {
            auto str = get_one(name);
            if (!str)
                return std::nullopt;
            return detail::Converter<T>::convert(*str);
        }

        /// Get value with default
        template <typename T> T get_or(const std::string &name, const T &default_value) const {
            auto result = get<T>(name);
            return result.value_or(default_value);
        }

        /// Get flag value (true if present)
        bool get_flag(const std::string &name) const { return contains(name); }

        /// Get count value
        std::size_t get_count(const std::string &name) const { return occurrences(name); }

        /// Get subcommand name if any was matched
        std::optional<std::string> subcommand() const { return m_subcommand; }

        /// Get subcommand matches
        const Matches *subcommand_matches() const { return m_subcommand_matches.get(); }

        /// Get subcommand matches for a specific subcommand
        const Matches *subcommand_matches(const std::string &name) const {
            if (m_subcommand && *m_subcommand == name) {
                return m_subcommand_matches.get();
            }
            return nullptr;
        }

        /// Get the full subcommand chain (e.g., ["remote", "add"] for "git remote add")
        std::vector<std::string> subcommand_chain() const {
            std::vector<std::string> chain;
            if (m_subcommand) {
                chain.push_back(*m_subcommand);
                if (m_subcommand_matches) {
                    auto sub_chain = m_subcommand_matches->subcommand_chain();
                    chain.insert(chain.end(), sub_chain.begin(), sub_chain.end());
                }
            }
            return chain;
        }

        /// Check if an external (unknown) subcommand was matched
        bool is_external_subcommand() const { return m_external_subcommand; }

        /// Get external subcommand arguments (args passed after the external subcommand)
        const std::vector<std::string> &external_args() const { return m_external_args; }

        /// Check if value came from environment variable
        bool is_from_env(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            return it != m_matches.end() && it->from_env;
        }

        /// Check if value came from config file
        bool is_from_config(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            return it != m_matches.end() && it->from_config;
        }

        /// Get the source of a value (precedence tracking)
        ValueSource value_source(const std::string &name) const {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            if (it != m_matches.end()) {
                return it->source;
            }
            return ValueSource::Default;
        }

      private:
        std::vector<ArgMatch> m_matches;
        std::optional<std::string> m_subcommand;
        std::unique_ptr<Matches> m_subcommand_matches;
        bool m_external_subcommand = false;
        std::vector<std::string> m_external_args;

        void add_match(ArgMatch match) { m_matches.push_back(std::move(match)); }

        ArgMatch *find_match(const std::string &name) {
            auto it =
                std::find_if(m_matches.begin(), m_matches.end(), [&](const ArgMatch &m) { return m.name == name; });
            return it != m_matches.end() ? &(*it) : nullptr;
        }

        ArgMatch &get_or_create_match(const std::string &name) {
            if (auto *m = find_match(name)) {
                return *m;
            }
            m_matches.push_back(ArgMatch{name, {}, 0, false, false});
            return m_matches.back();
        }
    };

    /// Command builder class (represents a command or subcommand)
    class Command {
        friend class Parser;

      public:
        /// Construct command with name
        explicit Command(std::string name) : m_name(std::move(name)) {}

        /// Set version string
        Command &version(std::string ver) {
            m_version = std::move(ver);
            return *this;
        }

        /// Set description (short)
        Command &about(std::string text) {
            m_about = std::move(text);
            return *this;
        }

        /// Set long description
        Command &long_about(std::string text) {
            m_long_about = std::move(text);
            return *this;
        }

        /// Set author info
        Command &author(std::string name) {
            m_author = std::move(name);
            return *this;
        }

        /// Set usage override
        Command &override_usage(std::string usage) {
            m_usage_override = std::move(usage);
            return *this;
        }

        /// Set help template
        Command &help_template(std::string tmpl) {
            m_help_template = std::move(tmpl);
            return *this;
        }

        /// Add an argument
        Command &arg(Arg argument) {
            // Auto-add help flag if this is the first arg and none exists
            if (m_args.empty() && !m_disable_help) {
                add_default_help();
            }
            m_args.push_back(std::move(argument));
            return *this;
        }

        /// Add multiple arguments
        Command &args(std::initializer_list<Arg> arguments) {
            for (auto &a : arguments) {
                arg(std::move(const_cast<Arg &>(a)));
            }
            return *this;
        }

        /// Add an argument group
        Command &group(ArgGroup grp) {
            m_groups.push_back(std::move(grp));
            return *this;
        }

        /// Add a subcommand
        Command &subcommand(Command cmd) {
            m_subcommands.push_back(std::make_shared<Command>(std::move(cmd)));
            return *this;
        }

        /// Require a subcommand
        Command &subcommand_required(bool required = true) {
            m_subcommand_required = required;
            return *this;
        }

        /// Allow external subcommands
        Command &allow_external_subcommands(bool allow = true) {
            m_allow_external = allow;
            return *this;
        }

        /// Disable help flag
        Command &disable_help_flag(bool disable = true) {
            m_disable_help = disable;
            return *this;
        }

        /// Disable version flag
        Command &disable_version_flag(bool disable = true) {
            m_disable_version = disable;
            return *this;
        }

        /// Set to propagate version to subcommands
        Command &propagate_version(bool propagate = true) {
            m_propagate_version = propagate;
            return *this;
        }

        /// Allow negative numbers as values
        Command &allow_negative_numbers(bool allow = true) {
            m_allow_negative = allow;
            return *this;
        }

        /// Set argument display order
        Command &next_display_order(int order) {
            m_display_order = order;
            return *this;
        }

        /// Set color mode for help
        Command &color(ColorMode mode) {
            m_color_mode = mode;
            return *this;
        }

        /// Set help theme
        Command &theme(HelpTheme t) {
            m_theme = std::move(t);
            return *this;
        }

        /// Add alias for command
        Command &alias(std::string name) {
            m_aliases.push_back(std::move(name));
            return *this;
        }

        /// Visible alias
        Command &visible_alias(std::string name) {
            m_visible_aliases.push_back(std::move(name));
            return *this;
        }

        /// Hide command from help
        Command &hidden(bool hide = true) {
            m_hidden = hide;
            return *this;
        }

        /// Set before_help text
        Command &before_help(std::string text) {
            m_before_help = std::move(text);
            return *this;
        }

        /// Set after_help text
        Command &after_help(std::string text) {
            m_after_help = std::move(text);
            return *this;
        }

        /// Set after_long_help text
        Command &after_long_help(std::string text) {
            m_after_long_help = std::move(text);
            return *this;
        }

        /// Add a custom help section with title and content
        Command &help_section(std::string title, std::string content, int order = 100) {
            m_custom_sections.push_back({std::move(title), std::move(content), order});
            return *this;
        }

        /// Set argument group ordering (lower numbers appear first)
        Command &group_order(const std::string &group_name, int order) {
            m_group_order[group_name] = order;
            return *this;
        }

        /// Enable config file support
        Command &config_file(std::string config_arg_name, std::string default_path = "") {
            m_config_arg = std::move(config_arg_name);
            m_default_config_path = std::move(default_path);
            return *this;
        }

        /// Add a config file to the layered config (lower priority files first)
        /// Files are loaded in order: system -> user -> project -> CLI
        Command &add_config_layer(std::string path, bool required = false) {
            m_config_layers.push_back({std::move(path), required});
            return *this;
        }

        /// Set multiple config layers at once (in priority order, lowest first)
        Command &config_layers(std::initializer_list<std::string> paths) {
            for (const auto &path : paths) {
                m_config_layers.push_back({path, false});
            }
            return *this;
        }

        /// Enable shell completion argument
        Command &enable_completions(std::string arg_name = "completions") {
            m_completion_arg = std::move(arg_name);
            return *this;
        }

        /// Set suggest threshold for typo suggestions (Levenshtein distance)
        Command &suggest_threshold(std::size_t threshold) {
            m_suggest_threshold = threshold;
            return *this;
        }

        /// Set callback to run when this command is matched
        Command &callback(std::function<void(const Matches &)> cb) {
            m_callback = std::move(cb);
            return *this;
        }

        /// Set pre-parse callback (runs before parsing starts)
        Command &pre_parse(std::function<void()> cb) {
            m_pre_parse = std::move(cb);
            return *this;
        }

        /// Set parse-complete callback (runs after parsing, before validation)
        Command &parse_complete(std::function<void(Matches &)> cb) {
            m_parse_complete = std::move(cb);
            return *this;
        }

        /// Set final callback (runs last, after all validation)
        Command &final_callback(std::function<void(const Matches &)> cb) {
            m_final_callback = std::move(cb);
            return *this;
        }

        /// Set conflict resolution mode
        Command &conflict_mode(ConflictMode mode) {
            m_conflict_mode = mode;
            return *this;
        }

        /// Allow POSIX-style "last wins" for conflicts
        Command &allow_overrides(bool allow = true) {
            m_conflict_mode = allow ? ConflictMode::LastWins : ConflictMode::Error;
            return *this;
        }

        /// Enable partial/prefix matching for long options
        Command &allow_partial_matching(bool allow = true) {
            m_allow_partial_matching = allow;
            return *this;
        }

        /// Enable partial/prefix matching for subcommands
        Command &allow_subcommand_prefix(bool allow = true) {
            m_allow_subcommand_prefix = allow;
            return *this;
        }

        /// Set deprecation warning stream (default: stderr)
        Command &deprecation_stream(std::ostream *stream) {
            m_deprecation_stream = stream;
            return *this;
        }

        /// Set environment variable prefix for all args (e.g., "MYAPP_")
        Command &env_prefix(std::string prefix) {
            m_env_prefix = std::move(prefix);
            return *this;
        }

        /// Capture external subcommand arguments
        Command &trailing_var_arg(bool capture = true) {
            m_trailing_var_arg = capture;
            return *this;
        }

        /// Set the name for captured trailing arguments
        Command &trailing_values_name(std::string name) {
            m_trailing_values_name = std::move(name);
            return *this;
        }

        /// Allow hyphen values (arguments starting with -)
        Command &allow_hyphen_values(bool allow = true) {
            m_allow_hyphen_values = allow;
            return *this;
        }

        /// Ignore errors and continue parsing
        Command &ignore_errors(bool ignore = true) {
            m_ignore_errors = ignore;
            return *this;
        }

        /// Set error handling mode (FirstError or Aggregate)
        Command &error_mode(ErrorMode mode) {
            m_error_mode = mode;
            return *this;
        }

        /// Display help on error
        Command &help_on_error(bool show = true) {
            m_help_on_error = show;
            return *this;
        }

        /// Set maximum terminal width for help
        Command &max_term_width(std::size_t width) {
            m_max_term_width = width;
            return *this;
        }

        /// Parse arguments
        ParseResult parse(int argc, char *argv[]);

        /// Parse arguments from vector
        ParseResult parse(const std::vector<std::string> &args);

        /// Get matches (call after parse)
        Matches &matches() { return m_matches; }
        const Matches &matches() const { return m_matches; }

        /// Get help string
        std::string help() const;

        /// Get version string
        std::string version_string() const;

        /// Generate shell completion script
        std::string generate_completions(Shell shell) const;

        // Getters
        const std::string &name() const { return m_name; }
        const std::optional<std::string> &get_about() const { return m_about; }
        const std::optional<std::string> &get_version() const { return m_version; }
        const std::vector<Arg> &get_args() const { return m_args; }
        const std::vector<ArgGroup> &get_groups() const { return m_groups; }
        const std::vector<std::shared_ptr<Command>> &get_subcommands() const { return m_subcommands; }
        bool is_subcommand_required() const { return m_subcommand_required; }
        bool is_hidden() const { return m_hidden; }
        const std::vector<std::string> &get_aliases() const { return m_aliases; }
        const std::vector<std::string> &get_visible_aliases() const { return m_visible_aliases; }
        ColorMode get_color_mode() const { return m_color_mode; }
        const HelpTheme &get_theme() const { return m_theme; }
        std::size_t get_suggest_threshold() const { return m_suggest_threshold; }
        const std::optional<std::string> &get_before_help() const { return m_before_help; }
        const std::optional<std::string> &get_after_help() const { return m_after_help; }

        /// Custom help section structure
        struct HelpSection {
            std::string title;
            std::string content;
            int order;
        };

        const std::vector<HelpSection> &get_custom_sections() const { return m_custom_sections; }
        int get_group_order(const std::string &group_name) const {
            auto it = m_group_order.find(group_name);
            return it != m_group_order.end() ? it->second : 50; // Default order is 50
        }
        ConflictMode get_conflict_mode() const { return m_conflict_mode; }
        bool allows_partial_matching() const { return m_allow_partial_matching; }
        bool allows_subcommand_prefix() const { return m_allow_subcommand_prefix; }
        std::ostream *get_deprecation_stream() const { return m_deprecation_stream; }
        const std::optional<std::string> &get_env_prefix() const { return m_env_prefix; }
        bool has_trailing_var_arg() const { return m_trailing_var_arg; }
        const std::string &get_trailing_values_name() const { return m_trailing_values_name; }
        bool allows_hyphen_values() const { return m_allow_hyphen_values; }
        bool ignores_errors() const { return m_ignore_errors; }
        bool shows_help_on_error() const { return m_help_on_error; }
        ErrorMode get_error_mode() const { return m_error_mode; }
        std::size_t get_max_term_width() const { return m_max_term_width; }

        const Arg *find_arg_by_name(const std::string &name) const {
            for (const auto &arg : m_args) {
                if (arg.name() == name)
                    return &arg;
            }
            return nullptr;
        }

      private:
        std::string m_name;
        std::optional<std::string> m_version;
        std::optional<std::string> m_about;
        std::optional<std::string> m_long_about;
        std::optional<std::string> m_author;
        std::optional<std::string> m_usage_override;
        std::optional<std::string> m_help_template;
        std::optional<std::string> m_before_help;
        std::optional<std::string> m_after_help;
        std::optional<std::string> m_after_long_help;
        std::vector<Arg> m_args;
        std::vector<ArgGroup> m_groups;
        std::vector<std::shared_ptr<Command>> m_subcommands;
        std::vector<std::string> m_aliases;
        std::vector<std::string> m_visible_aliases;
        std::function<void(const Matches &)> m_callback;
        std::function<void()> m_pre_parse;
        std::function<void(Matches &)> m_parse_complete;
        std::function<void(const Matches &)> m_final_callback;
        Matches m_matches;
        ColorMode m_color_mode = ColorMode::Auto;
        ConflictMode m_conflict_mode = ConflictMode::Error;
        HelpTheme m_theme;
        struct ConfigLayer {
            std::string path;
            bool required;
        };
        std::string m_config_arg;
        std::string m_default_config_path;
        std::vector<ConfigLayer> m_config_layers;
        std::string m_completion_arg;
        std::ostream *m_deprecation_stream = &std::cerr;
        std::optional<std::string> m_env_prefix;
        std::string m_trailing_values_name = "ARGS";
        std::vector<HelpSection> m_custom_sections;
        std::map<std::string, int> m_group_order;
        std::size_t m_suggest_threshold = 3;
        std::size_t m_max_term_width = 100;
        int m_display_order = 0;
        bool m_subcommand_required = false;
        bool m_allow_external = false;
        bool m_disable_help = false;
        bool m_disable_version = false;
        bool m_propagate_version = false;
        bool m_allow_negative = false;
        bool m_allow_partial_matching = false;
        bool m_allow_subcommand_prefix = false;
        bool m_trailing_var_arg = false;
        bool m_allow_hyphen_values = false;
        bool m_ignore_errors = false;
        bool m_help_on_error = true;
        bool m_hidden = false;
        ErrorMode m_error_mode = ErrorMode::FirstError;

        void add_default_help() {
            m_args.push_back(Arg("help")
                                 .short_name('h')
                                 .long_name("help")
                                 .help("Print help information")
                                 .flag()
                                 .action(ValueAction::Help));
        }

        void add_default_version() {
            if (m_version && !m_disable_version) {
                m_args.push_back(Arg("version")
                                     .short_name('V')
                                     .long_name("version")
                                     .help("Print version information")
                                     .flag()
                                     .action(ValueAction::Version));
            }
        }

        const Arg *find_arg_by_short(char c) const {
            for (const auto &arg : m_args) {
                if (arg.matches_short(c))
                    return &arg;
            }
            return nullptr;
        }

        const Arg *find_arg_by_long(const std::string &name) const {
            for (const auto &arg : m_args) {
                if (arg.matches_long(name))
                    return &arg;
            }
            return nullptr;
        }

        Command *find_subcommand(const std::string &name) {
            // First try exact match
            for (auto &sub : m_subcommands) {
                if (sub->m_name == name)
                    return sub.get();
                for (const auto &alias : sub->m_aliases) {
                    if (alias == name)
                        return sub.get();
                }
            }

            // Try prefix matching if enabled
            if (m_allow_subcommand_prefix && !name.empty()) {
                Command *match = nullptr;
                int match_count = 0;

                for (auto &sub : m_subcommands) {
                    if (sub->m_name.starts_with(name)) {
                        match = sub.get();
                        ++match_count;
                    }
                    for (const auto &alias : sub->m_aliases) {
                        if (alias.starts_with(name)) {
                            match = sub.get();
                            ++match_count;
                        }
                    }
                }

                // Only return if unambiguous match
                if (match_count == 1) {
                    return match;
                }
            }

            return nullptr;
        }

        /// Get all available short option names
        std::vector<std::string> get_short_options() const {
            std::vector<std::string> result;
            for (const auto &arg : m_args) {
                if (arg.short_opt()) {
                    result.push_back(std::string(1, *arg.short_opt()));
                }
            }
            return result;
        }

        /// Get all available long option names
        std::vector<std::string> get_long_options() const {
            std::vector<std::string> result;
            for (const auto &arg : m_args) {
                if (arg.long_opt()) {
                    result.push_back(*arg.long_opt());
                }
            }
            return result;
        }

        /// Get all available subcommand names (optionally excluding hidden)
        std::vector<std::string> get_subcommand_names(bool include_hidden = true) const {
            std::vector<std::string> result;
            for (const auto &sub : m_subcommands) {
                if (!include_hidden && sub->m_hidden) {
                    continue;
                }
                result.push_back(sub->m_name);
                for (const auto &alias : sub->m_aliases) {
                    result.push_back(alias);
                }
            }
            return result;
        }
    };

} // namespace argu
