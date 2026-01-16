#pragma once

/// @file arg.hpp
/// @brief Argument definition with builder pattern (CLAP-style)

#include <argue/error.hpp>

#include <any>
#include <charconv>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace argue {

    /// Forward declarations
    class Command;

    /// Value action types
    enum class ValueAction {
        Store,      ///< Store the value (default for options with values)
        StoreTrue,  ///< Store true when flag is present
        StoreFalse, ///< Store false when flag is present
        Append,     ///< Append to a vector
        Count,      ///< Count occurrences
        Help,       ///< Display help
        Version,    ///< Display version
    };

    /// Number of values expected
    struct ValueCount {
        std::size_t min = 1;
        std::size_t max = 1;

        static constexpr std::size_t unlimited = std::numeric_limits<std::size_t>::max();

        static ValueCount exactly(std::size_t n) { return {n, n}; }
        static ValueCount at_least(std::size_t n) { return {n, unlimited}; }
        static ValueCount at_most(std::size_t n) { return {0, n}; }
        static ValueCount any() { return {0, unlimited}; }
        static ValueCount none() { return {0, 0}; }
        static ValueCount one() { return {1, 1}; }
        static ValueCount one_or_more() { return {1, unlimited}; }
        static ValueCount zero_or_more() { return {0, unlimited}; }
        static ValueCount optional() { return {0, 1}; }
    };

    /// Internal representation of parsed argument
    struct ArgMatch {
        std::string name;
        std::vector<std::string> values;
        std::size_t occurrences = 0;
    };

    namespace detail {
        /// Type conversion helper
        template <typename T> struct Converter {
            static std::optional<T> convert(const std::string &s) {
                if constexpr (std::is_same_v<T, std::string>) {
                    return s;
                } else if constexpr (std::is_same_v<T, bool>) {
                    if (s == "true" || s == "1" || s == "yes" || s == "on")
                        return true;
                    if (s == "false" || s == "0" || s == "no" || s == "off")
                        return false;
                    return std::nullopt;
                } else if constexpr (std::is_integral_v<T>) {
                    T value{};
                    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
                    if (ec == std::errc{} && ptr == s.data() + s.size()) {
                        return value;
                    }
                    return std::nullopt;
                } else if constexpr (std::is_floating_point_v<T>) {
                    try {
                        if constexpr (std::is_same_v<T, float>) {
                            return std::stof(s);
                        } else if constexpr (std::is_same_v<T, double>) {
                            return std::stod(s);
                        } else {
                            return std::stold(s);
                        }
                    } catch (...) {
                        return std::nullopt;
                    }
                } else {
                    // Try stream extraction as fallback
                    std::istringstream iss(s);
                    T value{};
                    if (iss >> value && iss.eof()) {
                        return value;
                    }
                    return std::nullopt;
                }
            }
        };

        /// Vector specialization
        template <typename T> struct Converter<std::vector<T>> {
            static std::optional<std::vector<T>> convert(const std::vector<std::string> &values) {
                std::vector<T> result;
                result.reserve(values.size());
                for (const auto &v : values) {
                    auto converted = Converter<T>::convert(v);
                    if (!converted)
                        return std::nullopt;
                    result.push_back(std::move(*converted));
                }
                return result;
            }
        };
    } // namespace detail

    /// Validator function type
    using Validator = std::function<std::optional<std::string>(const std::string &)>;

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

        /// Set requires another argument
        Arg &requires_arg(std::string arg_name) {
            m_requires.push_back(std::move(arg_name));
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

        // Getters
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
        const std::vector<Validator> &get_validators() const { return m_validators; }

        /// Check if this arg matches a short option
        bool matches_short(char c) const { return m_short.has_value() && m_short.value() == c; }

        /// Check if this arg matches a long option
        bool matches_long(const std::string &name) const { return m_long.has_value() && m_long.value() == name; }

        /// Apply value to bound target
        void apply_value(const std::string &value) const {
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
            default:
                break;
            }
        }

        /// Apply flag value to bound target
        void apply_flag(bool value) const {
            if (!m_target)
                return;

            if (m_target_type == TargetType::Bool) {
                *static_cast<bool *>(m_target) = value;
            }
        }

        /// Apply count value to bound target
        void apply_count(int count) const {
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
        };

        std::string m_name;
        std::optional<char> m_short;
        std::optional<std::string> m_long;
        std::string m_help;
        std::string m_value_name = "VALUE";
        std::string m_group = "OPTIONS";
        std::optional<std::string> m_default;
        std::optional<std::string> m_env_var;
        std::optional<std::size_t> m_index;
        std::vector<std::string> m_choices;
        std::vector<std::string> m_conflicts;
        std::vector<std::string> m_requires;
        std::vector<Validator> m_validators;
        ValueCount m_value_count = ValueCount::one();
        ValueAction m_action = ValueAction::Store;
        bool m_required = false;
        bool m_positional = false;
        bool m_hidden = false;
        bool m_global = false;
        void *m_target = nullptr;
        TargetType m_target_type = TargetType::None;
    };

} // namespace argue
