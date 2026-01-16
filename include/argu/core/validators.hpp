#pragma once

/// @file argu/core/validators.hpp
/// @brief Built-in validators for argument values

#include <argu/core/types.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <regex>
#include <string>

namespace argu {

    /// Namespace for built-in validators
    namespace validators {

        // ============= File System Validators =============

        /// Validator that checks if a path exists
        inline Validator path_exists() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::filesystem::exists(value)) {
                    return "path does not exist: " + value;
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if a file exists
        inline Validator file_exists() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::filesystem::is_regular_file(value)) {
                    return "file does not exist: " + value;
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if a directory exists
        inline Validator dir_exists() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::filesystem::is_directory(value)) {
                    return "directory does not exist: " + value;
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if a path does NOT exist (for new file creation)
        inline Validator path_not_exists() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (std::filesystem::exists(value)) {
                    return "path already exists: " + value;
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if parent directory exists (for creating new files)
        inline Validator parent_exists() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::filesystem::path p(value);
                auto parent = p.parent_path();
                if (!parent.empty() && !std::filesystem::exists(parent)) {
                    return "parent directory does not exist: " + parent.string();
                }
                return std::nullopt;
            };
        }

        /// Validator that checks file extension
        inline Validator has_extension(const std::vector<std::string> &extensions) {
            return [extensions](const std::string &value) -> std::optional<std::string> {
                std::filesystem::path p(value);
                std::string ext = p.extension().string();
                if (!ext.empty() && ext[0] == '.')
                    ext = ext.substr(1);

                for (auto &c : ext)
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

                for (const auto &e : extensions) {
                    std::string lower_e = e;
                    for (auto &c : lower_e)
                        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    if (lower_e[0] == '.')
                        lower_e = lower_e.substr(1);
                    if (ext == lower_e)
                        return std::nullopt;
                }

                std::string valid;
                for (std::size_t i = 0; i < extensions.size(); ++i) {
                    if (i > 0)
                        valid += ", ";
                    valid += extensions[i];
                }
                return "file must have extension: " + valid;
            };
        }

        // ============= String Validators =============

        /// Validator that checks if a value is non-empty
        inline Validator non_empty() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (value.empty()) {
                    return "value cannot be empty";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks minimum length
        inline Validator min_length(std::size_t min) {
            return [min](const std::string &value) -> std::optional<std::string> {
                if (value.length() < min) {
                    return "value must be at least " + std::to_string(min) + " characters";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks maximum length
        inline Validator max_length(std::size_t max) {
            return [max](const std::string &value) -> std::optional<std::string> {
                if (value.length() > max) {
                    return "value must be at most " + std::to_string(max) + " characters";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks length is within range
        inline Validator length_range(std::size_t min, std::size_t max) {
            return [min, max](const std::string &value) -> std::optional<std::string> {
                if (value.length() < min || value.length() > max) {
                    return "value length must be between " + std::to_string(min) + " and " + std::to_string(max);
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value is alphanumeric
        inline Validator alphanumeric() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isalnum(static_cast<unsigned char>(c)); })) {
                    return "value must be alphanumeric";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value is alphabetic only
        inline Validator alphabetic() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isalpha(static_cast<unsigned char>(c)); })) {
                    return "value must contain only letters";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value is numeric only
        inline Validator numeric() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
                    return "value must contain only digits";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value starts with prefix
        inline Validator starts_with(const std::string &prefix) {
            return [prefix](const std::string &value) -> std::optional<std::string> {
                if (value.size() < prefix.size() || value.substr(0, prefix.size()) != prefix) {
                    return "value must start with '" + prefix + "'";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value ends with suffix
        inline Validator ends_with(const std::string &suffix) {
            return [suffix](const std::string &value) -> std::optional<std::string> {
                if (value.size() < suffix.size() || value.substr(value.size() - suffix.size()) != suffix) {
                    return "value must end with '" + suffix + "'";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value contains substring
        inline Validator contains(const std::string &substr) {
            return [substr](const std::string &value) -> std::optional<std::string> {
                if (value.find(substr) == std::string::npos) {
                    return "value must contain '" + substr + "'";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value does not contain substring
        inline Validator not_contains(const std::string &substr) {
            return [substr](const std::string &value) -> std::optional<std::string> {
                if (value.find(substr) != std::string::npos) {
                    return "value must not contain '" + substr + "'";
                }
                return std::nullopt;
            };
        }

        // ============= Numeric Validators =============

        /// Validator that checks value is within a numeric range (inclusive)
        template <typename T> Validator range(T min_val, T max_val) {
            return [min_val, max_val](const std::string &value) -> std::optional<std::string> {
                auto converted = detail::Converter<T>::convert(value);
                if (!converted) {
                    return "invalid number format";
                }
                if (*converted < min_val || *converted > max_val) {
                    return "value must be between " + std::to_string(min_val) + " and " + std::to_string(max_val);
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value is positive
        template <typename T = long long> Validator positive() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto converted = detail::Converter<T>::convert(value);
                if (!converted) {
                    return "invalid number format";
                }
                if (*converted <= 0) {
                    return "value must be positive";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks value is non-negative
        template <typename T = long long> Validator non_negative() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto converted = detail::Converter<T>::convert(value);
                if (!converted) {
                    return "invalid number format";
                }
                if (*converted < 0) {
                    return "value must be non-negative";
                }
                return std::nullopt;
            };
        }

        /// Validator for network port (1-65535)
        inline Validator port() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto converted = detail::Converter<int>::convert(value);
                if (!converted) {
                    return "invalid port number";
                }
                if (*converted < 1 || *converted > 65535) {
                    return "port must be between 1 and 65535";
                }
                return std::nullopt;
            };
        }

        /// Validator for non-privileged port (1024-65535)
        inline Validator non_privileged_port() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto converted = detail::Converter<int>::convert(value);
                if (!converted) {
                    return "invalid port number";
                }
                if (*converted < 1024 || *converted > 65535) {
                    return "port must be between 1024 and 65535 (non-privileged)";
                }
                return std::nullopt;
            };
        }

        // ============= Regex Validators =============

        /// Validator that checks value matches a regex pattern
        inline Validator regex(const std::string &pattern, const std::string &error_msg = "") {
            return [pattern, error_msg](const std::string &value) -> std::optional<std::string> {
                std::regex re(pattern);
                if (!std::regex_match(value, re)) {
                    if (!error_msg.empty()) {
                        return error_msg;
                    }
                    return "value does not match pattern: " + pattern;
                }
                return std::nullopt;
            };
        }

        // ============= Network Validators =============

        /// Validator for email addresses (basic check)
        inline Validator email() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
                if (!std::regex_match(value, re)) {
                    return "invalid email address";
                }
                return std::nullopt;
            };
        }

        /// Validator for URLs (basic check)
        inline Validator url() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(https?://[^\s/$.?#].[^\s]*)");
                if (!std::regex_match(value, re)) {
                    return "invalid URL";
                }
                return std::nullopt;
            };
        }

        /// Validator for IP addresses (v4)
        inline Validator ipv4() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"((\d{1,3}\.){3}\d{1,3})");
                if (!std::regex_match(value, re)) {
                    return "invalid IPv4 address";
                }
                std::istringstream iss(value);
                int octet;
                char dot;
                for (int i = 0; i < 4; ++i) {
                    if (i > 0)
                        iss >> dot;
                    iss >> octet;
                    if (octet < 0 || octet > 255) {
                        return "invalid IPv4 address: octet out of range";
                    }
                }
                return std::nullopt;
            };
        }

        /// Validator for IP addresses (v6)
        inline Validator ipv6() {
            return [](const std::string &value) -> std::optional<std::string> {
                // Simplified IPv6 regex
                std::regex re(R"(([0-9a-fA-F]{0,4}:){2,7}[0-9a-fA-F]{0,4})");
                if (!std::regex_match(value, re)) {
                    return "invalid IPv6 address";
                }
                return std::nullopt;
            };
        }

        /// Validator for hostname
        inline Validator hostname() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (value.empty() || value.length() > 253) {
                    return "invalid hostname length";
                }
                std::regex re(
                    R"(^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
                if (!std::regex_match(value, re)) {
                    return "invalid hostname";
                }
                return std::nullopt;
            };
        }

        /// Validator for host:port format
        inline Validator host_port() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto colon_pos = value.rfind(':');
                if (colon_pos == std::string::npos) {
                    return "value must be in host:port format";
                }
                std::string host = value.substr(0, colon_pos);
                std::string port_str = value.substr(colon_pos + 1);

                auto hostname_err = hostname()(host);
                if (hostname_err)
                    return hostname_err;

                auto port_err = port()(port_str);
                if (port_err)
                    return port_err;

                return std::nullopt;
            };
        }

        // ============= Semantic Version Validators =============

        /// Validator for semantic version (e.g., "1.2.3", "1.0.0-beta")
        inline Validator semver() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(
                    R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)");
                if (!std::regex_match(value, re)) {
                    return "invalid semantic version (expected X.Y.Z format)";
                }
                return std::nullopt;
            };
        }

        // ============= Date/Time Validators =============

        /// Validator for date in YYYY-MM-DD format
        inline Validator date_iso() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12]\d|3[01])$)");
                if (!std::regex_match(value, re)) {
                    return "invalid date (expected YYYY-MM-DD format)";
                }
                return std::nullopt;
            };
        }

        /// Validator for time in HH:MM:SS format
        inline Validator time_iso() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^([01]\d|2[0-3]):([0-5]\d):([0-5]\d)$)");
                if (!std::regex_match(value, re)) {
                    return "invalid time (expected HH:MM:SS format)";
                }
                return std::nullopt;
            };
        }

        /// Validator for duration (e.g., "1h30m", "30s", "1d")
        inline Validator duration() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^(\d+d)?(\d+h)?(\d+m)?(\d+s)?(\d+ms)?$)");
                if (value.empty() || !std::regex_match(value, re)) {
                    return "invalid duration (expected format like 1h30m, 30s, 1d)";
                }
                return std::nullopt;
            };
        }

        // ============= UUID Validators =============

        /// Validator for UUID (any version)
        inline Validator uuid() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
                if (!std::regex_match(value, re)) {
                    return "invalid UUID format (expected xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)";
                }
                return std::nullopt;
            };
        }

        /// Validator for UUID v4 (random)
        inline Validator uuid_v4() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(
                    R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-4[0-9a-fA-F]{3}-[89abAB][0-9a-fA-F]{3}-[0-9a-fA-F]{12}$)");
                if (!std::regex_match(value, re)) {
                    return "invalid UUID v4 format";
                }
                return std::nullopt;
            };
        }

        // ============= Color Validators =============

        /// Validator for hex color (e.g., #FF0000, #f00)
        inline Validator hex_color() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^#([0-9a-fA-F]{3}|[0-9a-fA-F]{6})$)");
                if (!std::regex_match(value, re)) {
                    return "invalid hex color (expected #RGB or #RRGGBB)";
                }
                return std::nullopt;
            };
        }

        // ============= JSON/Data Format Validators =============

        /// Validator for JSON-like object notation (basic check)
        inline Validator json_object() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (value.empty() || value.front() != '{' || value.back() != '}') {
                    return "value must be a JSON object (starts with '{' and ends with '}')";
                }
                return std::nullopt;
            };
        }

        /// Validator for JSON-like array notation (basic check)
        inline Validator json_array() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (value.empty() || value.front() != '[' || value.back() != ']') {
                    return "value must be a JSON array (starts with '[' and ends with ']')";
                }
                return std::nullopt;
            };
        }

        // ============= Identifier Validators =============

        /// Validator for C/C++ style identifier
        inline Validator identifier() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (value.empty())
                    return "identifier cannot be empty";
                if (!std::isalpha(static_cast<unsigned char>(value[0])) && value[0] != '_') {
                    return "identifier must start with a letter or underscore";
                }
                for (char c : value) {
                    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                        return "identifier must contain only letters, digits, and underscores";
                    }
                }
                return std::nullopt;
            };
        }

        /// Validator for kebab-case
        inline Validator kebab_case() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^[a-z][a-z0-9]*(-[a-z0-9]+)*$)");
                if (!std::regex_match(value, re)) {
                    return "value must be in kebab-case format";
                }
                return std::nullopt;
            };
        }

        /// Validator for snake_case
        inline Validator snake_case() {
            return [](const std::string &value) -> std::optional<std::string> {
                std::regex re(R"(^[a-z][a-z0-9]*(_[a-z0-9]+)*$)");
                if (!std::regex_match(value, re)) {
                    return "value must be in snake_case format";
                }
                return std::nullopt;
            };
        }

        // ============= Composite Validators =============

        /// Validator that applies multiple validators
        inline Validator all_of(std::initializer_list<Validator> validators) {
            std::vector<Validator> vals(validators);
            return [vals](const std::string &value) -> std::optional<std::string> {
                for (const auto &v : vals) {
                    auto error = v(value);
                    if (error)
                        return error;
                }
                return std::nullopt;
            };
        }

        /// Validator that passes if any validator passes
        inline Validator any_of(std::initializer_list<Validator> validators) {
            std::vector<Validator> vals(validators);
            return [vals](const std::string &value) -> std::optional<std::string> {
                std::string errors;
                for (const auto &v : vals) {
                    auto error = v(value);
                    if (!error)
                        return std::nullopt;
                    if (!errors.empty())
                        errors += "; ";
                    errors += *error;
                }
                return "none of the validators passed: " + errors;
            };
        }

        /// Validator that inverts another validator
        inline Validator not_validator(Validator validator, const std::string &error_msg = "") {
            return [validator, error_msg](const std::string &value) -> std::optional<std::string> {
                auto error = validator(value);
                if (!error) {
                    return error_msg.empty() ? "validation unexpectedly passed" : error_msg;
                }
                return std::nullopt;
            };
        }

        /// Create a custom validator with a predicate
        template <typename Pred> Validator predicate(Pred pred, const std::string &error_msg) {
            return [pred, error_msg](const std::string &value) -> std::optional<std::string> {
                if (!pred(value)) {
                    return error_msg;
                }
                return std::nullopt;
            };
        }

        // ============= Transforming Validators =============

        /// Validator that transforms the value before other validations
        inline Validator lowercase_before(Validator next) {
            return [next](const std::string &value) -> std::optional<std::string> {
                std::string lower = value;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return next(lower);
            };
        }

        /// Validator that trims whitespace before validation
        inline Validator trim_before(Validator next) {
            return [next](const std::string &value) -> std::optional<std::string> {
                std::string trimmed = value;
                trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                auto pos = trimmed.find_last_not_of(" \t\n\r");
                if (pos != std::string::npos) {
                    trimmed.erase(pos + 1);
                }
                return next(trimmed);
            };
        }

    } // namespace validators

    // ============= Transformers =============

    namespace transformers {

        /// Transform to lowercase
        inline Transformer lowercase() {
            return [](const std::string &value) -> std::string {
                std::string result = value;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return result;
            };
        }

        /// Transform to uppercase
        inline Transformer uppercase() {
            return [](const std::string &value) -> std::string {
                std::string result = value;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
                return result;
            };
        }

        /// Trim whitespace
        inline Transformer trim() {
            return [](const std::string &value) -> std::string {
                std::string result = value;
                result.erase(0, result.find_first_not_of(" \t\n\r"));
                auto pos = result.find_last_not_of(" \t\n\r");
                if (pos != std::string::npos) {
                    result.erase(pos + 1);
                }
                return result;
            };
        }

        /// Replace substring
        inline Transformer replace(const std::string &from, const std::string &to) {
            return [from, to](const std::string &value) -> std::string {
                std::string result = value;
                std::size_t pos = 0;
                while ((pos = result.find(from, pos)) != std::string::npos) {
                    result.replace(pos, from.length(), to);
                    pos += to.length();
                }
                return result;
            };
        }

        /// Add prefix if not present
        inline Transformer ensure_prefix(const std::string &prefix) {
            return [prefix](const std::string &value) -> std::string {
                if (value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix) {
                    return value;
                }
                return prefix + value;
            };
        }

        /// Add suffix if not present
        inline Transformer ensure_suffix(const std::string &suffix) {
            return [suffix](const std::string &value) -> std::string {
                if (value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix) {
                    return value;
                }
                return value + suffix;
            };
        }

        /// Expand ~ to home directory
        inline Transformer expand_tilde() {
            return [](const std::string &value) -> std::string {
                if (!value.empty() && value[0] == '~') {
                    const char *home = std::getenv("HOME");
                    if (!home)
                        home = std::getenv("USERPROFILE"); // Windows
                    if (home) {
                        return std::string(home) + value.substr(1);
                    }
                }
                return value;
            };
        }

        /// Bound/clamp a numeric value to a range (instead of rejecting)
        template <typename T> Transformer bounded(T min_val, T max_val) {
            return [min_val, max_val](const std::string &value) -> std::string {
                auto converted = detail::Converter<T>::convert(value);
                if (!converted) {
                    return value; // Return as-is if not a valid number
                }
                T clamped = std::max(min_val, std::min(max_val, *converted));
                if constexpr (std::is_floating_point_v<T>) {
                    std::ostringstream oss;
                    oss << clamped;
                    return oss.str();
                } else {
                    return std::to_string(clamped);
                }
            };
        }

        /// Normalize path separators (convert \ to / on all platforms)
        inline Transformer normalize_path() {
            return [](const std::string &value) -> std::string {
                std::string result = value;
                std::replace(result.begin(), result.end(), '\\', '/');
                return result;
            };
        }

        /// Split by delimiter and return first N items joined
        inline Transformer limit_items(char delimiter, std::size_t max_items) {
            return [delimiter, max_items](const std::string &value) -> std::string {
                std::string result;
                std::size_t count = 0;
                std::size_t start = 0;
                std::size_t pos = 0;
                while ((pos = value.find(delimiter, start)) != std::string::npos && count < max_items) {
                    if (!result.empty())
                        result += delimiter;
                    result += value.substr(start, pos - start);
                    start = pos + 1;
                    ++count;
                }
                if (count < max_items && start < value.size()) {
                    if (!result.empty())
                        result += delimiter;
                    result += value.substr(start);
                }
                return result;
            };
        }

        /// Map values to other values
        inline Transformer map_value(const std::vector<std::pair<std::string, std::string>> &mappings) {
            return [mappings](const std::string &value) -> std::string {
                for (const auto &[from, to] : mappings) {
                    if (value == from)
                        return to;
                }
                return value;
            };
        }

        /// Default value if empty
        inline Transformer default_if_empty(const std::string &default_val) {
            return
                [default_val](const std::string &value) -> std::string { return value.empty() ? default_val : value; };
        }

    } // namespace transformers

} // namespace argu
