#pragma once

/// @file argu/core/levenshtein.hpp
/// @brief Levenshtein distance calculation for typo suggestions

#include <algorithm>
#include <string>
#include <vector>

namespace argu {
    namespace detail {

        /// Calculate Levenshtein edit distance between two strings
        inline std::size_t levenshtein_distance(const std::string &s1, const std::string &s2) {
            const std::size_t m = s1.size();
            const std::size_t n = s2.size();

            if (m == 0)
                return n;
            if (n == 0)
                return m;

            std::vector<std::vector<std::size_t>> dp(m + 1, std::vector<std::size_t>(n + 1));

            for (std::size_t i = 0; i <= m; ++i)
                dp[i][0] = i;
            for (std::size_t j = 0; j <= n; ++j)
                dp[0][j] = j;

            for (std::size_t i = 1; i <= m; ++i) {
                for (std::size_t j = 1; j <= n; ++j) {
                    std::size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                    dp[i][j] = std::min({
                        dp[i - 1][j] + 1,       // deletion
                        dp[i][j - 1] + 1,       // insertion
                        dp[i - 1][j - 1] + cost // substitution
                    });
                }
            }

            return dp[m][n];
        }

        /// Calculate Damerau-Levenshtein distance (includes transpositions)
        inline std::size_t damerau_levenshtein_distance(const std::string &s1, const std::string &s2) {
            const std::size_t m = s1.size();
            const std::size_t n = s2.size();

            if (m == 0)
                return n;
            if (n == 0)
                return m;

            std::vector<std::vector<std::size_t>> dp(m + 1, std::vector<std::size_t>(n + 1));

            for (std::size_t i = 0; i <= m; ++i)
                dp[i][0] = i;
            for (std::size_t j = 0; j <= n; ++j)
                dp[0][j] = j;

            for (std::size_t i = 1; i <= m; ++i) {
                for (std::size_t j = 1; j <= n; ++j) {
                    std::size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

                    dp[i][j] = std::min({
                        dp[i - 1][j] + 1,       // deletion
                        dp[i][j - 1] + 1,       // insertion
                        dp[i - 1][j - 1] + cost // substitution
                    });

                    // Transposition
                    if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                        dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + cost);
                    }
                }
            }

            return dp[m][n];
        }

        /// Find closest matches from a list of candidates
        /// Returns suggestions with edit distance <= max_distance, sorted by distance
        inline std::vector<std::string> find_closest_matches(const std::string &input,
                                                             const std::vector<std::string> &candidates,
                                                             std::size_t max_distance = 3,
                                                             std::size_t max_results = 3) {
            std::vector<std::pair<std::size_t, std::string>> scored;

            for (const auto &candidate : candidates) {
                std::size_t dist = damerau_levenshtein_distance(input, candidate);
                if (dist <= max_distance && dist > 0) { // Exclude exact matches
                    scored.emplace_back(dist, candidate);
                }
            }

            // Sort by distance
            std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

            // Return top results
            std::vector<std::string> result;
            for (std::size_t i = 0; i < std::min(max_results, scored.size()); ++i) {
                result.push_back(scored[i].second);
            }

            return result;
        }

        /// Check if input looks like a flag prefix typo
        /// e.g., "--hepl" might be "--help"
        inline std::vector<std::string> suggest_flags(const std::string &input,
                                                      const std::vector<std::string> &short_flags,
                                                      const std::vector<std::string> &long_flags) {
            std::vector<std::string> suggestions;

            if (input.size() >= 2 && input[0] == '-' && input[1] == '-') {
                // Long flag
                std::string name = input.substr(2);
                auto matches = find_closest_matches(name, long_flags);
                for (const auto &m : matches) {
                    suggestions.push_back("--" + m);
                }
            } else if (input.size() >= 1 && input[0] == '-') {
                // Short flag (less likely to be mistyped, but check anyway)
                std::string name = input.substr(1);
                auto matches = find_closest_matches(name, short_flags);
                for (const auto &m : matches) {
                    suggestions.push_back("-" + m);
                }
                // Also check if they meant a long flag
                auto long_matches = find_closest_matches(name, long_flags);
                for (const auto &m : long_matches) {
                    suggestions.push_back("--" + m);
                }
            } else {
                // Might be a subcommand typo
                std::vector<std::string> all_options;
                for (const auto &f : short_flags)
                    all_options.push_back("-" + f);
                for (const auto &f : long_flags)
                    all_options.push_back("--" + f);
                suggestions = find_closest_matches(input, all_options);
            }

            return suggestions;
        }

    } // namespace detail
} // namespace argu
