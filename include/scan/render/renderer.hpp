#pragma once

/// @file renderer.hpp
/// @brief Rendering engine with diff-based updates

#include <scan/terminal/terminal.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace scan::render {

    /// Split a string into lines
    inline std::vector<std::string> split_lines(const std::string &s) {
        std::vector<std::string> lines;
        std::istringstream stream(s);
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }
        // Handle trailing newline
        if (!s.empty() && s.back() == '\n') {
            lines.push_back("");
        }
        return lines;
    }

    /// Renderer for TUI output
    /// Handles efficient updates by tracking previously rendered content
    class Renderer {
      public:
        /// Render new content, clearing previous output first
        void render(const std::string &content) {
            // Clear previous output
            if (m_lines_rendered > 0) {
                // Move to start of current line, then clear upward
                terminal::cursor_to_column(1);
                for (int i = 0; i < m_lines_rendered; i++) {
                    if (i > 0) {
                        terminal::cursor_up(1);
                    }
                    terminal::clear_line();
                }
                // Ensure cursor is at column 1 after clearing
                terminal::cursor_to_column(1);
            }

            // Split content into lines
            auto lines = split_lines(content);

            // Write new content
            for (size_t i = 0; i < lines.size(); i++) {
                if (i > 0) {
                    std::fputc('\n', stdout);
                }
                std::fputs("\r", stdout); // Carriage return to ensure column 1
                terminal::write(lines[i]);
            }
            std::fflush(stdout);

            // Track lines rendered
            m_lines_rendered = static_cast<int>(lines.size());
            if (m_lines_rendered > 0 && lines.back().empty()) {
                m_lines_rendered--; // Don't count trailing empty line
            }
            m_last_content = content;
        }

        /// Force a full repaint (clears screen)
        void repaint() {
            m_lines_rendered = 0;
            m_last_content.clear();
        }

        /// Clear all rendered content
        void clear() {
            if (m_lines_rendered > 0) {
                terminal::cursor_to_column(1);
                for (int i = 0; i < m_lines_rendered; i++) {
                    if (i > 0) {
                        terminal::cursor_up(1);
                    }
                    terminal::clear_line();
                }
            }
            m_lines_rendered = 0;
            m_last_content.clear();
            std::fflush(stdout);
        }

        /// Get number of lines currently rendered
        int lines_rendered() const { return m_lines_rendered; }

        /// Get last rendered content
        const std::string &last_content() const { return m_last_content; }

      private:
        int m_lines_rendered = 0;
        std::string m_last_content;
    };

    /// Simple inline view rendering (no state tracking)
    inline void render_inline(const std::string &content) {
        terminal::write(content);
        std::fflush(stdout);
    }

    /// Render with newline
    inline void render_line(const std::string &content) { terminal::writeln(content); }

} // namespace scan::render
