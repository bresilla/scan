#pragma once

/// @file argu/parsers/size.hpp
/// @brief Human-friendly size parsing (10KB, 5MB, 1GB, 500MiB)

#include <argu/core/types.hpp>

#include <cstdint>
#include <optional>
#include <regex>
#include <string>

namespace argu {
    namespace parsers {

        /// Size units (both SI and binary)
        enum class SizeUnit {
            Bytes,
            // SI units (base 1000)
            KB, // Kilobyte  = 1000
            MB, // Megabyte  = 1000^2
            GB, // Gigabyte  = 1000^3
            TB, // Terabyte  = 1000^4
            PB, // Petabyte  = 1000^5
            // Binary units (base 1024)
            KiB, // Kibibyte  = 1024
            MiB, // Mebibyte  = 1024^2
            GiB, // Gibibyte  = 1024^3
            TiB, // Tebibyte  = 1024^4
            PiB, // Pebibyte  = 1024^5
        };

        /// Parse a human-friendly size string to bytes
        /// Supports: B, KB, MB, GB, TB, PB (SI, base 1000)
        ///           KiB, MiB, GiB, TiB, PiB (binary, base 1024)
        ///           K, M, G, T, P (treated as binary for compatibility)
        inline std::optional<uint64_t> parse_size(const std::string &str) {
            if (str.empty()) {
                return std::nullopt;
            }

            // Check for pure numeric (interpreted as bytes)
            bool all_digits = true;
            for (char c : str) {
                if (!std::isdigit(static_cast<unsigned char>(c)) && c != '.') {
                    all_digits = false;
                    break;
                }
            }
            if (all_digits) {
                try {
                    return static_cast<uint64_t>(std::stoull(str));
                } catch (...) {
                    return std::nullopt;
                }
            }

            // Parse with unit
            std::regex pattern(R"(^\s*(\d+(?:\.\d+)?)\s*([A-Za-z]+)?\s*$)");
            std::smatch match;

            if (!std::regex_match(str, match, pattern)) {
                return std::nullopt;
            }

            double value = std::stod(match[1].str());
            std::string unit = match[2].str();

            // Normalize unit to uppercase for comparison
            for (auto &c : unit) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }

            uint64_t multiplier = 1;

            // Binary units (base 1024)
            if (unit.empty() || unit == "B" || unit == "BYTE" || unit == "BYTES") {
                multiplier = 1;
            } else if (unit == "KIB" || unit == "K" || unit == "KI") {
                multiplier = 1024ULL;
            } else if (unit == "MIB" || unit == "M" || unit == "MI") {
                multiplier = 1024ULL * 1024;
            } else if (unit == "GIB" || unit == "G" || unit == "GI") {
                multiplier = 1024ULL * 1024 * 1024;
            } else if (unit == "TIB" || unit == "T" || unit == "TI") {
                multiplier = 1024ULL * 1024 * 1024 * 1024;
            } else if (unit == "PIB" || unit == "P" || unit == "PI") {
                multiplier = 1024ULL * 1024 * 1024 * 1024 * 1024;
            }
            // SI units (base 1000)
            else if (unit == "KB") {
                multiplier = 1000ULL;
            } else if (unit == "MB") {
                multiplier = 1000ULL * 1000;
            } else if (unit == "GB") {
                multiplier = 1000ULL * 1000 * 1000;
            } else if (unit == "TB") {
                multiplier = 1000ULL * 1000 * 1000 * 1000;
            } else if (unit == "PB") {
                multiplier = 1000ULL * 1000 * 1000 * 1000 * 1000;
            } else {
                return std::nullopt; // Unknown unit
            }

            return static_cast<uint64_t>(value * static_cast<double>(multiplier));
        }

        /// Format bytes as human-readable string (binary units)
        inline std::string format_size(uint64_t bytes, bool use_binary = true) {
            const char *units_binary[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
            const char *units_si[] = {"B", "KB", "MB", "GB", "TB", "PB"};
            const char **units = use_binary ? units_binary : units_si;
            uint64_t base = use_binary ? 1024 : 1000;

            double size = static_cast<double>(bytes);
            int unit_index = 0;

            while (size >= static_cast<double>(base) && unit_index < 5) {
                size /= static_cast<double>(base);
                unit_index++;
            }

            // Format with appropriate precision
            char buffer[64];
            if (unit_index == 0) {
                std::snprintf(buffer, sizeof(buffer), "%llu%s", static_cast<unsigned long long>(bytes), units[0]);
            } else if (size >= 100) {
                std::snprintf(buffer, sizeof(buffer), "%.0f%s", size, units[unit_index]);
            } else if (size >= 10) {
                std::snprintf(buffer, sizeof(buffer), "%.1f%s", size, units[unit_index]);
            } else {
                std::snprintf(buffer, sizeof(buffer), "%.2f%s", size, units[unit_index]);
            }

            return buffer;
        }

    } // namespace parsers

    namespace validators {

        /// Validator for size strings
        inline Validator size() {
            return [](const std::string &value) -> std::optional<std::string> {
                auto result = parsers::parse_size(value);
                if (!result) {
                    return "invalid size format (expected: 10B, 5KB, 2MB, 1GB, or binary like 5KiB, 2MiB)";
                }
                return std::nullopt;
            };
        }

        /// Validator for size with minimum
        inline Validator size_min(uint64_t min_bytes) {
            return [min_bytes](const std::string &value) -> std::optional<std::string> {
                auto result = parsers::parse_size(value);
                if (!result) {
                    return "invalid size format";
                }
                if (*result < min_bytes) {
                    return "size must be at least " + parsers::format_size(min_bytes);
                }
                return std::nullopt;
            };
        }

        /// Validator for size with maximum
        inline Validator size_max(uint64_t max_bytes) {
            return [max_bytes](const std::string &value) -> std::optional<std::string> {
                auto result = parsers::parse_size(value);
                if (!result) {
                    return "invalid size format";
                }
                if (*result > max_bytes) {
                    return "size must be at most " + parsers::format_size(max_bytes);
                }
                return std::nullopt;
            };
        }

        /// Validator for size within range
        inline Validator size_range(uint64_t min_bytes, uint64_t max_bytes) {
            return [min_bytes, max_bytes](const std::string &value) -> std::optional<std::string> {
                auto result = parsers::parse_size(value);
                if (!result) {
                    return "invalid size format";
                }
                if (*result < min_bytes || *result > max_bytes) {
                    return "size must be between " + parsers::format_size(min_bytes) + " and " +
                           parsers::format_size(max_bytes);
                }
                return std::nullopt;
            };
        }

    } // namespace validators

    namespace transformers {

        /// Transform size string to bytes (as string)
        inline Transformer size_to_bytes() {
            return [](const std::string &value) -> std::string {
                auto result = parsers::parse_size(value);
                if (result) {
                    return std::to_string(*result);
                }
                return value; // Return original if parsing fails
            };
        }

    } // namespace transformers

} // namespace argu
