#pragma once

/// @file viewport.hpp
/// @brief Scrollable content viewport component

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace scan {

    struct ViewportModel {
        std::vector<std::string> lines;
        int width = 80;
        int height = 20;
        int y_offset = 0;
        int x_offset = 0;
        bool wrap = true;

        // Styling - uses theme
        Color text_color;
        Color scroll_indicator_color;

        ViewportModel() {
            auto &t = current_theme();
            text_color = t.colors.text;
            scroll_indicator_color = t.colors.text_muted;
        }
    };

    inline void viewport_set_content(ViewportModel &m, const std::string &content) {
        m.lines.clear();
        std::istringstream iss(content);
        std::string line;
        while (std::getline(iss, line)) {
            if (m.wrap && static_cast<int>(line.length()) > m.width) {
                size_t pos = 0;
                while (pos < line.length()) {
                    size_t end = std::min(pos + static_cast<size_t>(m.width), line.length());
                    m.lines.push_back(line.substr(pos, end - pos));
                    pos = end;
                }
            } else {
                m.lines.push_back(line);
            }
        }
        if (m.lines.empty()) {
            m.lines.push_back("");
        }
    }

    inline void viewport_scroll_down(ViewportModel &m, int n = 1) {
        int max_offset = std::max(0, static_cast<int>(m.lines.size()) - m.height);
        m.y_offset = std::min(m.y_offset + n, max_offset);
    }

    inline void viewport_scroll_up(ViewportModel &m, int n = 1) { m.y_offset = std::max(0, m.y_offset - n); }

    inline void viewport_goto_top(ViewportModel &m) { m.y_offset = 0; }

    inline void viewport_goto_bottom(ViewportModel &m) {
        m.y_offset = std::max(0, static_cast<int>(m.lines.size()) - m.height);
    }

    inline void viewport_page_down(ViewportModel &m) { viewport_scroll_down(m, m.height); }
    inline void viewport_page_up(ViewportModel &m) { viewport_scroll_up(m, m.height); }
    inline void viewport_half_page_down(ViewportModel &m) { viewport_scroll_down(m, m.height / 2); }
    inline void viewport_half_page_up(ViewportModel &m) { viewport_scroll_up(m, m.height / 2); }

    inline bool viewport_at_top(const ViewportModel &m) { return m.y_offset == 0; }
    inline bool viewport_at_bottom(const ViewportModel &m) {
        return m.y_offset >= static_cast<int>(m.lines.size()) - m.height;
    }

    inline int viewport_scroll_percent(const ViewportModel &m) {
        if (m.lines.size() <= static_cast<size_t>(m.height)) {
            return 100;
        }
        int max_offset = static_cast<int>(m.lines.size()) - m.height;
        return (m.y_offset * 100) / max_offset;
    }

    inline std::pair<ViewportModel, tea::Cmd> viewport_update(ViewportModel m, const tea::Msg &msg) {
        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Up:
            case input::Key::CtrlP:
                viewport_scroll_up(m);
                break;

            case input::Key::Down:
            case input::Key::CtrlN:
                viewport_scroll_down(m);
                break;

            case input::Key::PageUp:
            case input::Key::CtrlB:
                viewport_page_up(m);
                break;

            case input::Key::PageDown:
            case input::Key::CtrlF:
            case input::Key::Space:
                viewport_page_down(m);
                break;

            case input::Key::Home:
                viewport_goto_top(m);
                break;

            case input::Key::End:
                viewport_goto_bottom(m);
                break;

            case input::Key::CtrlU:
                viewport_half_page_up(m);
                break;

            case input::Key::CtrlD:
                viewport_half_page_down(m);
                break;

            case input::Key::Left:
                if (!m.wrap && m.x_offset > 0) {
                    m.x_offset--;
                }
                break;

            case input::Key::Right:
                if (!m.wrap) {
                    m.x_offset++;
                }
                break;

            case input::Key::Rune:
                if (key->rune == 'j' || key->rune == 'J') {
                    viewport_scroll_down(m);
                } else if (key->rune == 'k' || key->rune == 'K') {
                    viewport_scroll_up(m);
                } else if (key->rune == 'g') {
                    viewport_goto_top(m);
                } else if (key->rune == 'G') {
                    viewport_goto_bottom(m);
                } else if (key->rune == 'd') {
                    viewport_half_page_down(m);
                } else if (key->rune == 'u') {
                    viewport_half_page_up(m);
                } else if (key->rune == 'h') {
                    if (!m.wrap && m.x_offset > 0)
                        m.x_offset--;
                } else if (key->rune == 'l') {
                    if (!m.wrap)
                        m.x_offset++;
                }
                break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    inline std::string viewport_view(const ViewportModel &m) {
        std::string view;

        size_t start = static_cast<size_t>(m.y_offset);
        size_t end = std::min(start + static_cast<size_t>(m.height), m.lines.size());

        for (size_t i = start; i < end; i++) {
            if (i > start) {
                view += "\n";
            }

            std::string line = m.lines[i];

            if (!m.wrap && m.x_offset > 0) {
                if (static_cast<int>(line.length()) > m.x_offset) {
                    line = line.substr(m.x_offset);
                } else {
                    line = "";
                }
            }

            if (static_cast<int>(line.length()) > m.width) {
                line = line.substr(0, m.width);
            }

            view += Style().foreground(m.text_color).render(line);
        }

        return view;
    }

    class Viewport {
      public:
        Viewport &content(const std::string &text) {
            viewport_set_content(m_model, text);
            return *this;
        }

        Viewport &width(int w) {
            m_model.width = w;
            return *this;
        }

        Viewport &height(int h) {
            m_model.height = h;
            return *this;
        }

        Viewport &wrap(bool enable) {
            m_model.wrap = enable;
            return *this;
        }

        std::string render() const { return viewport_view(m_model); }

        Viewport &scroll_down(int n = 1) {
            viewport_scroll_down(m_model, n);
            return *this;
        }

        Viewport &scroll_up(int n = 1) {
            viewport_scroll_up(m_model, n);
            return *this;
        }

        Viewport &goto_top() {
            viewport_goto_top(m_model);
            return *this;
        }

        Viewport &goto_bottom() {
            viewport_goto_bottom(m_model);
            return *this;
        }

        Viewport &page_down() {
            viewport_page_down(m_model);
            return *this;
        }

        Viewport &page_up() {
            viewport_page_up(m_model);
            return *this;
        }

        int scroll_percent() const { return viewport_scroll_percent(m_model); }
        bool at_top() const { return viewport_at_top(m_model); }
        bool at_bottom() const { return viewport_at_bottom(m_model); }

        ViewportModel &model() { return m_model; }
        const ViewportModel &model() const { return m_model; }

      private:
        ViewportModel m_model;
    };

} // namespace scan
