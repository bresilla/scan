#pragma once

/// @file cmd.hpp
/// @brief Commands for the Tea runtime (async operations)

#include <chrono>
#include <functional>
#include <optional>
#include <scan/tea/msg.hpp>
#include <thread>
#include <vector>

namespace scan::tea {

    /// Command - a function that may produce a message
    /// Returns std::nullopt if no message to send
    using Cmd = std::function<std::optional<Msg>()>;

    /// No-op command - does nothing
    inline Cmd none() { return nullptr; }

    /// Quit command - signals the program to exit
    inline Cmd quit() {
        return []() -> std::optional<Msg> { return QuitMsg{}; };
    }

    /// Batch multiple commands together
    /// All commands will be executed, messages collected
    inline Cmd batch(std::vector<Cmd> cmds) {
        if (cmds.empty())
            return none();

        return [cmds = std::move(cmds)]() -> std::optional<Msg> {
            for (const auto &cmd : cmds) {
                if (cmd) {
                    auto msg = cmd();
                    if (msg)
                        return msg;
                }
            }
            return std::nullopt;
        };
    }

    /// Create a tick command that fires after a delay
    inline Cmd tick(std::chrono::milliseconds delay, int id = 0) {
        return [delay, id]() -> std::optional<Msg> {
            std::this_thread::sleep_for(delay);
            return TickMsg{id};
        };
    }

    /// Create a command that sends a custom message
    inline Cmd send(Msg msg) {
        return [msg = std::move(msg)]() -> std::optional<Msg> { return msg; };
    }

    /// Sequence commands - run one after another
    inline Cmd sequence(std::vector<Cmd> cmds) { return batch(std::move(cmds)); }

} // namespace scan::tea
