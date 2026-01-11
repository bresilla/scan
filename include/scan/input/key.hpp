#pragma once

/// @file key.hpp
/// @brief Key definitions for input handling

#include <cstdint>
#include <string>

namespace scan::input {

    /// Key codes for special keys
    enum class Key {
        // No key / unknown
        None,
        Unknown,

        // Printable character (check rune field)
        Rune,

        // Control keys
        Enter,
        Tab,
        Backspace,
        Delete,
        Escape,
        Space,

        // Navigation
        Up,
        Down,
        Left,
        Right,
        Home,
        End,
        PageUp,
        PageDown,
        Insert,

        // Ctrl combinations (Ctrl+A = 0x01, etc.)
        CtrlA,
        CtrlB,
        CtrlC,
        CtrlD,
        CtrlE,
        CtrlF,
        CtrlG,
        CtrlH, // Same as Backspace
        CtrlI, // Same as Tab
        CtrlJ, // Same as Enter (LF)
        CtrlK,
        CtrlL,
        CtrlM, // Same as Enter (CR)
        CtrlN,
        CtrlO,
        CtrlP,
        CtrlQ,
        CtrlR,
        CtrlS,
        CtrlT,
        CtrlU,
        CtrlV,
        CtrlW,
        CtrlX,
        CtrlY,
        CtrlZ,

        // Alt combinations
        AltA,
        AltB,
        AltC,
        AltD,
        AltE,
        AltF,
        AltG,
        AltH,
        AltI,
        AltJ,
        AltK,
        AltL,
        AltM,
        AltN,
        AltO,
        AltP,
        AltQ,
        AltR,
        AltS,
        AltT,
        AltU,
        AltV,
        AltW,
        AltX,
        AltY,
        AltZ,
        AltUp,
        AltDown,
        AltLeft,
        AltRight,
        AltBackspace,
        AltEnter,

        // Shift combinations
        ShiftTab,
        ShiftUp,
        ShiftDown,
        ShiftLeft,
        ShiftRight,

        // Ctrl+Shift combinations
        CtrlShiftUp,
        CtrlShiftDown,
        CtrlShiftLeft,
        CtrlShiftRight,

        // Function keys
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
    };

    /// Get a string representation of a key
    inline std::string key_name(Key key) {
        switch (key) {
        case Key::None:
            return "None";
        case Key::Unknown:
            return "Unknown";
        case Key::Rune:
            return "Rune";
        case Key::Enter:
            return "Enter";
        case Key::Tab:
            return "Tab";
        case Key::Backspace:
            return "Backspace";
        case Key::Delete:
            return "Delete";
        case Key::Escape:
            return "Escape";
        case Key::Space:
            return "Space";
        case Key::Up:
            return "Up";
        case Key::Down:
            return "Down";
        case Key::Left:
            return "Left";
        case Key::Right:
            return "Right";
        case Key::Home:
            return "Home";
        case Key::End:
            return "End";
        case Key::PageUp:
            return "PageUp";
        case Key::PageDown:
            return "PageDown";
        case Key::Insert:
            return "Insert";
        case Key::CtrlA:
            return "Ctrl+A";
        case Key::CtrlB:
            return "Ctrl+B";
        case Key::CtrlC:
            return "Ctrl+C";
        case Key::CtrlD:
            return "Ctrl+D";
        case Key::CtrlE:
            return "Ctrl+E";
        case Key::CtrlF:
            return "Ctrl+F";
        case Key::CtrlG:
            return "Ctrl+G";
        case Key::CtrlH:
            return "Ctrl+H";
        case Key::CtrlI:
            return "Ctrl+I";
        case Key::CtrlJ:
            return "Ctrl+J";
        case Key::CtrlK:
            return "Ctrl+K";
        case Key::CtrlL:
            return "Ctrl+L";
        case Key::CtrlM:
            return "Ctrl+M";
        case Key::CtrlN:
            return "Ctrl+N";
        case Key::CtrlO:
            return "Ctrl+O";
        case Key::CtrlP:
            return "Ctrl+P";
        case Key::CtrlQ:
            return "Ctrl+Q";
        case Key::CtrlR:
            return "Ctrl+R";
        case Key::CtrlS:
            return "Ctrl+S";
        case Key::CtrlT:
            return "Ctrl+T";
        case Key::CtrlU:
            return "Ctrl+U";
        case Key::CtrlV:
            return "Ctrl+V";
        case Key::CtrlW:
            return "Ctrl+W";
        case Key::CtrlX:
            return "Ctrl+X";
        case Key::CtrlY:
            return "Ctrl+Y";
        case Key::CtrlZ:
            return "Ctrl+Z";
        case Key::AltUp:
            return "Alt+Up";
        case Key::AltDown:
            return "Alt+Down";
        case Key::AltLeft:
            return "Alt+Left";
        case Key::AltRight:
            return "Alt+Right";
        case Key::AltBackspace:
            return "Alt+Backspace";
        case Key::AltEnter:
            return "Alt+Enter";
        case Key::ShiftTab:
            return "Shift+Tab";
        case Key::F1:
            return "F1";
        case Key::F2:
            return "F2";
        case Key::F3:
            return "F3";
        case Key::F4:
            return "F4";
        case Key::F5:
            return "F5";
        case Key::F6:
            return "F6";
        case Key::F7:
            return "F7";
        case Key::F8:
            return "F8";
        case Key::F9:
            return "F9";
        case Key::F10:
            return "F10";
        case Key::F11:
            return "F11";
        case Key::F12:
            return "F12";
        default:
            return "Unknown";
        }
    }

    /// Check if a key is a printable character
    inline bool is_printable(Key key) { return key == Key::Rune || key == Key::Space; }

    /// Check if a key is a navigation key
    inline bool is_navigation(Key key) {
        switch (key) {
        case Key::Up:
        case Key::Down:
        case Key::Left:
        case Key::Right:
        case Key::Home:
        case Key::End:
        case Key::PageUp:
        case Key::PageDown:
            return true;
        default:
            return false;
        }
    }

} // namespace scan::input
