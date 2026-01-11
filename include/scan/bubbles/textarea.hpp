#pragma once

/// @file textarea.hpp
/// @brief Multi-line text input component (Bubble Tea style)

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>
#include <scan/util/utf8.hpp>

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace scan {

    /// Model for TextArea component
    struct TextAreaModel {
        std::vector<std::string> lines{""};
        size_t cursor_row = 0;
        size_t cursor_col = 0;
        size_t offset_row = 0;
        int width = 60;
        int height = 10;
        std::string placeholder;
        bool show_line_numbers = false;
        bool focused = true;

        // Styling - uses theme
        Color text_color;
        Color line_number_color;
        Color placeholder_color;
        Color muted_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        TextAreaModel() {
            auto &t = current_theme();
            text_color = t.colors.text;
            line_number_color = t.colors.text_muted;
            placeholder_color = t.colors.text_subtle;
            muted_color = t.colors.text_muted;
        }
    };

    inline std::string textarea_get_value(const TextAreaModel &m) {
        std::string result;
        for (size_t i = 0; i < m.lines.size(); i++) {
            if (i > 0)
                result += "\n";
            result += m.lines[i];
        }
        return result;
    }

    inline void textarea_set_value(TextAreaModel &m, const std::string &text) {
        m.lines.clear();
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line)) {
            m.lines.push_back(line);
        }
        if (m.lines.empty()) {
            m.lines.push_back("");
        }
        m.cursor_row = 0;
        m.cursor_col = 0;
    }

    inline std::pair<TextAreaModel, tea::Cmd> textarea_update(TextAreaModel m, const tea::Msg &msg) {
        if (!m.focused) {
            return {std::move(m), tea::none()};
        }

        if (m.lines.empty()) {
            m.lines.push_back("");
        }

        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::CtrlD:
            case input::Key::Escape:
                m.submitted = true;
                return {std::move(m), tea::quit()};

            case input::Key::CtrlC:
                m.cancelled = true;
                return {std::move(m), tea::quit()};

            case input::Key::Enter: {
                std::string &current = m.lines[m.cursor_row];
                std::string before = utf8::substring(current, 0, m.cursor_col);
                std::string after = utf8::substring(current, m.cursor_col);
                m.lines[m.cursor_row] = before;
                m.lines.insert(m.lines.begin() + m.cursor_row + 1, after);
                m.cursor_row++;
                m.cursor_col = 0;
                size_t visible = static_cast<size_t>(m.height);
                if (m.cursor_row >= m.offset_row + visible) {
                    m.offset_row = m.cursor_row - visible + 1;
                }
            } break;

            case input::Key::Backspace:
            case input::Key::CtrlH:
                if (m.cursor_col > 0) {
                    m.lines[m.cursor_row] = utf8::erase(m.lines[m.cursor_row], m.cursor_col - 1, 1);
                    m.cursor_col--;
                } else if (m.cursor_row > 0) {
                    size_t prev_len = utf8::length(m.lines[m.cursor_row - 1]);
                    m.lines[m.cursor_row - 1] += m.lines[m.cursor_row];
                    m.lines.erase(m.lines.begin() + m.cursor_row);
                    m.cursor_row--;
                    m.cursor_col = prev_len;
                    if (m.cursor_row < m.offset_row) {
                        m.offset_row = m.cursor_row;
                    }
                }
                break;

            case input::Key::Delete: {
                size_t line_len = utf8::length(m.lines[m.cursor_row]);
                if (m.cursor_col < line_len) {
                    m.lines[m.cursor_row] = utf8::erase(m.lines[m.cursor_row], m.cursor_col, 1);
                } else if (m.cursor_row < m.lines.size() - 1) {
                    m.lines[m.cursor_row] += m.lines[m.cursor_row + 1];
                    m.lines.erase(m.lines.begin() + m.cursor_row + 1);
                }
            } break;

            case input::Key::Left:
            case input::Key::CtrlB:
                if (m.cursor_col > 0) {
                    m.cursor_col--;
                } else if (m.cursor_row > 0) {
                    m.cursor_row--;
                    m.cursor_col = utf8::length(m.lines[m.cursor_row]);
                    if (m.cursor_row < m.offset_row) {
                        m.offset_row = m.cursor_row;
                    }
                }
                break;

            case input::Key::Right:
            case input::Key::CtrlF: {
                size_t line_len = utf8::length(m.lines[m.cursor_row]);
                if (m.cursor_col < line_len) {
                    m.cursor_col++;
                } else if (m.cursor_row < m.lines.size() - 1) {
                    m.cursor_row++;
                    m.cursor_col = 0;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.cursor_row >= m.offset_row + visible) {
                        m.offset_row = m.cursor_row - visible + 1;
                    }
                }
            } break;

            case input::Key::Up:
            case input::Key::CtrlP:
                if (m.cursor_row > 0) {
                    m.cursor_row--;
                    m.cursor_col = std::min(m.cursor_col, utf8::length(m.lines[m.cursor_row]));
                    if (m.cursor_row < m.offset_row) {
                        m.offset_row = m.cursor_row;
                    }
                }
                break;

            case input::Key::Down:
            case input::Key::CtrlN:
                if (m.cursor_row < m.lines.size() - 1) {
                    m.cursor_row++;
                    m.cursor_col = std::min(m.cursor_col, utf8::length(m.lines[m.cursor_row]));
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.cursor_row >= m.offset_row + visible) {
                        m.offset_row = m.cursor_row - visible + 1;
                    }
                }
                break;

            case input::Key::Home:
            case input::Key::CtrlA:
                m.cursor_col = 0;
                break;

            case input::Key::End:
            case input::Key::CtrlE:
                m.cursor_col = utf8::length(m.lines[m.cursor_row]);
                break;

            case input::Key::CtrlK:
                m.lines[m.cursor_row] = utf8::substring(m.lines[m.cursor_row], 0, m.cursor_col);
                break;

            case input::Key::CtrlU:
                m.lines[m.cursor_row] = utf8::substring(m.lines[m.cursor_row], m.cursor_col);
                m.cursor_col = 0;
                break;

            case input::Key::Rune:
            case input::Key::Space: {
                std::string ch = utf8::encode(key->rune);
                m.lines[m.cursor_row] = utf8::insert(m.lines[m.cursor_row], m.cursor_col, ch);
                m.cursor_col++;
            } break;

            case input::Key::Tab: {
                std::string tab = "    ";
                m.lines[m.cursor_row] = utf8::insert(m.lines[m.cursor_row], m.cursor_col, tab);
                m.cursor_col += 4;
            } break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    inline std::string textarea_view(const TextAreaModel &m) {
        std::string view;

        bool is_empty = m.lines.empty() || (m.lines.size() == 1 && m.lines[0].empty());

        if (is_empty && !m.placeholder.empty() && !m.focused) {
            return Style().foreground(m.placeholder_color).italic().render(m.placeholder);
        }

        int line_num_width = 0;
        if (m.show_line_numbers) {
            line_num_width = static_cast<int>(std::to_string(m.lines.size()).length()) + 2;
        }

        size_t visible = static_cast<size_t>(m.height);
        size_t start = m.offset_row;
        size_t end = std::min(start + visible, m.lines.size());

        for (size_t i = start; i < end; i++) {
            bool is_cursor_line = (i == m.cursor_row);

            // Line number
            if (m.show_line_numbers) {
                std::string num = std::to_string(i + 1);
                while (static_cast<int>(num.length()) < line_num_width - 2) {
                    num = " " + num;
                }
                num += " ";
                view += Style().foreground(m.line_number_color).faint().render(num);
            }

            // Line content
            const std::string &line = m.lines[i];

            if (is_cursor_line && m.focused) {
                size_t line_len = utf8::length(line);
                std::string before = utf8::substring(line, 0, m.cursor_col);
                std::string cursor_char = m.cursor_col < line_len ? utf8::substring(line, m.cursor_col, 1) : " ";
                std::string after = m.cursor_col < line_len ? utf8::substring(line, m.cursor_col + 1) : "";

                view += Style().foreground(m.text_color).render(before);
                view += Style().reverse().render(cursor_char);
                view += Style().foreground(m.text_color).render(after);
            } else {
                view += Style().foreground(m.text_color).render(line);
            }

            if (i < end - 1) {
                view += "\n";
            }
        }

        // Scroll indicators
        std::string ind;
        if (start > 0)
            ind += "^";
        if (end < m.lines.size()) {
            if (!ind.empty())
                ind += " ";
            ind += "v";
        }
        if (!ind.empty()) {
            view += "\n" + Style().foreground(m.muted_color).faint().render(ind);
        }

        // Help text
        view += "\n" + Style().foreground(m.muted_color).italic().render("Ctrl+D submit, Ctrl+C cancel");

        return view;
    }

    class TextArea {
      public:
        TextArea &placeholder(const std::string &text) {
            m_model.placeholder = text;
            return *this;
        }

        TextArea &value(const std::string &text) {
            textarea_set_value(m_model, text);
            return *this;
        }

        TextArea &width(int w) {
            m_model.width = w;
            return *this;
        }

        TextArea &height(int h) {
            m_model.height = h;
            return *this;
        }

        TextArea &show_line_numbers(bool show = true) {
            m_model.show_line_numbers = show;
            return *this;
        }

        TextArea &text_color(int r, int g, int b) {
            m_model.text_color = {r, g, b};
            return *this;
        }

        std::optional<std::string> run() {
            if (m_model.lines.empty()) {
                m_model.lines.push_back("");
            }

            auto init = [this]() -> std::pair<TextAreaModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](TextAreaModel m, tea::Msg msg) { return textarea_update(std::move(m), msg); };
            auto view = [](const TextAreaModel &m) { return textarea_view(m); };

            auto final_model = tea::Program<TextAreaModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            return textarea_get_value(final_model);
        }

        TextAreaModel model() const { return m_model; }

      private:
        TextAreaModel m_model;
    };

} // namespace scan
