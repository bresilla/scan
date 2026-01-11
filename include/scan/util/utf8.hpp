#pragma once

/// @file utf8.hpp
/// @brief UTF-8 string utilities

#include <cstdint>
#include <string>
#include <vector>

namespace scan::utf8 {

    /// Get the number of bytes in a UTF-8 character from its first byte
    inline int char_length(unsigned char c) {
        if ((c & 0x80) == 0)
            return 1; // 0xxxxxxx
        if ((c & 0xE0) == 0xC0)
            return 2; // 110xxxxx
        if ((c & 0xF0) == 0xE0)
            return 3; // 1110xxxx
        if ((c & 0xF8) == 0xF0)
            return 4; // 11110xxx
        return 1;     // Invalid, treat as single byte
    }

    /// Check if a byte is a UTF-8 continuation byte
    inline bool is_continuation(unsigned char c) { return (c & 0xC0) == 0x80; }

    /// Count the number of Unicode codepoints in a UTF-8 string
    inline size_t length(const std::string &s) {
        size_t count = 0;
        for (size_t i = 0; i < s.size();) {
            i += char_length(static_cast<unsigned char>(s[i]));
            count++;
        }
        return count;
    }

    /// Get the display width of a string (accounting for wide characters)
    /// This is a simplified version - full implementation would need Unicode tables
    inline size_t display_width(const std::string &s) {
        size_t width = 0;
        for (size_t i = 0; i < s.size();) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            int len = char_length(c);

            // Decode codepoint
            char32_t cp = 0;
            if (len == 1) {
                cp = c;
            } else if (len == 2) {
                cp = (c & 0x1F) << 6;
                if (i + 1 < s.size())
                    cp |= (s[i + 1] & 0x3F);
            } else if (len == 3) {
                cp = (c & 0x0F) << 12;
                if (i + 1 < s.size())
                    cp |= (s[i + 1] & 0x3F) << 6;
                if (i + 2 < s.size())
                    cp |= (s[i + 2] & 0x3F);
            } else if (len == 4) {
                cp = (c & 0x07) << 18;
                if (i + 1 < s.size())
                    cp |= (s[i + 1] & 0x3F) << 12;
                if (i + 2 < s.size())
                    cp |= (s[i + 2] & 0x3F) << 6;
                if (i + 3 < s.size())
                    cp |= (s[i + 3] & 0x3F);
            }

            // Estimate width (simplified)
            // CJK characters are typically double-width
            if (cp >= 0x1100 && (cp <= 0x115F ||                      // Hangul Jamo
                                 cp == 0x2329 || cp == 0x232A ||      // Angle brackets
                                 (cp >= 0x2E80 && cp <= 0xA4CF) ||    // CJK
                                 (cp >= 0xAC00 && cp <= 0xD7A3) ||    // Hangul syllables
                                 (cp >= 0xF900 && cp <= 0xFAFF) ||    // CJK compatibility
                                 (cp >= 0xFE10 && cp <= 0xFE1F) ||    // Vertical forms
                                 (cp >= 0xFE30 && cp <= 0xFE6F) ||    // CJK compatibility forms
                                 (cp >= 0xFF00 && cp <= 0xFF60) ||    // Fullwidth forms
                                 (cp >= 0xFFE0 && cp <= 0xFFE6) ||    // Fullwidth symbols
                                 (cp >= 0x20000 && cp <= 0x2FFFF))) { // CJK Extension B+
                width += 2;
            } else if (cp < 32 || (cp >= 0x7F && cp < 0xA0)) {
                // Control characters have zero width
                width += 0;
            } else {
                width += 1;
            }

            i += len;
        }
        return width;
    }

    /// Decode a UTF-8 string into codepoints
    inline std::vector<char32_t> decode(const std::string &s) {
        std::vector<char32_t> result;
        for (size_t i = 0; i < s.size();) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            int len = char_length(c);
            char32_t cp = 0;

            if (len == 1) {
                cp = c;
            } else if (len == 2 && i + 1 < s.size()) {
                cp = ((c & 0x1F) << 6) | (s[i + 1] & 0x3F);
            } else if (len == 3 && i + 2 < s.size()) {
                cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
            } else if (len == 4 && i + 3 < s.size()) {
                cp = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) | ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F);
            }

            result.push_back(cp);
            i += len;
        }
        return result;
    }

    /// Encode a codepoint to UTF-8
    inline std::string encode(char32_t cp) {
        std::string result;
        if (cp < 0x80) {
            result += static_cast<char>(cp);
        } else if (cp < 0x800) {
            result += static_cast<char>(0xC0 | (cp >> 6));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            result += static_cast<char>(0xE0 | (cp >> 12));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            result += static_cast<char>(0xF0 | (cp >> 18));
            result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        }
        return result;
    }

    /// Encode a vector of codepoints to UTF-8
    inline std::string encode(const std::vector<char32_t> &codepoints) {
        std::string result;
        for (char32_t cp : codepoints) {
            result += encode(cp);
        }
        return result;
    }

    /// Get a substring by codepoint indices (not byte indices)
    inline std::string substring(const std::string &s, size_t start, size_t len = std::string::npos) {
        size_t byte_start = 0;
        size_t cp_index = 0;

        // Find start byte
        while (byte_start < s.size() && cp_index < start) {
            byte_start += char_length(static_cast<unsigned char>(s[byte_start]));
            cp_index++;
        }

        if (byte_start >= s.size())
            return "";

        // Find end byte
        size_t byte_end = byte_start;
        size_t cp_count = 0;
        while (byte_end < s.size() && (len == std::string::npos || cp_count < len)) {
            byte_end += char_length(static_cast<unsigned char>(s[byte_end]));
            cp_count++;
        }

        return s.substr(byte_start, byte_end - byte_start);
    }

    /// Insert a string at a codepoint position
    inline std::string insert(const std::string &s, size_t pos, const std::string &insert_str) {
        size_t byte_pos = 0;
        size_t cp_index = 0;

        while (byte_pos < s.size() && cp_index < pos) {
            byte_pos += char_length(static_cast<unsigned char>(s[byte_pos]));
            cp_index++;
        }

        std::string result = s.substr(0, byte_pos);
        result += insert_str;
        result += s.substr(byte_pos);
        return result;
    }

    /// Erase characters at codepoint positions
    inline std::string erase(const std::string &s, size_t pos, size_t count = 1) {
        size_t byte_start = 0;
        size_t cp_index = 0;

        while (byte_start < s.size() && cp_index < pos) {
            byte_start += char_length(static_cast<unsigned char>(s[byte_start]));
            cp_index++;
        }

        size_t byte_end = byte_start;
        size_t erased = 0;
        while (byte_end < s.size() && erased < count) {
            byte_end += char_length(static_cast<unsigned char>(s[byte_end]));
            erased++;
        }

        std::string result = s.substr(0, byte_start);
        result += s.substr(byte_end);
        return result;
    }

    /// Get the byte index for a codepoint index
    inline size_t byte_index(const std::string &s, size_t cp_index) {
        size_t byte_pos = 0;
        size_t current_cp = 0;

        while (byte_pos < s.size() && current_cp < cp_index) {
            byte_pos += char_length(static_cast<unsigned char>(s[byte_pos]));
            current_cp++;
        }

        return byte_pos;
    }

    /// Get the codepoint index for a byte index
    inline size_t codepoint_index(const std::string &s, size_t byte_pos) {
        size_t cp_index = 0;
        size_t current_byte = 0;

        while (current_byte < s.size() && current_byte < byte_pos) {
            current_byte += char_length(static_cast<unsigned char>(s[current_byte]));
            cp_index++;
        }

        return cp_index;
    }

} // namespace scan::utf8
