#pragma once

/// @file style.hpp
/// @brief Lip Gloss-style text styling and layout utilities

#include <scan/style/theme.hpp>

#include <echo/echo.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace scan {

    /// Position for alignment
    enum class Position { Top, Bottom, Left, Right, Center };

    /// Border style for styled boxes
    enum class BorderStyle { None, Normal, Rounded, Double, Thick, Hidden };

    /// Get border characters for a style
    struct BorderChars {
        std::string top_left, top_right, bottom_left, bottom_right;
        std::string horizontal, vertical;
        std::string cross, t_down, t_up, t_left, t_right;
    };

    inline BorderChars get_border_chars(BorderStyle style) {
        switch (style) {
        case BorderStyle::Rounded:
            return {"╭", "╮", "╰", "╯", "─", "│", "┼", "┬", "┴", "┤", "├"};
        case BorderStyle::Double:
            return {"╔", "╗", "╚", "╝", "═", "║", "╬", "╦", "╩", "╣", "╠"};
        case BorderStyle::Thick:
            return {"┏", "┓", "┗", "┛", "━", "┃", "╋", "┳", "┻", "┫", "┣"};
        case BorderStyle::Hidden:
            return {" ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "};
        case BorderStyle::Normal:
        default:
            return {"┌", "┐", "└", "┘", "─", "│", "┼", "┬", "┴", "┤", "├"};
        }
    }

    /// Split string into lines
    inline std::vector<std::string> split_lines(const std::string &s) {
        std::vector<std::string> lines;
        std::istringstream iss(s);
        std::string line;
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }
        if (lines.empty()) {
            lines.push_back("");
        }
        return lines;
    }

    /// Get visible width of a string (accounting for ANSI codes and UTF-8)
    inline size_t visible_width(const std::string &s) {
        size_t width = 0;
        bool in_escape = false;

        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '\033') {
                in_escape = true;
            } else if (in_escape) {
                if (s[i] == 'm') {
                    in_escape = false;
                }
            } else {
                unsigned char c = static_cast<unsigned char>(s[i]);
                if ((c & 0xC0) != 0x80) { // Not a continuation byte
                    width++;
                }
            }
        }

        return width;
    }

    /// Get max line width in a multi-line string
    inline size_t max_line_width(const std::string &s) {
        auto lines = split_lines(s);
        size_t max_w = 0;
        for (const auto &line : lines) {
            max_w = std::max(max_w, visible_width(line));
        }
        return max_w;
    }

    /// Pad string to width (right pad)
    inline std::string pad_right(const std::string &s, size_t width) {
        size_t vis_width = visible_width(s);
        if (vis_width >= width)
            return s;
        return s + std::string(width - vis_width, ' ');
    }

    /// Pad string to width (left pad)
    inline std::string pad_left(const std::string &s, size_t width) {
        size_t vis_width = visible_width(s);
        if (vis_width >= width)
            return s;
        return std::string(width - vis_width, ' ') + s;
    }

    /// Center string within width
    inline std::string pad_center(const std::string &s, size_t width) {
        size_t vis_width = visible_width(s);
        if (vis_width >= width)
            return s;
        size_t total = width - vis_width;
        size_t left = total / 2;
        size_t right = total - left;
        return std::string(left, ' ') + s + std::string(right, ' ');
    }

    /// Truncate string to max width with ellipsis
    inline std::string truncate(const std::string &s, size_t max_width, const std::string &ellipsis = "...") {
        if (visible_width(s) <= max_width)
            return s;
        if (max_width <= ellipsis.length())
            return ellipsis.substr(0, max_width);

        // Simple truncation - find byte position
        size_t target = max_width - ellipsis.length();
        size_t width = 0;
        size_t byte_pos = 0;

        for (size_t i = 0; i < s.size() && width < target; i++) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if ((c & 0xC0) != 0x80) {
                width++;
            }
            byte_pos = i + 1;
        }

        return s.substr(0, byte_pos) + ellipsis;
    }

    /// Repeat a string n times
    inline std::string repeat(const std::string &s, size_t n) {
        std::string result;
        result.reserve(s.size() * n);
        for (size_t i = 0; i < n; i++) {
            result += s;
        }
        return result;
    }

    /// Style class for text styling (Lip Gloss style)
    class Style {
      public:
        Style() = default;
        Style(const std::string &s) : m_text(s) {}

        /// Set the string to style
        Style &set_string(const std::string &s) {
            m_text = s;
            return *this;
        }

        // ========== Colors ==========

        Style &foreground(const Color &c) {
            m_fg = c;
            m_has_fg = true;
            return *this;
        }

        Style &foreground(int r, int g, int b) { return foreground(Color{r, g, b}); }

        Style &background(const Color &c) {
            m_bg = c;
            m_has_bg = true;
            return *this;
        }

        Style &background(int r, int g, int b) { return background(Color{r, g, b}); }

        // ========== Text Styles ==========

        Style &bold(bool v = true) {
            m_bold = v;
            return *this;
        }
        Style &italic(bool v = true) {
            m_italic = v;
            return *this;
        }
        Style &underline(bool v = true) {
            m_underline = v;
            return *this;
        }
        Style &strikethrough(bool v = true) {
            m_strikethrough = v;
            return *this;
        }
        Style &faint(bool v = true) {
            m_faint = v;
            return *this;
        }
        Style &reverse(bool v = true) {
            m_reverse = v;
            return *this;
        }
        Style &blink(bool v = true) {
            m_blink = v;
            return *this;
        }

        // ========== Padding ==========

        Style &padding(int all) {
            m_padding_top = m_padding_bottom = m_padding_left = m_padding_right = all;
            return *this;
        }

        Style &padding(int vertical, int horizontal) {
            m_padding_top = m_padding_bottom = vertical;
            m_padding_left = m_padding_right = horizontal;
            return *this;
        }

        Style &padding(int top, int right, int bottom, int left) {
            m_padding_top = top;
            m_padding_right = right;
            m_padding_bottom = bottom;
            m_padding_left = left;
            return *this;
        }

        Style &padding_left(int v) {
            m_padding_left = v;
            return *this;
        }
        Style &padding_right(int v) {
            m_padding_right = v;
            return *this;
        }
        Style &padding_top(int v) {
            m_padding_top = v;
            return *this;
        }
        Style &padding_bottom(int v) {
            m_padding_bottom = v;
            return *this;
        }

        // ========== Margin ==========

        Style &margin(int all) {
            m_margin_top = m_margin_bottom = m_margin_left = m_margin_right = all;
            return *this;
        }

        Style &margin(int vertical, int horizontal) {
            m_margin_top = m_margin_bottom = vertical;
            m_margin_left = m_margin_right = horizontal;
            return *this;
        }

        Style &margin(int top, int right, int bottom, int left) {
            m_margin_top = top;
            m_margin_right = right;
            m_margin_bottom = bottom;
            m_margin_left = left;
            return *this;
        }

        Style &margin_left(int v) {
            m_margin_left = v;
            return *this;
        }
        Style &margin_right(int v) {
            m_margin_right = v;
            return *this;
        }
        Style &margin_top(int v) {
            m_margin_top = v;
            return *this;
        }
        Style &margin_bottom(int v) {
            m_margin_bottom = v;
            return *this;
        }

        // ========== Border ==========

        Style &border(BorderStyle style) {
            m_border_style = style;
            return *this;
        }

        Style &border_foreground(const Color &c) {
            m_border_fg = c;
            m_has_border_fg = true;
            return *this;
        }

        Style &border_foreground(int r, int g, int b) { return border_foreground(Color{r, g, b}); }

        // ========== Sizing ==========

        Style &width(int w) {
            m_width = w;
            return *this;
        }

        Style &height(int h) {
            m_height = h;
            return *this;
        }

        Style &max_width(int w) {
            m_max_width = w;
            return *this;
        }

        Style &max_height(int h) {
            m_max_height = h;
            return *this;
        }

        // ========== Alignment ==========

        Style &align(Position h, Position v = Position::Top) {
            m_align_h = h;
            m_align_v = v;
            return *this;
        }

        Style &align_horizontal(Position p) {
            m_align_h = p;
            return *this;
        }

        Style &align_vertical(Position p) {
            m_align_v = p;
            return *this;
        }

        // ========== Rendering ==========

        std::string render() const { return render(m_text); }

        std::string render(const std::string &s) const {
            auto lines = split_lines(s);

            // Calculate content width (before any styling)
            size_t content_width = 0;
            for (const auto &line : lines) {
                content_width = std::max(content_width, visible_width(line));
            }

            // Apply width
            size_t target_width = content_width;
            if (m_width > 0) {
                target_width = static_cast<size_t>(m_width);
            }
            if (m_max_width > 0 && target_width > static_cast<size_t>(m_max_width)) {
                target_width = static_cast<size_t>(m_max_width);
            }

            // Add padding to target width
            size_t padded_width = target_width + m_padding_left + m_padding_right;

            // Apply alignment and padding FIRST (before styling)
            for (auto &line : lines) {
                line = std::string(m_padding_left, ' ') + line;
                switch (m_align_h) {
                case Position::Center:
                    line = pad_center(line, padded_width);
                    break;
                case Position::Right:
                    line = pad_left(line, padded_width);
                    break;
                default:
                    line = pad_right(line, padded_width);
                    break;
                }
            }

            // Vertical padding
            std::string blank_line(padded_width, ' ');
            for (int i = 0; i < m_padding_top; i++) {
                lines.insert(lines.begin(), blank_line);
            }
            for (int i = 0; i < m_padding_bottom; i++) {
                lines.push_back(blank_line);
            }

            // Now apply all styling (fg, bg, bold, etc.) to the ENTIRE padded line
            for (auto &line : lines) {
                line = apply_all_styles(line);
            }

            // Border
            if (m_border_style != BorderStyle::None) {
                lines = add_border(lines, padded_width);
            }

            // Margin
            if (m_margin_left > 0) {
                std::string margin_str(m_margin_left, ' ');
                for (auto &line : lines) {
                    line = margin_str + line;
                }
            }
            for (int i = 0; i < m_margin_top; i++) {
                lines.insert(lines.begin(), "");
            }
            for (int i = 0; i < m_margin_bottom; i++) {
                lines.push_back("");
            }

            // Join
            std::string result;
            for (size_t i = 0; i < lines.size(); i++) {
                if (i > 0)
                    result += "\n";
                result += lines[i];
            }
            return result;
        }

        /// Render just the text with styling (no layout)
        std::string inline_render(const std::string &s) const { return apply_all_styles(s); }

        Style copy() const { return *this; }

      private:
        std::string m_text;

        Color m_fg{255, 255, 255};
        Color m_bg{0, 0, 0};
        bool m_has_fg = false;
        bool m_has_bg = false;

        bool m_bold = false;
        bool m_italic = false;
        bool m_underline = false;
        bool m_strikethrough = false;
        bool m_faint = false;
        bool m_reverse = false;
        bool m_blink = false;

        int m_padding_top = 0;
        int m_padding_right = 0;
        int m_padding_bottom = 0;
        int m_padding_left = 0;

        int m_margin_top = 0;
        int m_margin_right = 0;
        int m_margin_bottom = 0;
        int m_margin_left = 0;

        BorderStyle m_border_style = BorderStyle::None;
        Color m_border_fg{255, 255, 255};
        bool m_has_border_fg = false;

        int m_width = 0;
        int m_height = 0;
        int m_max_width = 0;
        int m_max_height = 0;

        Position m_align_h = Position::Left;
        Position m_align_v = Position::Top;

        /// Apply all styles (fg, bg, bold, etc.) to a string at once using echo::format::String
        std::string apply_all_styles(const std::string &s) const {
            bool has_any_style = m_has_fg || m_has_bg || m_bold || m_italic || m_underline || m_strikethrough ||
                                 m_faint || m_reverse || m_blink;

            if (!has_any_style) {
                return s;
            }

            echo::format::String styled(s);

            // Apply colors
            if (m_has_fg) {
                styled.fg(m_fg.r, m_fg.g, m_fg.b);
            }
            if (m_has_bg) {
                styled.bg(m_bg.r, m_bg.g, m_bg.b);
            }

            // Apply text styles
            if (m_bold)
                styled.bold();
            if (m_faint)
                styled.dim();
            if (m_italic)
                styled.italic();
            if (m_underline)
                styled.underline();
            if (m_blink)
                styled.blink();
            if (m_reverse)
                styled.reverse();
            if (m_strikethrough)
                styled.strikethrough();

            return styled.to_string();
        }

        std::vector<std::string> add_border(const std::vector<std::string> &lines, size_t width) const {
            auto chars = get_border_chars(m_border_style);
            std::vector<std::string> result;

            auto sb = [this](const std::string &s) -> std::string {
                if (!m_has_border_fg)
                    return s;
                echo::format::String styled(s);
                styled.fg(m_border_fg.r, m_border_fg.g, m_border_fg.b);
                return styled.to_string();
            };

            // Top
            result.push_back(sb(chars.top_left) + sb(repeat(chars.horizontal, width)) + sb(chars.top_right));

            // Content
            for (const auto &line : lines) {
                result.push_back(sb(chars.vertical) + line + sb(chars.vertical));
            }

            // Bottom
            result.push_back(sb(chars.bottom_left) + sb(repeat(chars.horizontal, width)) + sb(chars.bottom_right));

            return result;
        }
    };

    // ========== Layout Utilities ==========

    /// Join strings horizontally with vertical alignment
    inline std::string join_horizontal(Position align, const std::vector<std::string> &blocks) {
        if (blocks.empty())
            return "";

        std::vector<std::vector<std::string>> all_lines;
        std::vector<size_t> widths;
        size_t max_height = 0;

        for (const auto &block : blocks) {
            auto lines = split_lines(block);
            size_t w = 0;
            for (const auto &line : lines) {
                w = std::max(w, visible_width(line));
            }
            widths.push_back(w);
            all_lines.push_back(lines);
            max_height = std::max(max_height, lines.size());
        }

        // Pad to max height
        for (size_t b = 0; b < all_lines.size(); b++) {
            while (all_lines[b].size() < max_height) {
                switch (align) {
                case Position::Bottom:
                    all_lines[b].insert(all_lines[b].begin(), std::string(widths[b], ' '));
                    break;
                case Position::Center:
                    if (all_lines[b].size() % 2 == 0)
                        all_lines[b].insert(all_lines[b].begin(), std::string(widths[b], ' '));
                    else
                        all_lines[b].push_back(std::string(widths[b], ' '));
                    break;
                default:
                    all_lines[b].push_back(std::string(widths[b], ' '));
                    break;
                }
            }
            // Ensure all lines are same width
            for (auto &line : all_lines[b]) {
                line = pad_right(line, widths[b]);
            }
        }

        // Join
        std::string result;
        for (size_t row = 0; row < max_height; row++) {
            if (row > 0)
                result += "\n";
            for (size_t b = 0; b < all_lines.size(); b++) {
                result += all_lines[b][row];
            }
        }

        return result;
    }

    /// Join strings vertically with horizontal alignment
    inline std::string join_vertical(Position align, const std::vector<std::string> &blocks) {
        if (blocks.empty())
            return "";

        size_t max_width = 0;
        for (const auto &block : blocks) {
            max_width = std::max(max_width, max_line_width(block));
        }

        std::string result;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (i > 0)
                result += "\n";

            auto lines = split_lines(blocks[i]);
            for (size_t j = 0; j < lines.size(); j++) {
                if (j > 0)
                    result += "\n";
                switch (align) {
                case Position::Center:
                    result += pad_center(lines[j], max_width);
                    break;
                case Position::Right:
                    result += pad_left(lines[j], max_width);
                    break;
                default:
                    result += pad_right(lines[j], max_width);
                    break;
                }
            }
        }

        return result;
    }

    /// Place content at a position within bounds
    inline std::string place(int width, int height, Position h, Position v, const std::string &content) {
        auto lines = split_lines(content);

        // Horizontal
        for (auto &line : lines) {
            switch (h) {
            case Position::Center:
                line = pad_center(line, width);
                break;
            case Position::Right:
                line = pad_left(line, width);
                break;
            default:
                line = pad_right(line, width);
                break;
            }
        }

        // Vertical
        std::string empty_line(width, ' ');
        while (static_cast<int>(lines.size()) < height) {
            switch (v) {
            case Position::Bottom:
                lines.insert(lines.begin(), empty_line);
                break;
            case Position::Center:
                if (lines.size() % 2 == 0)
                    lines.insert(lines.begin(), empty_line);
                else
                    lines.push_back(empty_line);
                break;
            default:
                lines.push_back(empty_line);
                break;
            }
        }

        std::string result;
        for (size_t i = 0; i < lines.size(); i++) {
            if (i > 0)
                result += "\n";
            result += lines[i];
        }
        return result;
    }

} // namespace scan
