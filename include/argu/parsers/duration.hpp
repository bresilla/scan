#pragma once

/// @file argu/parsers/duration.hpp
/// @brief Human-friendly duration parsing (5s, 10m, 2h, 1d)

#include <chrono>
#include <optional>
#include <regex>
#include <string>

namespace argu {
    namespace parsers {

        /// Parse a human-friendly duration string
        /// Supports: ns, us, ms, s, m, h, d (and combinations like "2h30m")
        /// Returns duration in the specified unit (default: seconds)
        template <typename Duration = std::chrono::seconds>
        std::optional<Duration> parse_duration(const std::string &str) {
            if (str.empty()) {
                return std::nullopt;
            }

            // Check for pure numeric (interpreted as default unit)
            bool all_digits = true;
            for (char c : str) {
                if (!std::isdigit(static_cast<unsigned char>(c)) && c != '.' && c != '-') {
                    all_digits = false;
                    break;
                }
            }
            if (all_digits) {
                try {
                    double val = std::stod(str);
                    return std::chrono::duration_cast<Duration>(
                        std::chrono::duration<double, typename Duration::period>(val));
                } catch (...) {
                    return std::nullopt;
                }
            }

            // Parse compound duration like "2h30m45s"
            std::regex pattern(R"((\d+(?:\.\d+)?)\s*(ns|us|µs|ms|s|m|h|d))");
            std::smatch match;
            std::string remaining = str;

            std::chrono::nanoseconds total{0};
            bool found_any = false;

            while (std::regex_search(remaining, match, pattern)) {
                found_any = true;
                double value = std::stod(match[1].str());
                std::string unit = match[2].str();

                std::chrono::nanoseconds part{0};
                if (unit == "ns") {
                    part = std::chrono::nanoseconds(static_cast<int64_t>(value));
                } else if (unit == "us" || unit == "µs") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<double, std::micro>(value));
                } else if (unit == "ms") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<double, std::milli>(value));
                } else if (unit == "s") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(value));
                } else if (unit == "m") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<double, std::ratio<60>>(value));
                } else if (unit == "h") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<double, std::ratio<3600>>(value));
                } else if (unit == "d") {
                    part = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<double, std::ratio<86400>>(value));
                }

                total += part;
                remaining = match.suffix().str();
            }

            if (!found_any) {
                return std::nullopt;
            }

            return std::chrono::duration_cast<Duration>(total);
        }

        /// Parse duration to seconds (convenience function)
        inline std::optional<int64_t> parse_duration_seconds(const std::string &str) {
            auto result = parse_duration<std::chrono::seconds>(str);
            if (result) {
                return result->count();
            }
            return std::nullopt;
        }

        /// Parse duration to milliseconds (convenience function)
        inline std::optional<int64_t> parse_duration_ms(const std::string &str) {
            auto result = parse_duration<std::chrono::milliseconds>(str);
            if (result) {
                return result->count();
            }
            return std::nullopt;
        }

        /// Format a duration as human-readable string
        template <typename Duration> std::string format_duration(Duration dur) {
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();

            if (ns == 0)
                return "0s";

            std::string result;

            // Days
            auto days = ns / (86400LL * 1000000000LL);
            if (days > 0) {
                result += std::to_string(days) + "d";
                ns %= (86400LL * 1000000000LL);
            }

            // Hours
            auto hours = ns / (3600LL * 1000000000LL);
            if (hours > 0) {
                result += std::to_string(hours) + "h";
                ns %= (3600LL * 1000000000LL);
            }

            // Minutes
            auto minutes = ns / (60LL * 1000000000LL);
            if (minutes > 0) {
                result += std::to_string(minutes) + "m";
                ns %= (60LL * 1000000000LL);
            }

            // Seconds
            auto seconds = ns / 1000000000LL;
            if (seconds > 0 || result.empty()) {
                result += std::to_string(seconds) + "s";
                ns %= 1000000000LL;
            }

            // Milliseconds (only if no larger units and remaining)
            if (result.empty() && ns > 0) {
                auto ms = ns / 1000000LL;
                if (ms > 0) {
                    result = std::to_string(ms) + "ms";
                } else {
                    auto us = ns / 1000LL;
                    if (us > 0) {
                        result = std::to_string(us) + "us";
                    } else {
                        result = std::to_string(ns) + "ns";
                    }
                }
            }

            return result;
        }

    } // namespace parsers

    namespace transformers {

        /// Transform duration string to seconds (as string)
        inline Transformer duration_to_seconds() {
            return [](const std::string &value) -> std::string {
                auto result = parsers::parse_duration_seconds(value);
                if (result) {
                    return std::to_string(*result);
                }
                return value; // Return original if parsing fails (let validator handle it)
            };
        }

        /// Transform duration string to milliseconds (as string)
        inline Transformer duration_to_ms() {
            return [](const std::string &value) -> std::string {
                auto result = parsers::parse_duration_ms(value);
                if (result) {
                    return std::to_string(*result);
                }
                return value;
            };
        }

    } // namespace transformers

} // namespace argu
