#pragma once

/// @file reader.hpp
/// @brief Input reading and ANSI escape sequence parsing

#include <scan/input/key.hpp>
#include <scan/terminal/terminal.hpp>

#include <chrono>
#include <optional>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <poll.h>
#include <sys/select.h>
#include <unistd.h>
#endif

namespace scan::input {

    /// Result of reading a key
    struct KeyEvent {
        Key key = Key::None;
        char32_t rune = 0;  // The character for Key::Rune
        bool alt = false;   // Alt modifier
        bool ctrl = false;  // Ctrl modifier (implicit in CtrlX keys)
        bool shift = false; // Shift modifier
    };

    /// Check if input is available (non-blocking)
    inline bool has_input() {
#ifdef _WIN32
        return _kbhit() != 0;
#else
        struct pollfd pfd;
        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;
        return poll(&pfd, 1, 0) > 0;
#endif
    }

    /// Read a single byte with optional timeout
    /// @param timeout_ms Timeout in milliseconds (-1 for blocking)
    /// @return The byte read, or -1 on timeout/error
    inline int read_byte(int timeout_ms = -1) {
#ifdef _WIN32
        if (timeout_ms >= 0) {
            auto start = std::chrono::steady_clock::now();
            while (!_kbhit()) {
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start)
                        .count();
                if (elapsed >= timeout_ms)
                    return -1;
                Sleep(1);
            }
        }
        return _getch();
#else
        if (timeout_ms >= 0) {
            struct pollfd pfd;
            pfd.fd = STDIN_FILENO;
            pfd.events = POLLIN;
            int ret = poll(&pfd, 1, timeout_ms);
            if (ret <= 0)
                return -1;
        }

        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        return (n == 1) ? c : -1;
#endif
    }

    /// Read multiple bytes with short timeout (for escape sequences)
    inline std::string read_escape_sequence() {
        std::string seq;

        // Read with short timeout to catch escape sequences
        while (true) {
            int c = read_byte(50); // 50ms timeout
            if (c < 0)
                break;
            seq += static_cast<char>(c);

            // Most escape sequences end with a letter
            if (seq.length() > 1 && std::isalpha(c))
                break;
            // Or with ~
            if (c == '~')
                break;
            // Limit sequence length
            if (seq.length() > 16)
                break;
        }

        return seq;
    }

    /// Parse an escape sequence into a key event
    inline KeyEvent parse_escape_sequence(const std::string &seq) {
        KeyEvent event;

        if (seq.empty()) {
            event.key = Key::Escape;
            return event;
        }

        // CSI sequences: ESC [
        if (seq[0] == '[') {
            std::string rest = seq.substr(1);

            // Arrow keys
            if (rest == "A") {
                event.key = Key::Up;
                return event;
            }
            if (rest == "B") {
                event.key = Key::Down;
                return event;
            }
            if (rest == "C") {
                event.key = Key::Right;
                return event;
            }
            if (rest == "D") {
                event.key = Key::Left;
                return event;
            }
            if (rest == "H") {
                event.key = Key::Home;
                return event;
            }
            if (rest == "F") {
                event.key = Key::End;
                return event;
            }

            // With modifiers: ESC [ 1 ; modifier letter
            if (rest.length() >= 3 && rest[0] == '1' && rest[1] == ';') {
                char mod = rest[2];
                char key = rest.length() > 3 ? rest[3] : 0;

                // Modifier 3 = Alt, 5 = Ctrl, 2 = Shift
                if (mod == '3') { // Alt
                    event.alt = true;
                    if (key == 'A') {
                        event.key = Key::AltUp;
                        return event;
                    }
                    if (key == 'B') {
                        event.key = Key::AltDown;
                        return event;
                    }
                    if (key == 'C') {
                        event.key = Key::AltRight;
                        return event;
                    }
                    if (key == 'D') {
                        event.key = Key::AltLeft;
                        return event;
                    }
                }
                if (mod == '2') { // Shift
                    event.shift = true;
                    if (key == 'A') {
                        event.key = Key::ShiftUp;
                        return event;
                    }
                    if (key == 'B') {
                        event.key = Key::ShiftDown;
                        return event;
                    }
                    if (key == 'C') {
                        event.key = Key::ShiftRight;
                        return event;
                    }
                    if (key == 'D') {
                        event.key = Key::ShiftLeft;
                        return event;
                    }
                }
                if (mod == '6') { // Ctrl+Shift
                    event.ctrl = true;
                    event.shift = true;
                    if (key == 'A') {
                        event.key = Key::CtrlShiftUp;
                        return event;
                    }
                    if (key == 'B') {
                        event.key = Key::CtrlShiftDown;
                        return event;
                    }
                    if (key == 'C') {
                        event.key = Key::CtrlShiftRight;
                        return event;
                    }
                    if (key == 'D') {
                        event.key = Key::CtrlShiftLeft;
                        return event;
                    }
                }
            }

            // Function keys and special keys with ~
            if (rest.length() >= 2 && rest.back() == '~') {
                std::string num = rest.substr(0, rest.length() - 1);
                if (num == "1" || num == "7") {
                    event.key = Key::Home;
                    return event;
                }
                if (num == "2") {
                    event.key = Key::Insert;
                    return event;
                }
                if (num == "3") {
                    event.key = Key::Delete;
                    return event;
                }
                if (num == "4" || num == "8") {
                    event.key = Key::End;
                    return event;
                }
                if (num == "5") {
                    event.key = Key::PageUp;
                    return event;
                }
                if (num == "6") {
                    event.key = Key::PageDown;
                    return event;
                }
                if (num == "11") {
                    event.key = Key::F1;
                    return event;
                }
                if (num == "12") {
                    event.key = Key::F2;
                    return event;
                }
                if (num == "13") {
                    event.key = Key::F3;
                    return event;
                }
                if (num == "14") {
                    event.key = Key::F4;
                    return event;
                }
                if (num == "15") {
                    event.key = Key::F5;
                    return event;
                }
                if (num == "17") {
                    event.key = Key::F6;
                    return event;
                }
                if (num == "18") {
                    event.key = Key::F7;
                    return event;
                }
                if (num == "19") {
                    event.key = Key::F8;
                    return event;
                }
                if (num == "20") {
                    event.key = Key::F9;
                    return event;
                }
                if (num == "21") {
                    event.key = Key::F10;
                    return event;
                }
                if (num == "23") {
                    event.key = Key::F11;
                    return event;
                }
                if (num == "24") {
                    event.key = Key::F12;
                    return event;
                }
            }

            // Shift+Tab
            if (rest == "Z") {
                event.key = Key::ShiftTab;
                return event;
            }
        }

        // SS3 sequences: ESC O
        if (seq[0] == 'O') {
            if (seq.length() > 1) {
                char c = seq[1];
                if (c == 'A') {
                    event.key = Key::Up;
                    return event;
                }
                if (c == 'B') {
                    event.key = Key::Down;
                    return event;
                }
                if (c == 'C') {
                    event.key = Key::Right;
                    return event;
                }
                if (c == 'D') {
                    event.key = Key::Left;
                    return event;
                }
                if (c == 'H') {
                    event.key = Key::Home;
                    return event;
                }
                if (c == 'F') {
                    event.key = Key::End;
                    return event;
                }
                if (c == 'P') {
                    event.key = Key::F1;
                    return event;
                }
                if (c == 'Q') {
                    event.key = Key::F2;
                    return event;
                }
                if (c == 'R') {
                    event.key = Key::F3;
                    return event;
                }
                if (c == 'S') {
                    event.key = Key::F4;
                    return event;
                }
            }
        }

        // Alt + letter: ESC followed by letter
        if (seq.length() == 1) {
            char c = seq[0];
            if (c >= 'a' && c <= 'z') {
                event.alt = true;
                event.key = static_cast<Key>(static_cast<int>(Key::AltA) + (c - 'a'));
                return event;
            }
            if (c >= 'A' && c <= 'Z') {
                event.alt = true;
                event.shift = true;
                event.key = static_cast<Key>(static_cast<int>(Key::AltA) + (c - 'A'));
                return event;
            }
            if (c == 0x7f) { // Alt+Backspace
                event.key = Key::AltBackspace;
                return event;
            }
            if (c == '\r' || c == '\n') {
                event.key = Key::AltEnter;
                return event;
            }
        }

        event.key = Key::Unknown;
        return event;
    }

    /// Parse a single byte into a key event
    inline KeyEvent parse_byte(unsigned char c) {
        KeyEvent event;

        // Control characters
        if (c == 0) {
            event.key = Key::None;
        } else if (c == 9) {
            event.key = Key::Tab;
        } else if (c == 10 || c == 13) {
            event.key = Key::Enter;
        } else if (c == 27) {
            event.key = Key::Escape;
        } else if (c == 32) {
            event.key = Key::Space;
            event.rune = ' ';
        } else if (c == 127) {
            event.key = Key::Backspace;
        } else if (c >= 1 && c <= 26) {
            // Ctrl+A through Ctrl+Z
            event.ctrl = true;
            event.key = static_cast<Key>(static_cast<int>(Key::CtrlA) + (c - 1));
        } else if (c >= 32 && c < 127) {
            // Printable ASCII
            event.key = Key::Rune;
            event.rune = c;
        } else if (c >= 128) {
            // Start of UTF-8 sequence
            event.key = Key::Rune;
            event.rune = c; // Will need more bytes for full codepoint
        } else {
            event.key = Key::Unknown;
        }

        return event;
    }

    /// Read a complete key event (handles escape sequences)
    /// @param timeout_ms Timeout in milliseconds (-1 for blocking)
    /// @return Key event, or nullopt on timeout
    inline std::optional<KeyEvent> read_key(int timeout_ms = -1) {
        int c = read_byte(timeout_ms);
        if (c < 0)
            return std::nullopt;

        // Handle escape sequences
        if (c == 27) { // ESC
            // Check if more bytes are available (escape sequence)
            if (has_input()) {
                std::string seq = read_escape_sequence();
                return parse_escape_sequence(seq);
            }
            // Just ESC key
            KeyEvent event;
            event.key = Key::Escape;
            return event;
        }

        // Handle UTF-8 multi-byte sequences
        if (c >= 0xC0) {
            KeyEvent event;
            event.key = Key::Rune;

            // Decode UTF-8
            char32_t codepoint = 0;
            int remaining = 0;

            if ((c & 0xE0) == 0xC0) {
                codepoint = c & 0x1F;
                remaining = 1;
            } else if ((c & 0xF0) == 0xE0) {
                codepoint = c & 0x0F;
                remaining = 2;
            } else if ((c & 0xF8) == 0xF0) {
                codepoint = c & 0x07;
                remaining = 3;
            }

            for (int i = 0; i < remaining; i++) {
                int next = read_byte(10);
                if (next < 0 || (next & 0xC0) != 0x80) {
                    event.key = Key::Unknown;
                    return event;
                }
                codepoint = (codepoint << 6) | (next & 0x3F);
            }

            event.rune = codepoint;
            return event;
        }

        return parse_byte(static_cast<unsigned char>(c));
    }

    /// Convert a key event to a string representation (for debugging)
    inline std::string key_event_to_string(const KeyEvent &event) {
        std::string result;

        if (event.ctrl)
            result += "Ctrl+";
        if (event.alt)
            result += "Alt+";
        if (event.shift)
            result += "Shift+";

        if (event.key == Key::Rune) {
            // Convert rune to UTF-8
            char32_t cp = event.rune;
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
        } else {
            result += key_name(event.key);
        }

        return result;
    }

} // namespace scan::input
