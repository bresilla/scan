#pragma once

/// @file raw_mode.hpp
/// @brief RAII wrapper for terminal raw mode

#include <scan/terminal/terminal.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace scan::terminal {

    /// RAII wrapper for terminal raw mode
    /// Enables character-by-character input without echo
    /// Automatically restores original terminal settings on destruction
    class RawMode {
      public:
        /// Enter raw mode
        RawMode() {
            if (!is_tty()) {
                m_enabled = false;
                return;
            }

#ifdef _WIN32
            m_handle = GetStdHandle(STD_INPUT_HANDLE);
            if (m_handle == INVALID_HANDLE_VALUE) {
                m_enabled = false;
                return;
            }

            if (!GetConsoleMode(m_handle, &m_original_mode)) {
                m_enabled = false;
                return;
            }

            DWORD raw_mode = m_original_mode;
            raw_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
            raw_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

            if (!SetConsoleMode(m_handle, raw_mode)) {
                m_enabled = false;
                return;
            }

            // Also enable VT processing on output
            HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD out_mode;
            if (GetConsoleMode(out_handle, &out_mode)) {
                out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(out_handle, out_mode);
            }

            m_enabled = true;
#else
            if (tcgetattr(STDIN_FILENO, &m_original_termios) < 0) {
                m_enabled = false;
                return;
            }

            struct termios raw = m_original_termios;

            // Input flags: no break, no CR to NL, no parity check, no strip char,
            // no start/stop output control
            raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

            // Output flags: disable post processing
            raw.c_oflag &= ~(OPOST);

            // Control flags: set 8 bit chars
            raw.c_cflag |= (CS8);

            // Local flags: no echo, no canonical mode, no extended functions,
            // no signal chars (^Z, ^C)
            raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

            // Control chars: set return condition to min 1 byte and timer to 0
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;

            if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
                m_enabled = false;
                return;
            }

            m_enabled = true;
#endif
        }

        /// Restore original terminal settings
        ~RawMode() { restore(); }

        /// Explicitly restore terminal (called automatically by destructor)
        void restore() {
            if (!m_enabled)
                return;

#ifdef _WIN32
            SetConsoleMode(m_handle, m_original_mode);
#else
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_original_termios);
#endif

            m_enabled = false;
        }

        /// Check if raw mode was successfully enabled
        bool enabled() const { return m_enabled; }

        /// Check if raw mode is currently active
        operator bool() const { return m_enabled; }

        // Non-copyable
        RawMode(const RawMode &) = delete;
        RawMode &operator=(const RawMode &) = delete;

        // Movable
        RawMode(RawMode &&other) noexcept
            : m_enabled(other.m_enabled)
#ifdef _WIN32
              ,
              m_handle(other.m_handle), m_original_mode(other.m_original_mode)
#else
              ,
              m_original_termios(other.m_original_termios)
#endif
        {
            other.m_enabled = false;
        }

        RawMode &operator=(RawMode &&other) noexcept {
            if (this != &other) {
                restore();
                m_enabled = other.m_enabled;
#ifdef _WIN32
                m_handle = other.m_handle;
                m_original_mode = other.m_original_mode;
#else
                m_original_termios = other.m_original_termios;
#endif
                other.m_enabled = false;
            }
            return *this;
        }

      private:
        bool m_enabled = false;

#ifdef _WIN32
        HANDLE m_handle = INVALID_HANDLE_VALUE;
        DWORD m_original_mode = 0;
#else
        struct termios m_original_termios{};
#endif
    };

    /// RAII guard that hides the cursor while in scope
    class HiddenCursor {
      public:
        HiddenCursor() { hide_cursor(); }
        ~HiddenCursor() { show_cursor(); }

        // Non-copyable, non-movable
        HiddenCursor(const HiddenCursor &) = delete;
        HiddenCursor &operator=(const HiddenCursor &) = delete;
        HiddenCursor(HiddenCursor &&) = delete;
        HiddenCursor &operator=(HiddenCursor &&) = delete;
    };

} // namespace scan::terminal
