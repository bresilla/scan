#pragma once

/// @file theme.hpp
/// @brief Unified theme system for consistent styling across all components

#include <string>

namespace scan {

    /// RGB color struct
    struct Color {
        int r = 255, g = 255, b = 255;

        constexpr Color() = default;
        constexpr Color(int r_, int g_, int b_) : r(r_), g(g_), b(b_) {}

        bool operator==(const Color &other) const { return r == other.r && g == other.g && b == other.b; }
        bool operator!=(const Color &other) const { return !(*this == other); }
    };

    /// Adaptive colors - normal and dimmed variants
    struct AdaptiveColor {
        Color normal;
        Color dimmed;
    };

    /// Theme colors for consistent styling
    struct ThemeColors {
        // Primary accent color (used for cursor, selection highlight)
        Color primary{212, 175, 55}; // Gold

        // Secondary accent (used for matches, secondary highlights)
        Color secondary{99, 179, 237}; // Light blue

        // Success/positive
        Color success{72, 187, 120}; // Green

        // Error/danger
        Color error{245, 101, 101}; // Red

        // Warning
        Color warning{237, 187, 99}; // Orange

        // Text colors
        Color text{229, 229, 229};       // Light gray - main text
        Color text_muted{113, 113, 122}; // Zinc gray - secondary text
        Color text_subtle{82, 82, 91};   // Dark gray - hints, disabled
        Color text_inverted{24, 24, 27}; // Near black - for light backgrounds

        // Background colors
        Color bg{24, 24, 27};          // Near black
        Color bg_subtle{39, 39, 42};   // Zinc 800
        Color bg_muted{63, 63, 70};    // Zinc 700
        Color bg_emphasis{82, 82, 91}; // Zinc 600

        // Border colors
        Color border{63, 63, 70};         // Zinc 700
        Color border_muted{52, 52, 58};   // Darker
        Color border_focus{212, 175, 55}; // Primary for focus

        // Special purpose
        Color cursor{212, 175, 55};          // Same as primary
        Color selection{99, 179, 237};       // Same as secondary
        Color match_highlight{250, 204, 21}; // Yellow for search matches
    };

    /// Theme spacing values
    struct ThemeSpacing {
        int none = 0;
        int xs = 1;
        int sm = 2;
        int md = 3;
        int lg = 4;
        int xl = 6;
    };

    /// Complete theme definition
    struct Theme {
        ThemeColors colors;
        ThemeSpacing spacing;

        // Common strings
        std::string cursor_char = ">";
        std::string bullet = "*";
        std::string check = "[x]";
        std::string uncheck = "[ ]";
        std::string radio_on = "(*)";
        std::string radio_off = "( )";
        std::string arrow_up = "^";
        std::string arrow_down = "v";
        std::string arrow_left = "<";
        std::string arrow_right = ">";
        std::string ellipsis = "...";
    };

    /// Default dark theme
    inline Theme default_theme() { return Theme{}; }

    /// Light theme variant
    inline Theme light_theme() {
        Theme t;
        t.colors.primary = {147, 51, 234};  // Purple
        t.colors.secondary = {37, 99, 235}; // Blue
        t.colors.success = {34, 197, 94};   // Green
        t.colors.error = {239, 68, 68};     // Red
        t.colors.warning = {245, 158, 11};  // Amber

        t.colors.text = {24, 24, 27};           // Near black
        t.colors.text_muted = {113, 113, 122};  // Gray
        t.colors.text_subtle = {161, 161, 170}; // Light gray
        t.colors.text_inverted = {250, 250, 250};

        t.colors.bg = {250, 250, 250};        // Near white
        t.colors.bg_subtle = {244, 244, 245}; // Zinc 100
        t.colors.bg_muted = {228, 228, 231};  // Zinc 200
        t.colors.bg_emphasis = {212, 212, 216};

        t.colors.border = {212, 212, 216};
        t.colors.border_muted = {228, 228, 231};
        t.colors.border_focus = {147, 51, 234};

        t.colors.cursor = {147, 51, 234};
        t.colors.selection = {37, 99, 235};
        t.colors.match_highlight = {234, 179, 8};

        return t;
    }

    /// Global theme instance (can be changed at runtime)
    inline Theme &current_theme() {
        static Theme theme = default_theme();
        return theme;
    }

    /// Set the current theme
    inline void set_theme(const Theme &t) { current_theme() = t; }

} // namespace scan
