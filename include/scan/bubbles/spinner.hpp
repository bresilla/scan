#pragma once

/// @file spinner.hpp
/// @brief Spinner component for showing progress during async tasks

#include <echo/widget.hpp>
#include <scan/style/style.hpp>
#include <scan/terminal/terminal.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

namespace scan {

    // Re-export echo's spinner styles for convenience
    using SpinnerStyle = echo::spinner_style;

    struct SpinnerModel {
        std::string title;
        SpinnerStyle style = SpinnerStyle::line;
        std::chrono::milliseconds interval{100};

        // Styling
        std::string color; // Hex color like "#FF5733"
        std::vector<std::string> gradient;

        SpinnerModel() {
            auto &t = current_theme();
            // Convert theme color to hex
            char buf[8];
            std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", t.colors.primary.r, t.colors.primary.g,
                          t.colors.primary.b);
            color = buf;
        }
    };

    class Spinner {
      public:
        Spinner &title(const std::string &text) {
            m_model.title = text;
            return *this;
        }

        Spinner &style(SpinnerStyle s) {
            m_model.style = s;
            return *this;
        }

        Spinner &color(int r, int g, int b) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
            m_model.color = buf;
            return *this;
        }

        Spinner &color(const std::string &hex) {
            m_model.color = hex;
            return *this;
        }

        Spinner &gradient(const std::vector<std::string> &colors) {
            m_model.gradient = colors;
            return *this;
        }

        Spinner &interval(int ms) {
            m_model.interval = std::chrono::milliseconds(ms);
            return *this;
        }

        template <typename F> auto run(F &&task) -> decltype(task()) {
            using ResultType = decltype(task());

            terminal::hide_cursor();

            std::atomic<bool> done{false};
            std::future<ResultType> future = std::async(std::launch::async, [&]() {
                auto result = task();
                done = true;
                return result;
            });

            echo::spinner spin(m_model.style);
            spin.set_message(m_model.title);

            if (!m_model.gradient.empty()) {
                spin.set_gradient(m_model.gradient);
            } else if (!m_model.color.empty()) {
                spin.set_color(m_model.color);
            }

            while (!done) {
                spin.tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(spin.get_interval_ms()));
            }

            spin.stop();
            terminal::show_cursor();

            return future.get();
        }

        void run_until_cancelled() {
            terminal::hide_cursor();

            echo::spinner spin(m_model.style);
            spin.set_message(m_model.title);

            if (!m_model.gradient.empty()) {
                spin.set_gradient(m_model.gradient);
            } else if (!m_model.color.empty()) {
                spin.set_color(m_model.color);
            }

            for (int i = 0; i < 50; i++) {
                spin.tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(spin.get_interval_ms()));
            }

            spin.stop();
            terminal::show_cursor();
        }

        SpinnerModel model() const { return m_model; }

      private:
        SpinnerModel m_model;
    };

} // namespace scan
