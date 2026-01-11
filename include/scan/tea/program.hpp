#pragma once

/// @file program.hpp
/// @brief The Tea runtime - runs the Model-View-Update loop

#include <scan/input/reader.hpp>
#include <scan/render/renderer.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/terminal/alt_screen.hpp>
#include <scan/terminal/raw_mode.hpp>
#include <scan/terminal/terminal.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

namespace scan::tea {

    /// Program options
    struct ProgramOptions {
        bool alt_screen = false;    // Use alternate screen buffer
        bool mouse = false;         // Enable mouse tracking
        bool hide_cursor = true;    // Hide cursor during execution
        int input_timeout_ms = 100; // Timeout for input polling
    };

    /// The Tea Program - runs the MVU loop
    /// @tparam Model The model type for this program
    template <typename Model> class Program {
      public:
        /// Function types
        using InitFn = std::function<std::pair<Model, Cmd>()>;
        using UpdateFn = std::function<std::pair<Model, Cmd>(Model, Msg)>;
        using ViewFn = std::function<std::string(const Model &)>;

        /// Construct a program with init, update, and view functions
        Program(InitFn init, UpdateFn update, ViewFn view)
            : m_init(std::move(init)), m_update(std::move(update)), m_view(std::move(view)) {}

        /// Set program options
        Program &with_options(const ProgramOptions &opts) {
            m_options = opts;
            return *this;
        }

        /// Enable/disable alternate screen
        Program &with_alt_screen(bool enable) {
            m_options.alt_screen = enable;
            return *this;
        }

        /// Enable/disable mouse tracking
        Program &with_mouse(bool enable) {
            m_options.mouse = enable;
            return *this;
        }

        /// Enable/disable cursor hiding
        Program &with_hidden_cursor(bool hide) {
            m_options.hide_cursor = hide;
            return *this;
        }

        /// Run the program, returns final model
        Model run() {
            // Initialize terminal
            terminal::RawMode raw_mode;
            std::optional<terminal::AltScreen> alt_screen;
            std::optional<terminal::HiddenCursor> hidden_cursor;
            std::optional<terminal::MouseTracking> mouse_tracking;

            if (m_options.alt_screen) {
                alt_screen.emplace();
                terminal::clear_screen();
            }

            if (m_options.hide_cursor) {
                hidden_cursor.emplace();
            }

            if (m_options.mouse) {
                mouse_tracking.emplace();
            }

            // Initialize model
            auto [model, init_cmd] = m_init();

            // Process initial command
            if (init_cmd) {
                auto msg = init_cmd();
                if (msg) {
                    auto [new_model, cmd] = m_update(std::move(model), *msg);
                    model = std::move(new_model);
                    // Queue the command for processing
                    if (cmd) {
                        auto cmd_msg = cmd();
                        if (cmd_msg && is<QuitMsg>(*cmd_msg)) {
                            return model;
                        }
                    }
                }
            }

            // Initial render
            render::Renderer renderer;
            renderer.render(m_view(model));

            // Event loop
            m_running = true;
            while (m_running) {
                // Read input
                auto key_event = input::read_key(m_options.input_timeout_ms);
                if (key_event) {
                    // Convert to message
                    KeyMsg key_msg;
                    key_msg.key = key_event->key;
                    key_msg.rune = key_event->rune;
                    key_msg.alt = key_event->alt;

                    // Check for Ctrl+C
                    if (key_event->key == input::Key::CtrlC) {
                        m_running = false;
                        break;
                    }

                    // Update model
                    auto [new_model, cmd] = m_update(std::move(model), key_msg);
                    model = std::move(new_model);

                    // Process command
                    if (cmd) {
                        auto msg = cmd();
                        if (msg) {
                            if (is<QuitMsg>(*msg)) {
                                m_running = false;
                                break;
                            }
                            // Process the message
                            auto [updated_model, new_cmd] = m_update(std::move(model), *msg);
                            model = std::move(updated_model);
                        }
                    }

                    // Render
                    renderer.render(m_view(model));
                }
            }

            // Final cleanup - clear rendered content
            renderer.clear();

            return model;
        }

        /// Request the program to quit
        void quit() { m_running = false; }

      private:
        InitFn m_init;
        UpdateFn m_update;
        ViewFn m_view;
        ProgramOptions m_options;
        std::atomic<bool> m_running{false};
    };

    /// Convenience function to create and run a simple program
    template <typename Model>
    Model run(std::function<std::pair<Model, Cmd>()> init, std::function<std::pair<Model, Cmd>(Model, Msg)> update,
              std::function<std::string(const Model &)> view, const ProgramOptions &options = {}) {
        return Program<Model>(std::move(init), std::move(update), std::move(view)).with_options(options).run();
    }

} // namespace scan::tea
