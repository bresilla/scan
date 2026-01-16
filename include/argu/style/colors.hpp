#pragma once

/// @file argu/style/colors.hpp
/// @brief Color and styling support for help output

#include <argu/core/types.hpp>

#include <cstdlib>
#include <string>

namespace argu {

    /// RGB Color representation
    struct Color {
        int r = 255;
        int g = 255;
        int b = 255;

        constexpr Color() = default;
        constexpr Color(int red, int green, int blue) : r(red), g(green), b(blue) {}

        /// Create from hex string (e.g., "#FF5500" or "FF5500")
        static Color from_hex(const std::string &hex) {
            std::string h = hex;
            if (!h.empty() && h[0] == '#')
                h = h.substr(1);
            if (h.size() != 6)
                return Color{255, 255, 255};

            try {
                int r = std::stoi(h.substr(0, 2), nullptr, 16);
                int g = std::stoi(h.substr(2, 2), nullptr, 16);
                int b = std::stoi(h.substr(4, 2), nullptr, 16);
                return Color{r, g, b};
            } catch (...) {
                return Color{255, 255, 255};
            }
        }

        /// Predefined colors
        static constexpr Color white() { return {255, 255, 255}; }
        static constexpr Color black() { return {0, 0, 0}; }
        static constexpr Color red() { return {255, 0, 0}; }
        static constexpr Color green() { return {0, 255, 0}; }
        static constexpr Color blue() { return {0, 0, 255}; }
        static constexpr Color yellow() { return {255, 255, 0}; }
        static constexpr Color cyan() { return {0, 255, 255}; }
        static constexpr Color magenta() { return {255, 0, 255}; }
        static constexpr Color orange() { return {255, 165, 0}; }
        static constexpr Color pink() { return {255, 192, 203}; }
        static constexpr Color purple() { return {128, 0, 128}; }
        static constexpr Color gray() { return {128, 128, 128}; }
        static constexpr Color light_gray() { return {192, 192, 192}; }
        static constexpr Color dark_gray() { return {64, 64, 64}; }

        // Terminal-friendly colors (ANSI 256)
        static constexpr Color term_red() { return {205, 49, 49}; }
        static constexpr Color term_green() { return {13, 188, 121}; }
        static constexpr Color term_yellow() { return {229, 229, 16}; }
        static constexpr Color term_blue() { return {36, 114, 200}; }
        static constexpr Color term_magenta() { return {188, 63, 188}; }
        static constexpr Color term_cyan() { return {17, 168, 205}; }
    };

    /// ANSI escape codes for terminal styling
    namespace ansi {
        inline const char *reset = "\033[0m";
        inline const char *bold = "\033[1m";
        inline const char *dim = "\033[2m";
        inline const char *italic = "\033[3m";
        inline const char *underline = "\033[4m";
        inline const char *blink = "\033[5m";
        inline const char *reverse = "\033[7m";
        inline const char *hidden = "\033[8m";
        inline const char *strikethrough = "\033[9m";

        /// Generate foreground color code
        inline std::string fg(const Color &c) {
            return "\033[38;2;" + std::to_string(c.r) + ";" + std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
        }

        /// Generate background color code
        inline std::string bg(const Color &c) {
            return "\033[48;2;" + std::to_string(c.r) + ";" + std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
        }

        /// Check if terminal supports colors
        inline bool supports_color() {
            // Check common environment variables
            const char *term = std::getenv("TERM");
            const char *colorterm = std::getenv("COLORTERM");
            const char *no_color = std::getenv("NO_COLOR");
            const char *force_color = std::getenv("FORCE_COLOR");
            const char *clicolor = std::getenv("CLICOLOR");
            const char *clicolor_force = std::getenv("CLICOLOR_FORCE");

            // NO_COLOR takes precedence
            if (no_color && no_color[0] != '\0')
                return false;

            // FORCE_COLOR and CLICOLOR_FORCE override
            if (force_color && force_color[0] != '\0')
                return true;
            if (clicolor_force && clicolor_force[0] != '\0' && std::string(clicolor_force) != "0")
                return true;

            // CLICOLOR=0 disables
            if (clicolor && std::string(clicolor) == "0")
                return false;

            // Check COLORTERM
            if (colorterm) {
                std::string ct = colorterm;
                if (ct == "truecolor" || ct == "24bit")
                    return true;
            }

            // Check TERM
            if (term) {
                std::string t = term;
                if (t.find("color") != std::string::npos)
                    return true;
                if (t.find("256") != std::string::npos)
                    return true;
                if (t.find("xterm") != std::string::npos)
                    return true;
                if (t.find("screen") != std::string::npos)
                    return true;
                if (t.find("tmux") != std::string::npos)
                    return true;
                if (t.find("vt100") != std::string::npos)
                    return true;
                if (t.find("linux") != std::string::npos)
                    return true;
                if (t == "dumb")
                    return false;
            }

            // Default to supporting color if TERM is set
            return term != nullptr;
        }

        /// Check if terminal supports truecolor (24-bit)
        inline bool supports_truecolor() {
            const char *colorterm = std::getenv("COLORTERM");
            if (colorterm) {
                std::string ct = colorterm;
                if (ct == "truecolor" || ct == "24bit")
                    return true;
            }
            return false;
        }
    } // namespace ansi

    /// Theme for help output styling
    struct HelpTheme {
        Color command_name = Color::term_cyan();
        Color option_name = Color::term_green();
        Color option_short = Color::term_green();
        Color positional = Color::term_yellow();
        Color subcommand = Color::term_blue();
        Color section_header = Color::term_yellow();
        Color default_value = Color::gray();
        Color env_var = Color::gray();
        Color error = Color::term_red();
        Color success = Color::term_green();
        Color warning = Color::term_yellow();
        Color description = Color::white();

        bool use_bold_headers = true;
        bool use_bold_options = false;
        bool use_dim_defaults = true;

        /// Predefined themes
        static HelpTheme default_theme() { return {}; }

        static HelpTheme minimal() {
            HelpTheme t;
            t.command_name = Color::white();
            t.option_name = Color::white();
            t.option_short = Color::white();
            t.positional = Color::white();
            t.subcommand = Color::white();
            t.section_header = Color::white();
            t.use_bold_headers = true;
            t.use_bold_options = false;
            return t;
        }

        static HelpTheme colorful() {
            HelpTheme t;
            t.command_name = Color::term_magenta();
            t.option_name = Color::term_cyan();
            t.option_short = Color::term_cyan();
            t.positional = Color::orange();
            t.subcommand = Color::term_green();
            t.section_header = Color::term_magenta();
            t.use_bold_headers = true;
            t.use_bold_options = true;
            return t;
        }

        static HelpTheme dark() {
            HelpTheme t;
            t.command_name = Color{100, 200, 255};
            t.option_name = Color{100, 255, 150};
            t.option_short = Color{100, 255, 150};
            t.positional = Color{255, 200, 100};
            t.subcommand = Color{150, 150, 255};
            t.section_header = Color{255, 200, 100};
            return t;
        }
    };

    /// Styled text builder
    class StyledText {
      public:
        StyledText() = default;
        explicit StyledText(const std::string &text) : m_text(text) {}

        StyledText &text(const std::string &t) {
            m_text = t;
            return *this;
        }
        StyledText &fg(const Color &c) {
            m_fg = c;
            m_has_fg = true;
            return *this;
        }
        StyledText &bg(const Color &c) {
            m_bg = c;
            m_has_bg = true;
            return *this;
        }
        StyledText &bold(bool b = true) {
            m_bold = b;
            return *this;
        }
        StyledText &dim(bool d = true) {
            m_dim = d;
            return *this;
        }
        StyledText &italic(bool i = true) {
            m_italic = i;
            return *this;
        }
        StyledText &underline(bool u = true) {
            m_underline = u;
            return *this;
        }

        std::string render(bool color_enabled = true) const {
            if (!color_enabled)
                return m_text;

            std::string result;

            if (m_bold)
                result += ansi::bold;
            if (m_dim)
                result += ansi::dim;
            if (m_italic)
                result += ansi::italic;
            if (m_underline)
                result += ansi::underline;
            if (m_has_fg)
                result += ansi::fg(m_fg);
            if (m_has_bg)
                result += ansi::bg(m_bg);

            result += m_text;

            if (m_has_fg || m_has_bg || m_bold || m_dim || m_italic || m_underline) {
                result += ansi::reset;
            }

            return result;
        }

      private:
        std::string m_text;
        Color m_fg;
        Color m_bg;
        bool m_has_fg = false;
        bool m_has_bg = false;
        bool m_bold = false;
        bool m_dim = false;
        bool m_italic = false;
        bool m_underline = false;
    };

    /// Helper function for quick styled text
    inline std::string styled(const std::string &text, const Color &fg, bool bold = false, bool color_enabled = true) {
        return StyledText(text).fg(fg).bold(bold).render(color_enabled);
    }

} // namespace argu
