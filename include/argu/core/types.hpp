#pragma once

/// @file argu/core/types.hpp
/// @brief Core type definitions for argu

#include <any>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace argu {

    /// Value action types
    enum class ValueAction {
        Store,      ///< Store the value (default for options with values)
        StoreTrue,  ///< Store true when flag is present
        StoreFalse, ///< Store false when flag is present
        Append,     ///< Append to a vector
        Count,      ///< Count occurrences
        Help,       ///< Display help
        Version,    ///< Display version
        Completion, ///< Generate shell completions
    };

    /// Number of values expected
    struct ValueCount {
        std::size_t min = 1;
        std::size_t max = 1;

        static constexpr std::size_t unlimited = std::numeric_limits<std::size_t>::max();

        static constexpr ValueCount exactly(std::size_t n) { return {n, n}; }
        static constexpr ValueCount at_least(std::size_t n) { return {n, unlimited}; }
        static constexpr ValueCount at_most(std::size_t n) { return {0, n}; }
        static constexpr ValueCount any() { return {0, unlimited}; }
        static constexpr ValueCount none() { return {0, 0}; }
        static constexpr ValueCount one() { return {1, 1}; }
        static constexpr ValueCount one_or_more() { return {1, unlimited}; }
        static constexpr ValueCount zero_or_more() { return {0, unlimited}; }
        static constexpr ValueCount optional() { return {0, 1}; }
    };

    /// Source of a value (for precedence tracking)
    enum class ValueSource {
        Default,     ///< From default value
        ConfigFile,  ///< From config file
        Environment, ///< From environment variable
        CommandLine, ///< From command line argument
    };

    /// Internal representation of parsed argument
    struct ArgMatch {
        std::string name;
        std::vector<std::string> values;
        std::size_t occurrences = 0;
        bool from_env = false;
        bool from_config = false;
        ValueSource source = ValueSource::Default;
    };

    /// Argument group types
    enum class GroupType {
        Normal,            ///< Regular argument group (visual only)
        MutuallyExclusive, ///< Only one argument in group can be used
        RequiredTogether,  ///< All arguments in group must be used together
        AtLeastOne,        ///< At least one argument must be used
    };

    /// Color mode for help output
    enum class ColorMode {
        Auto,   ///< Detect terminal capability
        Always, ///< Always use colors
        Never,  ///< Never use colors
    };

    /// Shell type for completion generation
    enum class Shell {
        Bash,
        Zsh,
        Fish,
        PowerShell,
        Elvish,
    };

    /// Conflict resolution mode
    enum class ConflictMode {
        Error,     ///< Raise error on conflict (default)
        LastWins,  ///< Last value wins (POSIX style)
        FirstWins, ///< First value wins
    };

    /// Stdin/stdout handling mode
    enum class StdioMode {
        None,   ///< No special handling
        Stdin,  ///< Treat "-" as stdin
        Stdout, ///< Treat "-" as stdout
        Both,   ///< Treat "-" as stdin for input, stdout for output
    };

    namespace detail {
        /// Type conversion helper
        template <typename T> struct Converter {
            static std::optional<T> convert(const std::string &s) {
                if constexpr (std::is_same_v<T, std::string>) {
                    return s;
                } else if constexpr (std::is_same_v<T, bool>) {
                    std::string lower = s;
                    for (auto &c : lower)
                        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on" || lower == "y")
                        return true;
                    if (lower == "false" || lower == "0" || lower == "no" || lower == "off" || lower == "n")
                        return false;
                    return std::nullopt;
                } else if constexpr (std::is_integral_v<T>) {
                    // Handle hex (0x), octal (0o), binary (0b)
                    if (s.size() > 2) {
                        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
                            T value{};
                            auto [ptr, ec] = std::from_chars(s.data() + 2, s.data() + s.size(), value, 16);
                            if (ec == std::errc{} && ptr == s.data() + s.size())
                                return value;
                            return std::nullopt;
                        }
                        if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
                            T value{};
                            auto [ptr, ec] = std::from_chars(s.data() + 2, s.data() + s.size(), value, 2);
                            if (ec == std::errc{} && ptr == s.data() + s.size())
                                return value;
                            return std::nullopt;
                        }
                        if (s[0] == '0' && (s[1] == 'o' || s[1] == 'O')) {
                            T value{};
                            auto [ptr, ec] = std::from_chars(s.data() + 2, s.data() + s.size(), value, 8);
                            if (ec == std::errc{} && ptr == s.data() + s.size())
                                return value;
                            return std::nullopt;
                        }
                    }
                    T value{};
                    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
                    if (ec == std::errc{} && ptr == s.data() + s.size()) {
                        return value;
                    }
                    return std::nullopt;
                } else if constexpr (std::is_floating_point_v<T>) {
                    try {
                        std::size_t pos = 0;
                        if constexpr (std::is_same_v<T, float>) {
                            float val = std::stof(s, &pos);
                            if (pos == s.size())
                                return val;
                            return std::nullopt;
                        } else if constexpr (std::is_same_v<T, double>) {
                            double val = std::stod(s, &pos);
                            if (pos == s.size())
                                return val;
                            return std::nullopt;
                        } else {
                            long double val = std::stold(s, &pos);
                            if (pos == s.size())
                                return val;
                            return std::nullopt;
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

        /// Optional specialization
        template <typename T> struct Converter<std::optional<T>> {
            static std::optional<std::optional<T>> convert(const std::string &s) {
                if (s.empty())
                    return std::optional<T>{};
                auto inner = Converter<T>::convert(s);
                if (!inner)
                    return std::nullopt;
                return std::optional<T>{*inner};
            }
        };
    } // namespace detail

    /// Validator function type - returns error message if validation fails
    using Validator = std::function<std::optional<std::string>(const std::string &)>;

    /// Transformer function type - transforms a value before storing
    using Transformer = std::function<std::string(const std::string &)>;

    /// Completer function type - returns possible completions
    using Completer = std::function<std::vector<std::string>(const std::string &partial)>;

} // namespace argu
