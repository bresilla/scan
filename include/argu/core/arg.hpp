#pragma once

/// @file argu/core/arg.hpp
/// @brief Argument definition with builder pattern (CLAP-style)

#include <argu/core/error.hpp>
#include <argu/core/types.hpp>

#include <any>
#include <climits>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace argu {

    /// Forward declarations
    class Command;

    /// Argument builder class (CLAP/CLI11 inspired)
    class Arg {
        friend class Command;
        friend class Parser;

      public:
        /// Construct argument with internal name/id
        explicit Arg(std::string name) : m_name(std::move(name)) {}

        /// Set short option name (e.g., 'v' for -v)
        Arg &short_name(char c) {
            m_short = c;
            return *this;
        }

        /// Set long option name (e.g., "verbose" for --verbose)
        Arg &long_name(std::string name) {
            m_long = std::move(name);
            return *this;
        }

        /// Set help description
        Arg &help(std::string text) {
            m_help = std::move(text);
            return *this;
        }

        /// Alias for help()
        Arg &about(std::string text) { return help(std::move(text)); }

        /// Mark as required
        Arg &required(bool req = true) {
            m_required = req;
            return *this;
        }

        /// Set default value (as string)
        Arg &default_value(std::string value) {
            m_default = std::move(value);
            return *this;
        }

        /// Set default value (any type)
        template <typename T> Arg &default_value(const T &value) {
            if constexpr (std::is_same_v<T, const char *>) {
                m_default = std::string(value);
            } else {
                std::ostringstream oss;
                oss << value;
                m_default = oss.str();
            }
            return *this;
        }

        /// Set default value that is only shown in help (not applied)
        Arg &default_value_if_missing(std::string value) {
            m_default_if_missing = std::move(value);
            return *this;
        }

        /// Set value name for help display (e.g., "FILE", "NUM")
        Arg &value_name(std::string name) {
            m_value_name = std::move(name);
            return *this;
        }

        /// Alias for value_name
        Arg &metavar(std::string name) { return value_name(std::move(name)); }

        /// Set number of values this argument takes
        Arg &num_values(ValueCount count) {
            m_value_count = count;
            return *this;
        }

        /// Takes exactly N values
        Arg &takes_values(std::size_t n) {
            m_value_count = ValueCount::exactly(n);
            return *this;
        }

        /// Takes 0 or more values
        Arg &takes_multiple() {
            m_value_count = ValueCount::zero_or_more();
            return *this;
        }

        /// Takes 1 or more values
        Arg &takes_one_or_more() {
            m_value_count = ValueCount::one_or_more();
            return *this;
        }

        /// Set action type
        Arg &action(ValueAction act) {
            m_action = act;
            return *this;
        }

        /// Mark as a flag (stores true/false, takes no value)
        Arg &flag() {
            m_action = ValueAction::StoreTrue;
            m_value_count = ValueCount::none();
            return *this;
        }

        /// Bind to a bool variable (flag)
        Arg &flag(bool &target) {
            flag();
            m_target = &target;
            m_target_type = TargetType::Bool;
            return *this;
        }

        /// Mark as a counting flag (e.g., -vvv for verbosity)
        Arg &count() {
            m_action = ValueAction::Count;
            m_value_count = ValueCount::none();
            return *this;
        }

        /// Bind to an int for counting
        Arg &count(int &target) {
            count();
            m_target = &target;
            m_target_type = TargetType::Int;
            return *this;
        }

        /// Mark as positional argument (no - or -- prefix)
        Arg &positional(bool pos = true) {
            m_positional = pos;
            return *this;
        }

        /// Set positional index (for ordering)
        Arg &index(std::size_t idx) {
            m_positional = true;
            m_index = idx;
            return *this;
        }

        /// Hide from help output
        Arg &hidden(bool hide = true) {
            m_hidden = hide;
            return *this;
        }

        /// Set environment variable to read from
        Arg &env(std::string var_name) {
            m_env_var = std::move(var_name);
            return *this;
        }

        /// Add a validator
        Arg &validate(Validator validator) {
            m_validators.push_back(std::move(validator));
            return *this;
        }

        /// Add a value transformer (runs before validation)
        Arg &transform(Transformer transformer) {
            m_transformers.push_back(std::move(transformer));
            return *this;
        }

        /// Set custom completer for shell completion
        Arg &completer(Completer comp) {
            m_completer = std::move(comp);
            return *this;
        }

        /// Add possible values (choices)
        Arg &choices(std::vector<std::string> values) {
            m_choices = std::move(values);
            return *this;
        }

        /// Add possible values (initializer list)
        Arg &choices(std::initializer_list<std::string> values) {
            m_choices = std::vector<std::string>(values);
            return *this;
        }

        /// Set group for help display
        Arg &group(std::string name) {
            m_group = std::move(name);
            return *this;
        }

        /// Mark as global (available to subcommands)
        Arg &global(bool g = true) {
            m_global = g;
            return *this;
        }

        /// Set conflicts with another argument
        Arg &conflicts_with(std::string arg_name) {
            m_conflicts.push_back(std::move(arg_name));
            return *this;
        }

        /// Set conflicts with multiple arguments
        Arg &conflicts_with(std::initializer_list<std::string> args) {
            for (const auto &a : args)
                m_conflicts.push_back(a);
            return *this;
        }

        /// Set requires another argument
        Arg &requires_arg(std::string arg_name) {
            m_requires.push_back(std::move(arg_name));
            return *this;
        }

        /// Set requires multiple arguments
        Arg &requires_arg(std::initializer_list<std::string> args) {
            for (const auto &a : args)
                m_requires.push_back(a);
            return *this;
        }

        /// Set requires if another argument has specific value
        Arg &requires_if(std::string arg_name, std::string value) {
            m_requires_if.emplace_back(std::move(arg_name), std::move(value));
            return *this;
        }

        /// Required unless another argument is present
        Arg &required_unless(std::string arg_name) {
            m_required_unless.push_back(std::move(arg_name));
            return *this;
        }

        /// Required unless multiple arguments (any one satisfies)
        Arg &required_unless(std::initializer_list<std::string> args) {
            for (const auto &a : args)
                m_required_unless.push_back(a);
            return *this;
        }

        /// Required if another argument equals specific value
        Arg &required_if_eq(std::string arg_name, std::string value) {
            m_required_if_eq.emplace_back(std::move(arg_name), std::move(value));
            return *this;
        }

        /// Set default if another argument has specific value
        Arg &default_value_if(std::string arg_name, std::string arg_value, std::string default_val) {
            m_default_value_if.push_back({std::move(arg_name), std::move(arg_value), std::move(default_val)});
            return *this;
        }

        /// Mark as deprecated with message
        Arg &deprecated(std::string message = "") {
            m_deprecated = true;
            m_deprecated_message = std::move(message);
            return *this;
        }

        /// Add hidden alias (for backwards compatibility)
        Arg &hidden_alias(std::string alias) {
            m_hidden_aliases.push_back(std::move(alias));
            return *this;
        }

        /// Renamed from old name (will warn users)
        Arg &renamed_from(std::string old_name) {
            m_renamed_from = std::move(old_name);
            return *this;
        }

        /// Set implicit value (used when flag present without value)
        Arg &implicit_value(std::string value) {
            m_implicit_value = std::move(value);
            return *this;
        }

        /// Set value delimiter for parsing multiple values from one string
        Arg &value_delimiter(char delim) {
            m_value_delimiter = delim;
            return *this;
        }

        /// Allow "-" to mean stdin
        Arg &allow_stdin(bool allow = true) {
            m_allow_stdin = allow;
            return *this;
        }

        /// Allow "-" to mean stdout
        Arg &allow_stdout(bool allow = true) {
            m_allow_stdout = allow;
            return *this;
        }

        /// Set value terminator (stops consuming values)
        Arg &value_terminator(std::string term) {
            m_value_terminator = std::move(term);
            return *this;
        }

        /// Make this a negatable flag (--flag and --no-flag both work)
        Arg &negatable(bool neg = true) {
            m_negatable = neg;
            return *this;
        }

        /// Set visible aliases (shown in help)
        Arg &visible_alias(std::string alias) {
            m_visible_aliases.push_back(std::move(alias));
            return *this;
        }

        /// Set multiple visible aliases
        Arg &visible_aliases(std::initializer_list<std::string> aliases) {
            for (const auto &a : aliases)
                m_visible_aliases.push_back(a);
            return *this;
        }

        /// Set environment variable prefix (for auto-detection)
        Arg &env_prefix(std::string prefix) {
            m_env_prefix = std::move(prefix);
            return *this;
        }

        /// Set display order in help
        Arg &display_order(int order) {
            m_display_order = order;
            return *this;
        }

        /// Set exclusive group (only one arg in this exclusive group can be used)
        Arg &exclusive_group(std::string group_name) {
            m_exclusive_group = std::move(group_name);
            return *this;
        }

        /// Set value hint for shell completions
        Arg &value_hint(std::string hint) {
            m_value_hint = std::move(hint);
            return *this;
        }

        /// Common value hints
        Arg &hint_file() { return value_hint("FILE"); }
        Arg &hint_dir() { return value_hint("DIR"); }
        Arg &hint_path() { return value_hint("PATH"); }
        Arg &hint_url() { return value_hint("URL"); }
        Arg &hint_hostname() { return value_hint("HOST"); }
        Arg &hint_username() { return value_hint("USER"); }
        Arg &hint_email() { return value_hint("EMAIL"); }
        Arg &hint_command() { return value_hint("CMD"); }

        /// Set last value wins behavior (override previous values)
        Arg &last_wins(bool lw = true) {
            m_last_wins = lw;
            return *this;
        }

        /// Set number of times this arg can be used (0 = unlimited)
        Arg &max_occurrences(std::size_t max) {
            m_max_occurrences = max;
            return *this;
        }

        // ===== Action Callbacks =====

        /// Callback type for single value
        using ValueCallback = std::function<void(const std::string &)>;
        /// Callback type for flag
        using FlagCallback = std::function<void(bool)>;
        /// Callback type for count
        using CountCallback = std::function<void(int)>;

        /// Add a callback that's called when this argument is parsed (chainable)
        Arg &on_parse(ValueCallback cb) {
            m_value_callbacks.push_back(std::move(cb));
            return *this;
        }

        /// Add a callback for flag values
        Arg &on_flag(FlagCallback cb) {
            m_flag_callbacks.push_back(std::move(cb));
            return *this;
        }

        /// Add a callback for count values
        Arg &on_count(CountCallback cb) {
            m_count_callbacks.push_back(std::move(cb));
            return *this;
        }

        /// Add a callback that transforms and validates the value
        Arg &each(std::function<std::string(const std::string &)> transform) {
            m_transformers.push_back(std::move(transform));
            return *this;
        }

        /// Add a callback that runs if this argument is missing
        Arg &on_missing(std::function<void()> cb) {
            m_missing_callback = std::move(cb);
            return *this;
        }

        /// Bind to a string variable
        Arg &value_of(std::string &target) {
            m_target = &target;
            m_target_type = TargetType::String;
            return *this;
        }

        /// Bind to an int variable
        Arg &value_of(int &target) {
            m_target = &target;
            m_target_type = TargetType::Int;
            return *this;
        }

        /// Bind to a long variable
        Arg &value_of(long &target) {
            m_target = &target;
            m_target_type = TargetType::Long;
            return *this;
        }

        /// Bind to a long long variable
        Arg &value_of(long long &target) {
            m_target = &target;
            m_target_type = TargetType::LongLong;
            return *this;
        }

        /// Bind to an unsigned int variable
        Arg &value_of(unsigned int &target) {
            m_target = &target;
            m_target_type = TargetType::UInt;
            return *this;
        }

        /// Bind to a size_t variable
        Arg &value_of(std::size_t &target) {
            m_target = &target;
            m_target_type = TargetType::SizeT;
            return *this;
        }

        /// Bind to a float variable
        Arg &value_of(float &target) {
            m_target = &target;
            m_target_type = TargetType::Float;
            return *this;
        }

        /// Bind to a double variable
        Arg &value_of(double &target) {
            m_target = &target;
            m_target_type = TargetType::Double;
            return *this;
        }

        /// Bind to a bool variable (for options that take bool values)
        Arg &value_of(bool &target) {
            m_target = &target;
            m_target_type = TargetType::Bool;
            return *this;
        }

        /// Bind to a vector<string>
        Arg &value_of(std::vector<std::string> &target) {
            m_target = &target;
            m_target_type = TargetType::VectorString;
            m_action = ValueAction::Append;
            return *this;
        }

        /// Bind to a vector<int>
        Arg &value_of(std::vector<int> &target) {
            m_target = &target;
            m_target_type = TargetType::VectorInt;
            m_action = ValueAction::Append;
            return *this;
        }

        /// Bind to a vector<double>
        Arg &value_of(std::vector<double> &target) {
            m_target = &target;
            m_target_type = TargetType::VectorDouble;
            m_action = ValueAction::Append;
            return *this;
        }

        /// Bind to an optional<string>
        Arg &value_of(std::optional<std::string> &target) {
            m_target = &target;
            m_target_type = TargetType::OptionalString;
            return *this;
        }

        /// Bind to an optional<int>
        Arg &value_of(std::optional<int> &target) {
            m_target = &target;
            m_target_type = TargetType::OptionalInt;
            return *this;
        }

        // ============= Getters =============

        const std::string &name() const { return m_name; }
        std::optional<char> short_opt() const { return m_short; }
        const std::optional<std::string> &long_opt() const { return m_long; }
        const std::string &help_text() const { return m_help; }
        bool is_required() const { return m_required; }
        bool is_positional() const { return m_positional; }
        bool is_hidden() const { return m_hidden; }
        bool is_global() const { return m_global; }
        bool is_flag() const { return m_action == ValueAction::StoreTrue || m_action == ValueAction::StoreFalse; }
        bool is_count() const { return m_action == ValueAction::Count; }
        const std::optional<std::string> &default_val() const { return m_default; }
        const std::string &value_name_str() const { return m_value_name; }
        ValueCount value_count() const { return m_value_count; }
        ValueAction get_action() const { return m_action; }
        const std::vector<std::string> &get_choices() const { return m_choices; }
        const std::string &get_group() const { return m_group; }
        const std::optional<std::string> &get_env() const { return m_env_var; }
        std::optional<std::size_t> get_index() const { return m_index; }
        const std::vector<std::string> &get_conflicts() const { return m_conflicts; }
        const std::vector<std::string> &get_requires() const { return m_requires; }
        const std::vector<std::string> &get_required_unless() const { return m_required_unless; }
        const std::vector<std::pair<std::string, std::string>> &get_required_if_eq() const { return m_required_if_eq; }
        const std::vector<std::pair<std::string, std::string>> &get_requires_if() const { return m_requires_if; }
        const std::vector<std::tuple<std::string, std::string, std::string>> &get_default_value_if() const { return m_default_value_if; }
        const std::vector<Validator> &get_validators() const { return m_validators; }
        const std::vector<Transformer> &get_transformers() const { return m_transformers; }
        const Completer &get_completer() const { return m_completer; }
        bool is_deprecated() const { return m_deprecated; }
        const std::string &get_deprecated_message() const { return m_deprecated_message; }
        const std::vector<std::string> &get_hidden_aliases() const { return m_hidden_aliases; }
        const std::optional<std::string> &get_renamed_from() const { return m_renamed_from; }
        const std::optional<std::string> &get_implicit_value() const { return m_implicit_value; }
        std::optional<char> get_value_delimiter() const { return m_value_delimiter; }
        bool allows_stdin() const { return m_allow_stdin; }
        bool allows_stdout() const { return m_allow_stdout; }
        const std::optional<std::string> &get_value_terminator() const { return m_value_terminator; }
        bool is_negatable() const { return m_negatable; }
        const std::vector<std::string> &get_visible_aliases() const { return m_visible_aliases; }
        const std::optional<std::string> &get_env_prefix() const { return m_env_prefix; }
        int get_display_order() const { return m_display_order; }
        const std::optional<std::string> &get_exclusive_group() const { return m_exclusive_group; }
        const std::optional<std::string> &get_value_hint() const { return m_value_hint; }
        bool is_last_wins() const { return m_last_wins; }
        std::size_t get_max_occurrences() const { return m_max_occurrences; }

        /// Check if this arg matches a short option
        bool matches_short(char c) const { return m_short.has_value() && m_short.value() == c; }

        /// Check if this arg matches a long option (including hidden aliases, visible aliases, negatable)
        bool matches_long(const std::string &name) const {
            if (m_long.has_value() && m_long.value() == name)
                return true;
            // Check negatable (--no-flag matches --flag if negatable)
            if (m_negatable && m_long.has_value()) {
                if (name == "no-" + *m_long)
                    return true;
            }
            // Check visible aliases
            for (const auto &alias : m_visible_aliases) {
                if (alias == name)
                    return true;
            }
            // Check hidden aliases
            for (const auto &alias : m_hidden_aliases) {
                if (alias == name)
                    return true;
            }
            // Check renamed_from
            if (m_renamed_from && *m_renamed_from == name)
                return true;
            return false;
        }

        /// Check if this is a negated match (--no-flag)
        bool is_negated_match(const std::string &name) const {
            if (m_negatable && m_long.has_value()) {
                return name == "no-" + *m_long;
            }
            return false;
        }

        /// Check if match was via deprecated name
        bool is_deprecated_match(const std::string &name) const {
            if (m_deprecated)
                return true;
            // Check if matched via renamed_from
            if (m_renamed_from && *m_renamed_from == name)
                return true;
            return false;
        }

        /// Apply transformers to a value
        std::string apply_transformers(const std::string &value) const {
            std::string result = value;
            for (const auto &t : m_transformers) {
                result = t(result);
            }
            return result;
        }

        /// Apply value to bound target
        void apply_value(const std::string &value) const {
            // Invoke value callbacks
            for (const auto &cb : m_value_callbacks) {
                cb(value);
            }

            if (!m_target)
                return;

            switch (m_target_type) {
            case TargetType::String:
                *static_cast<std::string *>(m_target) = value;
                break;
            case TargetType::Int:
                if (auto v = detail::Converter<int>::convert(value)) {
                    *static_cast<int *>(m_target) = *v;
                }
                break;
            case TargetType::Long:
                if (auto v = detail::Converter<long>::convert(value)) {
                    *static_cast<long *>(m_target) = *v;
                }
                break;
            case TargetType::LongLong:
                if (auto v = detail::Converter<long long>::convert(value)) {
                    *static_cast<long long *>(m_target) = *v;
                }
                break;
            case TargetType::UInt:
                if (auto v = detail::Converter<unsigned int>::convert(value)) {
                    *static_cast<unsigned int *>(m_target) = *v;
                }
                break;
            case TargetType::SizeT:
                if (auto v = detail::Converter<std::size_t>::convert(value)) {
                    *static_cast<std::size_t *>(m_target) = *v;
                }
                break;
            case TargetType::Float:
                if (auto v = detail::Converter<float>::convert(value)) {
                    *static_cast<float *>(m_target) = *v;
                }
                break;
            case TargetType::Double:
                if (auto v = detail::Converter<double>::convert(value)) {
                    *static_cast<double *>(m_target) = *v;
                }
                break;
            case TargetType::Bool:
                if (auto v = detail::Converter<bool>::convert(value)) {
                    *static_cast<bool *>(m_target) = *v;
                }
                break;
            case TargetType::OptionalString:
                *static_cast<std::optional<std::string> *>(m_target) = value;
                break;
            case TargetType::OptionalInt:
                if (auto v = detail::Converter<int>::convert(value)) {
                    *static_cast<std::optional<int> *>(m_target) = *v;
                }
                break;
            default:
                break;
            }
        }

        /// Apply flag value to bound target
        void apply_flag(bool value) const {
            // Invoke flag callbacks
            for (const auto &cb : m_flag_callbacks) {
                cb(value);
            }

            if (!m_target)
                return;

            if (m_target_type == TargetType::Bool) {
                *static_cast<bool *>(m_target) = value;
            }
        }

        /// Apply count value to bound target
        void apply_count(int count) const {
            // Invoke count callbacks
            for (const auto &cb : m_count_callbacks) {
                cb(count);
            }

            if (!m_target)
                return;

            if (m_target_type == TargetType::Int) {
                *static_cast<int *>(m_target) = count;
            }
        }

        /// Append value to vector target
        void append_value(const std::string &value) const {
            if (!m_target)
                return;

            switch (m_target_type) {
            case TargetType::VectorString:
                static_cast<std::vector<std::string> *>(m_target)->push_back(value);
                break;
            case TargetType::VectorInt:
                if (auto v = detail::Converter<int>::convert(value)) {
                    static_cast<std::vector<int> *>(m_target)->push_back(*v);
                }
                break;
            case TargetType::VectorDouble:
                if (auto v = detail::Converter<double>::convert(value)) {
                    static_cast<std::vector<double> *>(m_target)->push_back(*v);
                }
                break;
            default:
                break;
            }
        }

        /// Apply default value to target
        void apply_default() const {
            if (m_default && m_target) {
                if (m_action == ValueAction::Append) {
                    // Don't apply default to vectors if they already have values
                } else {
                    apply_value(*m_default);
                }
            }
        }

      private:
        enum class TargetType {
            None,
            String,
            Int,
            Long,
            LongLong,
            UInt,
            SizeT,
            Float,
            Double,
            Bool,
            VectorString,
            VectorInt,
            VectorDouble,
            OptionalString,
            OptionalInt,
        };

        std::string m_name;
        std::optional<char> m_short;
        std::optional<std::string> m_long;
        std::string m_help;
        std::string m_value_name = "VALUE";
        std::string m_group = "OPTIONS";
        std::optional<std::string> m_default;
        std::optional<std::string> m_default_if_missing;
        std::optional<std::string> m_env_var;
        std::optional<std::size_t> m_index;
        std::vector<std::string> m_choices;
        std::vector<std::string> m_conflicts;
        std::vector<std::string> m_requires;
        std::vector<std::string> m_required_unless;
        std::vector<std::pair<std::string, std::string>> m_requires_if;
        std::vector<std::pair<std::string, std::string>> m_required_if_eq;
        std::vector<std::tuple<std::string, std::string, std::string>> m_default_value_if;
        std::vector<Validator> m_validators;
        std::vector<Transformer> m_transformers;
        std::vector<ValueCallback> m_value_callbacks;
        std::vector<FlagCallback> m_flag_callbacks;
        std::vector<CountCallback> m_count_callbacks;
        std::function<void()> m_missing_callback;
        Completer m_completer;
        ValueCount m_value_count = ValueCount::one();
        ValueAction m_action = ValueAction::Store;
        // Deprecation support
        bool m_deprecated = false;
        std::string m_deprecated_message;
        std::vector<std::string> m_hidden_aliases;
        std::optional<std::string> m_renamed_from;
        // Implicit value support
        std::optional<std::string> m_implicit_value;
        // Value delimiter
        std::optional<char> m_value_delimiter;
        // Value terminator
        std::optional<std::string> m_value_terminator;
        // Stdin/stdout handling
        bool m_allow_stdin = false;
        bool m_allow_stdout = false;
        // Negatable flags
        bool m_negatable = false;
        std::vector<std::string> m_visible_aliases;
        std::optional<std::string> m_env_prefix;
        int m_display_order = 0;
        std::optional<std::string> m_exclusive_group;
        std::optional<std::string> m_value_hint;
        bool m_last_wins = false;
        std::size_t m_max_occurrences = 0; // 0 = unlimited
        // Flags
        bool m_required = false;
        bool m_positional = false;
        bool m_hidden = false;
        bool m_global = false;
        void *m_target = nullptr;
        TargetType m_target_type = TargetType::None;
    };

} // namespace argu
