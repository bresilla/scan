#pragma once

/// @file fuzzy.hpp
/// @brief Fuzzy matching algorithm for filtering

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace scan::fuzzy {

    /// Match result with score and match positions
    struct MatchResult {
        bool matched = false;
        int score = 0;
        std::vector<size_t> positions; // Indices of matched characters
    };

    /// Convert string to lowercase
    inline std::string to_lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /// Fuzzy match a pattern against a target string
    /// Returns match result with score and positions of matching characters
    ///
    /// Scoring:
    ///   - Exact match: +100
    ///   - Prefix match: +50
    ///   - Contains match: +25
    ///   - Consecutive characters: +10 each
    ///   - Start of word: +5
    ///   - Case match: +1
    inline MatchResult match(const std::string &pattern, const std::string &target, bool case_sensitive = false) {
        MatchResult result;

        if (pattern.empty()) {
            result.matched = true;
            result.score = 0;
            return result;
        }

        if (target.empty()) {
            return result;
        }

        std::string p = case_sensitive ? pattern : to_lower(pattern);
        std::string t = case_sensitive ? target : to_lower(target);

        // Exact match bonus
        if (t == p) {
            result.matched = true;
            result.score = 100;
            for (size_t i = 0; i < target.size(); i++) {
                result.positions.push_back(i);
            }
            return result;
        }

        // Prefix match bonus
        if (t.find(p) == 0) {
            result.matched = true;
            result.score = 50;
            for (size_t i = 0; i < pattern.size(); i++) {
                result.positions.push_back(i);
            }
            return result;
        }

        // Contains match bonus
        size_t pos = t.find(p);
        if (pos != std::string::npos) {
            result.matched = true;
            result.score = 25;
            for (size_t i = 0; i < pattern.size(); i++) {
                result.positions.push_back(pos + i);
            }
            return result;
        }

        // Fuzzy match - find all pattern characters in order
        size_t pattern_idx = 0;
        size_t last_match = SIZE_MAX;
        int consecutive = 0;

        for (size_t i = 0; i < t.size() && pattern_idx < p.size(); i++) {
            if (t[i] == p[pattern_idx]) {
                result.positions.push_back(i);

                // Consecutive character bonus
                if (last_match != SIZE_MAX && i == last_match + 1) {
                    consecutive++;
                    result.score += 10;
                } else {
                    consecutive = 0;
                }

                // Start of word bonus (after space, underscore, dash, or capital)
                if (i == 0 || target[i - 1] == ' ' || target[i - 1] == '_' || target[i - 1] == '-' ||
                    (i > 0 && std::isupper(target[i]) && std::islower(target[i - 1]))) {
                    result.score += 5;
                }

                // Case match bonus
                if (!case_sensitive && pattern[pattern_idx] == target[i]) {
                    result.score += 1;
                }

                last_match = i;
                pattern_idx++;
            }
        }

        // Check if all pattern characters were found
        result.matched = (pattern_idx == p.size());

        if (result.matched) {
            // Base score for fuzzy match
            result.score += 10;

            // Penalty for gaps (positions spread out)
            if (result.positions.size() > 1) {
                size_t total_gap = result.positions.back() - result.positions.front() - (result.positions.size() - 1);
                result.score -= static_cast<int>(total_gap);
            }

            // Bonus for matching at start
            if (!result.positions.empty() && result.positions[0] == 0) {
                result.score += 15;
            }
        }

        return result;
    }

    /// Filter a list of items by fuzzy matching
    /// Returns indices sorted by score (best matches first)
    inline std::vector<size_t> filter(const std::vector<std::string> &items, const std::string &query,
                                      bool case_sensitive = false) {
        if (query.empty()) {
            std::vector<size_t> all;
            for (size_t i = 0; i < items.size(); i++) {
                all.push_back(i);
            }
            return all;
        }

        std::vector<std::pair<size_t, int>> scored;

        for (size_t i = 0; i < items.size(); i++) {
            auto result = match(query, items[i], case_sensitive);
            if (result.matched) {
                scored.push_back({i, result.score});
            }
        }

        // Sort by score descending
        std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

        std::vector<size_t> indices;
        for (const auto &[idx, score] : scored) {
            indices.push_back(idx);
        }

        return indices;
    }

    /// Get match positions for highlighting
    inline std::vector<size_t> get_match_positions(const std::string &pattern, const std::string &target,
                                                   bool case_sensitive = false) {
        auto result = match(pattern, target, case_sensitive);
        return result.positions;
    }

} // namespace scan::fuzzy
