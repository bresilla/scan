#pragma once

/// @file command.hpp
/// @brief Command and subcommand definitions (CLAP-style)

#include <argue/arg.hpp>
#include <argue/error.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace argue {

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
        Matches(const Matches &other) : m_matches(other.m_matches), m_subcommand(other.m_subcommand) {
            if (other.m_subcommand_matches) {
                m_subcommand_matches = std::make_unique<Matches>(*other.m_subcommand_matches);
            }
        }

        // Copy assignment
        Matches &operator=(const Matches &other) {
            if (this != &other) {
                m_matches = other.m_matches;
                m_subcommand = other.m_subcommand;
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

      private:
        std::vector<ArgMatch> m_matches;
        std::optional<std::string> m_subcommand;
        std::unique_ptr<Matches> m_subcommand_matches;

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
            m_matches.push_back(ArgMatch{name, {}, 0});
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

        /// Set color choice for help
        enum class ColorChoice { Auto, Always, Never };
        Command &color(ColorChoice choice) {
            m_color = choice;
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

        /// Set callback to run when this command is matched
        Command &callback(std::function<void(const Matches &)> cb) {
            m_callback = std::move(cb);
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

        // Getters
        const std::string &name() const { return m_name; }
        const std::optional<std::string> &get_about() const { return m_about; }
        const std::optional<std::string> &get_version() const { return m_version; }
        const std::vector<Arg> &get_args() const { return m_args; }
        const std::vector<std::shared_ptr<Command>> &get_subcommands() const { return m_subcommands; }
        bool is_subcommand_required() const { return m_subcommand_required; }
        bool is_hidden() const { return m_hidden; }

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
        std::vector<std::shared_ptr<Command>> m_subcommands;
        std::vector<std::string> m_aliases;
        std::vector<std::string> m_visible_aliases;
        std::function<void(const Matches &)> m_callback;
        Matches m_matches;
        ColorChoice m_color = ColorChoice::Auto;
        int m_display_order = 0;
        bool m_subcommand_required = false;
        bool m_allow_external = false;
        bool m_disable_help = false;
        bool m_disable_version = false;
        bool m_propagate_version = false;
        bool m_allow_negative = false;
        bool m_hidden = false;

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

        const Arg *find_arg_by_name(const std::string &name) const {
            for (const auto &arg : m_args) {
                if (arg.name() == name)
                    return &arg;
            }
            return nullptr;
        }

        Command *find_subcommand(const std::string &name) {
            for (auto &sub : m_subcommands) {
                if (sub->m_name == name)
                    return sub.get();
                for (const auto &alias : sub->m_aliases) {
                    if (alias == name)
                        return sub.get();
                }
            }
            return nullptr;
        }
    };

} // namespace argue
