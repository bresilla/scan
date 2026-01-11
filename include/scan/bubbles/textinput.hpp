#pragma once

/// @file textinput.hpp
/// @brief Single-line text input component (Bubble Tea style)

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>
#include <scan/util/utf8.hpp>

#include <functional>
#include <optional>
#include <string>

namespace scan {

    /// Model for TextInput component
    struct TextInputModel {
        std::string value;         // Current input value
        std::string placeholder;   // Placeholder text
        std::string prompt = "> "; // Prompt string
        size_t cursor = 0;         // Cursor position (codepoint index)
        int width = 40;            // Input width
        int char_limit = 0;        // Max characters (0 = unlimited)
        bool focused = true;       // Is focused
        bool password = false;     // Password mode
        char mask_char = '*';      // Mask character for password

        // Styling - uses theme
        Color prompt_color;
        Color text_color;
        Color placeholder_color;
        Color cursor_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        TextInputModel() {
            auto &t = current_theme();
            prompt_color = t.colors.primary;
            text_color = t.colors.text;
            placeholder_color = t.colors.text_subtle;
            cursor_color = t.colors.primary;
        }
    };

    /// Update function for TextInput
    inline std::pair<TextInputModel, tea::Cmd> textinput_update(TextInputModel m, const tea::Msg &msg) {
        if (!m.focused) {
            return {std::move(m), tea::none()};
        }

        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Enter:
                m.submitted = true;
                return {std::move(m), tea::quit()};

            case input::Key::Escape:
            case input::Key::CtrlC:
                m.cancelled = true;
                return {std::move(m), tea::quit()};

            case input::Key::Backspace:
            case input::Key::CtrlH:
                if (m.cursor > 0) {
                    m.value = utf8::erase(m.value, m.cursor - 1, 1);
                    m.cursor--;
                }
                break;

            case input::Key::Delete:
            case input::Key::CtrlD:
                if (m.cursor < utf8::length(m.value)) {
                    m.value = utf8::erase(m.value, m.cursor, 1);
                }
                break;

            case input::Key::Left:
            case input::Key::CtrlB:
                if (m.cursor > 0)
                    m.cursor--;
                break;

            case input::Key::Right:
            case input::Key::CtrlF:
                if (m.cursor < utf8::length(m.value))
                    m.cursor++;
                break;

            case input::Key::Home:
            case input::Key::CtrlA:
                m.cursor = 0;
                break;

            case input::Key::End:
            case input::Key::CtrlE:
                m.cursor = utf8::length(m.value);
                break;

            case input::Key::CtrlK:
                m.value = utf8::substring(m.value, 0, m.cursor);
                break;

            case input::Key::CtrlU:
                m.value = utf8::substring(m.value, m.cursor);
                m.cursor = 0;
                break;

            case input::Key::CtrlW:
                if (m.cursor > 0) {
                    size_t new_cursor = m.cursor - 1;
                    auto codepoints = utf8::decode(m.value);
                    while (new_cursor > 0 && codepoints[new_cursor] == ' ')
                        new_cursor--;
                    while (new_cursor > 0 && codepoints[new_cursor - 1] != ' ')
                        new_cursor--;
                    m.value = utf8::erase(m.value, new_cursor, m.cursor - new_cursor);
                    m.cursor = new_cursor;
                }
                break;

            case input::Key::Rune:
            case input::Key::Space:
                if (m.char_limit == 0 || utf8::length(m.value) < static_cast<size_t>(m.char_limit)) {
                    std::string ch = utf8::encode(key->rune);
                    m.value = utf8::insert(m.value, m.cursor, ch);
                    m.cursor++;
                }
                break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    /// View function for TextInput
    inline std::string textinput_view(const TextInputModel &m) {
        std::string view;

        // Prompt
        if (!m.prompt.empty()) {
            view += Style().foreground(m.prompt_color).bold().render(m.prompt);
        }

        // Display value
        std::string display_value;
        bool show_placeholder = m.value.empty() && !m.placeholder.empty();

        if (show_placeholder) {
            display_value = m.placeholder;
        } else if (m.password) {
            size_t len = utf8::length(m.value);
            display_value = std::string(len, m.mask_char);
        } else {
            display_value = m.value;
        }

        // Render with cursor
        if (m.focused && !show_placeholder) {
            size_t len = utf8::length(display_value);
            std::string before = utf8::substring(display_value, 0, m.cursor);
            std::string cursor_char = m.cursor < len ? utf8::substring(display_value, m.cursor, 1) : " ";
            std::string after = m.cursor < len ? utf8::substring(display_value, m.cursor + 1) : "";

            view += Style().foreground(m.text_color).render(before);
            view += Style().reverse().render(cursor_char);
            view += Style().foreground(m.text_color).render(after);
        } else {
            Style s;
            if (show_placeholder) {
                s.foreground(m.placeholder_color).italic();
            } else {
                s.foreground(m.text_color);
            }
            view += s.render(display_value);
        }

        return view;
    }

    /// Builder class for TextInput component
    class TextInput {
      public:
        TextInput &placeholder(const std::string &text) {
            m_model.placeholder = text;
            return *this;
        }

        TextInput &prompt(const std::string &text) {
            m_model.prompt = text;
            return *this;
        }

        TextInput &value(const std::string &text) {
            m_model.value = text;
            m_model.cursor = utf8::length(text);
            return *this;
        }

        TextInput &password(bool enabled = true) {
            m_model.password = enabled;
            return *this;
        }

        TextInput &mask(char c) {
            m_model.mask_char = c;
            return *this;
        }

        TextInput &char_limit(int limit) {
            m_model.char_limit = limit;
            return *this;
        }

        TextInput &width(int w) {
            m_model.width = w;
            return *this;
        }

        TextInput &prompt_color(int r, int g, int b) {
            m_model.prompt_color = {r, g, b};
            return *this;
        }

        TextInput &text_color(int r, int g, int b) {
            m_model.text_color = {r, g, b};
            return *this;
        }

        TextInput &placeholder_color(int r, int g, int b) {
            m_model.placeholder_color = {r, g, b};
            return *this;
        }

        std::optional<std::string> run() {
            auto init = [this]() -> std::pair<TextInputModel, tea::Cmd> { return {m_model, tea::none()}; };

            auto update = [](TextInputModel m, tea::Msg msg) { return textinput_update(std::move(m), msg); };

            auto view = [](const TextInputModel &m) { return textinput_view(m); };

            auto final_model = tea::Program<TextInputModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            return final_model.value;
        }

        TextInputModel model() const { return m_model; }

      private:
        TextInputModel m_model;
    };

} // namespace scan
