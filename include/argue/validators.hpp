#pragma once

/// @file validators.hpp
/// @brief Built-in validators for argument values

#include <argue/arg.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <regex>
#include <string>

namespace argue {

    /// Namespace for built-in validators
    namespace validators {

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

        /// Validator for email addresses (basic check)
        inline Validator email() {
            return [](const std::string &value) -> std::optional<std::string> {
                // Basic email regex
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
                // Additional range check
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

        /// Validator that checks if value is alphanumeric
        inline Validator alphanumeric() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isalnum(static_cast<unsigned char>(c)); })) {
                    return "value must be alphanumeric";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if value is alphabetic only
        inline Validator alphabetic() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isalpha(static_cast<unsigned char>(c)); })) {
                    return "value must contain only letters";
                }
                return std::nullopt;
            };
        }

        /// Validator that checks if value is numeric only
        inline Validator numeric() {
            return [](const std::string &value) -> std::optional<std::string> {
                if (!std::all_of(value.begin(), value.end(),
                                 [](char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
                    return "value must contain only digits";
                }
                return std::nullopt;
            };
        }

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

        /// Create a custom validator with a predicate
        template <typename Pred> Validator predicate(Pred pred, const std::string &error_msg) {
            return [pred, error_msg](const std::string &value) -> std::optional<std::string> {
                if (!pred(value)) {
                    return error_msg;
                }
                return std::nullopt;
            };
        }

        /// Validator that transforms the value before other validations (doesn't modify original)
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
                trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
                return next(trimmed);
            };
        }

    } // namespace validators

} // namespace argue
