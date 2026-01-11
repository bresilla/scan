#pragma once

/// @file msg.hpp
/// @brief Message types for the Tea runtime (Bubble Tea style)

#include <scan/input/key.hpp>
#include <string>
#include <variant>

namespace scan::tea {

    /// Key press message - user pressed a key
    struct KeyMsg {
        input::Key key = input::Key::None;
        char32_t rune = 0; // The actual character for Key::Rune
        bool alt = false;  // Alt modifier was held
    };

    /// Window size changed
    struct WindowSizeMsg {
        int width = 0;
        int height = 0;
    };

    /// Tick message for animations/timers
    struct TickMsg {
        int id = 0; // Timer ID
    };

    /// Focus gained
    struct FocusMsg {};

    /// Focus lost
    struct BlurMsg {};

    /// Request to quit the program
    struct QuitMsg {};

    /// Custom message for user-defined events
    struct CustomMsg {
        std::string type;
        std::string data;
    };

    /// Union of all possible message types
    using Msg = std::variant<KeyMsg, WindowSizeMsg, TickMsg, FocusMsg, BlurMsg, QuitMsg, CustomMsg>;

    /// Helper to check message type
    template <typename T> inline bool is(const Msg &msg) { return std::holds_alternative<T>(msg); }

    /// Helper to get message as specific type
    template <typename T> inline const T &as(const Msg &msg) { return std::get<T>(msg); }

    /// Helper to try get message as specific type
    template <typename T> inline const T *try_as(const Msg &msg) { return std::get_if<T>(&msg); }

} // namespace scan::tea
