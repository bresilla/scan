#pragma once

/// @file confirm.hpp
/// @brief Yes/No confirmation dialog (Bubble Tea style)

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>

#include <optional>
#include <string>

namespace scan {

    /// Model for Confirm component
    struct ConfirmModel {
        std::string prompt;              // Question to ask
        std::string affirmative = "Yes"; // Affirmative option
        std::string negative = "No";     // Negative option
        bool value = true;               // Current selection (true = affirmative)

        // Styling - uses theme by default
        Color prompt_color;
        Color selected_fg;
        Color selected_bg;
        Color unselected_color;
        bool use_theme = true;

        // State
        bool submitted = false;
        bool cancelled = false;

        ConfirmModel() {
            auto &t = current_theme();
            prompt_color = t.colors.text;
            selected_fg = t.colors.text_inverted;
            selected_bg = t.colors.primary;
            unselected_color = t.colors.text_muted;
        }
    };

    /// Update function for Confirm
    inline std::pair<ConfirmModel, tea::Cmd> confirm_update(ConfirmModel m, const tea::Msg &msg) {
        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Enter:
                m.submitted = true;
                return {std::move(m), tea::quit()};

            case input::Key::Escape:
            case input::Key::CtrlC:
                m.cancelled = true;
                return {std::move(m), tea::quit()};

            case input::Key::Left:
            case input::Key::Right:
            case input::Key::Tab:
            case input::Key::CtrlN:
            case input::Key::CtrlP:
                m.value = !m.value;
                break;

            case input::Key::Rune:
                if (key->rune == 'y' || key->rune == 'Y') {
                    m.value = true;
                    m.submitted = true;
                    return {std::move(m), tea::quit()};
                }
                if (key->rune == 'n' || key->rune == 'N') {
                    m.value = false;
                    m.submitted = true;
                    return {std::move(m), tea::quit()};
                }
                if (key->rune == 'h' || key->rune == 'H') {
                    m.value = true;
                }
                if (key->rune == 'l' || key->rune == 'L') {
                    m.value = false;
                }
                break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    /// View function for Confirm
    inline std::string confirm_view(const ConfirmModel &m) {
        std::string view;

        // Prompt
        if (!m.prompt.empty()) {
            view += Style().foreground(m.prompt_color).bold().render(m.prompt);
            view += "\n\n";
        }

        // Options with proper button-like styling
        auto render_option = [&](const std::string &text, bool selected) -> std::string {
            Style s;
            if (selected) {
                s.foreground(m.selected_fg).background(m.selected_bg).bold();
            } else {
                s.foreground(m.unselected_color).faint();
            }
            return s.padding(0, 2).render(text);
        };

        std::string aff = render_option(m.affirmative, m.value);
        std::string neg = render_option(m.negative, !m.value);

        view += "  " + aff + "  " + neg;

        return view;
    }

    /// Builder class for Confirm component
    class Confirm {
      public:
        Confirm &prompt(const std::string &text) {
            m_model.prompt = text;
            return *this;
        }

        Confirm &affirmative(const std::string &text) {
            m_model.affirmative = text;
            return *this;
        }

        Confirm &negative(const std::string &text) {
            m_model.negative = text;
            return *this;
        }

        Confirm &default_value(bool value) {
            m_model.value = value;
            return *this;
        }

        Confirm &prompt_color(int r, int g, int b) {
            m_model.prompt_color = {r, g, b};
            return *this;
        }

        Confirm &selected_color(int fg_r, int fg_g, int fg_b, int bg_r, int bg_g, int bg_b) {
            m_model.selected_fg = {fg_r, fg_g, fg_b};
            m_model.selected_bg = {bg_r, bg_g, bg_b};
            return *this;
        }

        Confirm &unselected_color(int r, int g, int b) {
            m_model.unselected_color = {r, g, b};
            return *this;
        }

        std::optional<bool> run() {
            auto init = [this]() -> std::pair<ConfirmModel, tea::Cmd> { return {m_model, tea::none()}; };

            auto update = [](ConfirmModel m, tea::Msg msg) { return confirm_update(std::move(m), msg); };

            auto view = [](const ConfirmModel &m) { return confirm_view(m); };

            auto final_model = tea::Program<ConfirmModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            return final_model.value;
        }

        ConfirmModel model() const { return m_model; }

      private:
        ConfirmModel m_model;
    };

} // namespace scan
