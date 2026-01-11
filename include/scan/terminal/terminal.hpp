#pragma once

/// @file terminal.hpp
/// @brief Terminal utilities for cursor control, screen manipulation, etc.

#include <cstdio>
#include <string>
#include <utility>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
inline int isatty(int fd) { return _isatty(fd); }
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace scan::terminal {

    /// Terminal size
    struct Size {
        int cols = 80;
        int rows = 24;
    };

    /// Cursor position
    struct Cursor {
        int row = 0;
        int col = 0;
    };

    /// Check if stdin is a TTY
    inline bool is_tty() { return isatty(STDIN_FILENO) != 0; }

    /// Check if stdout is a TTY
    inline bool is_tty_out() { return isatty(STDOUT_FILENO) != 0; }

    /// Get terminal size
    inline Size get_size() {
        Size size;

#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            size.cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            size.rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
#else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            size.cols = w.ws_col;
            size.rows = w.ws_row;
        }
#endif

        return size;
    }

    // =============================================================================
    // ANSI Escape Sequences
    // =============================================================================

    /// CSI (Control Sequence Introducer)
    inline constexpr const char *CSI = "\x1b[";

    /// Clear entire screen
    inline void clear_screen() {
        std::fputs("\x1b[2J\x1b[H", stdout);
        std::fflush(stdout);
    }

    /// Clear from cursor to end of screen
    inline void clear_to_end() {
        std::fputs("\x1b[J", stdout);
        std::fflush(stdout);
    }

    /// Clear current line
    inline void clear_line() {
        std::fputs("\x1b[2K", stdout);
        std::fflush(stdout);
    }

    /// Clear from cursor to end of line
    inline void clear_line_to_end() {
        std::fputs("\x1b[K", stdout);
        std::fflush(stdout);
    }

    /// Clear from cursor to beginning of line
    inline void clear_line_to_start() {
        std::fputs("\x1b[1K", stdout);
        std::fflush(stdout);
    }

    /// Move cursor to position (1-indexed)
    inline void move_cursor(int row, int col) {
        std::fprintf(stdout, "\x1b[%d;%dH", row, col);
        std::fflush(stdout);
    }

    /// Move cursor to home position (1,1)
    inline void cursor_home() {
        std::fputs("\x1b[H", stdout);
        std::fflush(stdout);
    }

    /// Move cursor up n lines
    inline void cursor_up(int n = 1) {
        std::fprintf(stdout, "\x1b[%dA", n);
        std::fflush(stdout);
    }

    /// Move cursor down n lines
    inline void cursor_down(int n = 1) {
        std::fprintf(stdout, "\x1b[%dB", n);
        std::fflush(stdout);
    }

    /// Move cursor right n columns
    inline void cursor_right(int n = 1) {
        std::fprintf(stdout, "\x1b[%dC", n);
        std::fflush(stdout);
    }

    /// Move cursor left n columns
    inline void cursor_left(int n = 1) {
        std::fprintf(stdout, "\x1b[%dD", n);
        std::fflush(stdout);
    }

    /// Move cursor to beginning of line
    inline void cursor_to_column(int col = 1) {
        std::fprintf(stdout, "\x1b[%dG", col);
        std::fflush(stdout);
    }

    /// Hide cursor
    inline void hide_cursor() {
        std::fputs("\x1b[?25l", stdout);
        std::fflush(stdout);
    }

    /// Show cursor
    inline void show_cursor() {
        std::fputs("\x1b[?25h", stdout);
        std::fflush(stdout);
    }

    /// Save cursor position
    inline void save_cursor() {
        std::fputs("\x1b[s", stdout);
        std::fflush(stdout);
    }

    /// Restore cursor position
    inline void restore_cursor() {
        std::fputs("\x1b[u", stdout);
        std::fflush(stdout);
    }

    /// Save cursor position (alternative, DEC)
    inline void save_cursor_dec() {
        std::fputs("\x1b"
                   "7",
                   stdout);
        std::fflush(stdout);
    }

    /// Restore cursor position (alternative, DEC)
    inline void restore_cursor_dec() {
        std::fputs("\x1b"
                   "8",
                   stdout);
        std::fflush(stdout);
    }

    /// Enable line wrapping
    inline void enable_line_wrap() {
        std::fputs("\x1b[?7h", stdout);
        std::fflush(stdout);
    }

    /// Disable line wrapping
    inline void disable_line_wrap() {
        std::fputs("\x1b[?7l", stdout);
        std::fflush(stdout);
    }

    /// Scroll up n lines
    inline void scroll_up(int n = 1) {
        std::fprintf(stdout, "\x1b[%dS", n);
        std::fflush(stdout);
    }

    /// Scroll down n lines
    inline void scroll_down(int n = 1) {
        std::fprintf(stdout, "\x1b[%dT", n);
        std::fflush(stdout);
    }

    /// Insert n blank lines at cursor
    inline void insert_lines(int n = 1) {
        std::fprintf(stdout, "\x1b[%dL", n);
        std::fflush(stdout);
    }

    /// Delete n lines at cursor
    inline void delete_lines(int n = 1) {
        std::fprintf(stdout, "\x1b[%dM", n);
        std::fflush(stdout);
    }

    /// Ring the terminal bell
    inline void bell() {
        std::fputs("\a", stdout);
        std::fflush(stdout);
    }

    /// Write raw output to stdout
    inline void write(const std::string &s) {
        std::fwrite(s.data(), 1, s.size(), stdout);
        std::fflush(stdout);
    }

    /// Write raw output followed by newline
    inline void writeln(const std::string &s = "") {
        std::fwrite(s.data(), 1, s.size(), stdout);
        std::fputc('\n', stdout);
        std::fflush(stdout);
    }

} // namespace scan::terminal
