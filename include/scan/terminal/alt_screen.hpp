#pragma once

/// @file alt_screen.hpp
/// @brief RAII wrapper for alternate screen buffer (like vim, less)

#include <cstdio>
#include <scan/terminal/terminal.hpp>

namespace scan::terminal {

    /// RAII wrapper for alternate screen buffer
    /// Enters alternate screen on construction, exits on destruction
    /// The alternate screen preserves the main screen content
    class AltScreen {
      public:
        /// Enter alternate screen buffer
        AltScreen() {
            if (!is_tty_out()) {
                m_enabled = false;
                return;
            }

            // Enter alternate screen buffer
            std::fputs("\x1b[?1049h", stdout);
            std::fflush(stdout);
            m_enabled = true;
        }

        /// Exit alternate screen buffer
        ~AltScreen() { leave(); }

        /// Explicitly leave alternate screen (called automatically by destructor)
        void leave() {
            if (!m_enabled)
                return;

            // Exit alternate screen buffer
            std::fputs("\x1b[?1049l", stdout);
            std::fflush(stdout);
            m_enabled = false;
        }

        /// Check if alternate screen is active
        bool enabled() const { return m_enabled; }

        /// Check if alternate screen is active
        operator bool() const { return m_enabled; }

        // Non-copyable
        AltScreen(const AltScreen &) = delete;
        AltScreen &operator=(const AltScreen &) = delete;

        // Movable
        AltScreen(AltScreen &&other) noexcept : m_enabled(other.m_enabled) { other.m_enabled = false; }

        AltScreen &operator=(AltScreen &&other) noexcept {
            if (this != &other) {
                leave();
                m_enabled = other.m_enabled;
                other.m_enabled = false;
            }
            return *this;
        }

      private:
        bool m_enabled = false;
    };

    /// RAII guard that enables mouse tracking while in scope
    class MouseTracking {
      public:
        /// Enable mouse tracking
        /// @param all_motion If true, reports all mouse motion (not just button presses)
        explicit MouseTracking(bool all_motion = false) : m_all_motion(all_motion) {
            if (!is_tty_out()) {
                m_enabled = false;
                return;
            }

            if (all_motion) {
                // Enable all mouse motion tracking
                std::fputs("\x1b[?1003h", stdout);
            } else {
                // Enable button event tracking
                std::fputs("\x1b[?1000h", stdout);
            }
            // Enable SGR extended mouse mode for larger coordinates
            std::fputs("\x1b[?1006h", stdout);
            std::fflush(stdout);
            m_enabled = true;
        }

        /// Disable mouse tracking
        ~MouseTracking() { disable(); }

        /// Explicitly disable mouse tracking
        void disable() {
            if (!m_enabled)
                return;

            if (m_all_motion) {
                std::fputs("\x1b[?1003l", stdout);
            } else {
                std::fputs("\x1b[?1000l", stdout);
            }
            std::fputs("\x1b[?1006l", stdout);
            std::fflush(stdout);
            m_enabled = false;
        }

        /// Check if mouse tracking is enabled
        bool enabled() const { return m_enabled; }

        // Non-copyable, non-movable
        MouseTracking(const MouseTracking &) = delete;
        MouseTracking &operator=(const MouseTracking &) = delete;
        MouseTracking(MouseTracking &&) = delete;
        MouseTracking &operator=(MouseTracking &&) = delete;

      private:
        bool m_enabled = false;
        bool m_all_motion = false;
    };

    /// RAII guard that enables bracketed paste mode while in scope
    class BracketedPaste {
      public:
        BracketedPaste() {
            if (!is_tty_out()) {
                m_enabled = false;
                return;
            }

            std::fputs("\x1b[?2004h", stdout);
            std::fflush(stdout);
            m_enabled = true;
        }

        ~BracketedPaste() { disable(); }

        void disable() {
            if (!m_enabled)
                return;

            std::fputs("\x1b[?2004l", stdout);
            std::fflush(stdout);
            m_enabled = false;
        }

        bool enabled() const { return m_enabled; }

        // Non-copyable, non-movable
        BracketedPaste(const BracketedPaste &) = delete;
        BracketedPaste &operator=(const BracketedPaste &) = delete;
        BracketedPaste(BracketedPaste &&) = delete;
        BracketedPaste &operator=(BracketedPaste &&) = delete;

      private:
        bool m_enabled = false;
    };

} // namespace scan::terminal
