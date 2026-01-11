#pragma once

/// @file pager.hpp
/// @brief Full-screen pager component for viewing large content

#include <scan/bubbles/viewport.hpp>
#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>
#include <scan/terminal/alt_screen.hpp>
#include <scan/terminal/terminal.hpp>

#include <algorithm>
#include <sstream>
#include <string>

namespace scan {

    struct PagerModel {
        ViewportModel viewport;
        std::string title;
        bool show_line_numbers = false;
        bool show_status_bar = true;
        bool quit_requested = false;

        // Styling - uses theme
        Color title_color;
        Color status_bg;
        Color status_fg;
        Color line_number_color;

        PagerModel() {
            auto &t = current_theme();
            title_color = t.colors.primary;
            status_bg = t.colors.bg_muted;
            status_fg = t.colors.text;
            line_number_color = t.colors.text_muted;
        }
    };

    inline void pager_init(PagerModel &m) {
        auto [cols, rows] = terminal::get_size();
        m.viewport.width = cols;
        int reserved = 0;
        if (!m.title.empty())
            reserved += 1;
        if (m.show_status_bar)
            reserved += 1;
        m.viewport.height = std::max(1, rows - reserved);
    }

    inline std::pair<PagerModel, tea::Cmd> pager_update(PagerModel m, const tea::Msg &msg) {
        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Escape:
            case input::Key::CtrlC:
                m.quit_requested = true;
                return {std::move(m), tea::quit()};

            case input::Key::Rune:
                if (key->rune == 'q' || key->rune == 'Q') {
                    m.quit_requested = true;
                    return {std::move(m), tea::quit()};
                }
                break;

            default:
                break;
            }
        }

        auto [new_viewport, cmd] = viewport_update(m.viewport, msg);
        m.viewport = std::move(new_viewport);

        return {std::move(m), cmd};
    }

    inline std::string pager_view(const PagerModel &m) {
        std::string view;

        // Title
        if (!m.title.empty()) {
            view += Style().foreground(m.title_color).bold().render(m.title);
            view += "\n";
        }

        // Content
        if (m.show_line_numbers) {
            size_t start = static_cast<size_t>(m.viewport.y_offset);
            size_t end = std::min(start + static_cast<size_t>(m.viewport.height), m.viewport.lines.size());

            int line_num_width = 1;
            size_t max_line = m.viewport.lines.size();
            while (max_line >= 10) {
                line_num_width++;
                max_line /= 10;
            }

            for (size_t i = start; i < end; i++) {
                if (i > start) {
                    view += "\n";
                }

                std::string line_num = std::to_string(i + 1);
                while (static_cast<int>(line_num.length()) < line_num_width) {
                    line_num = " " + line_num;
                }

                view += Style().foreground(m.line_number_color).faint().render(line_num + " | ");

                std::string line = m.viewport.lines[i];
                if (!m.viewport.wrap && m.viewport.x_offset > 0) {
                    if (static_cast<int>(line.length()) > m.viewport.x_offset) {
                        line = line.substr(m.viewport.x_offset);
                    } else {
                        line = "";
                    }
                }

                int available_width = m.viewport.width - line_num_width - 3;
                if (static_cast<int>(line.length()) > available_width) {
                    line = line.substr(0, available_width);
                }

                view += line;
            }
        } else {
            view += viewport_view(m.viewport);
        }

        // Status bar
        if (m.show_status_bar) {
            view += "\n";

            int percent = viewport_scroll_percent(m.viewport);
            std::string pos_info;
            if (viewport_at_top(m.viewport) && viewport_at_bottom(m.viewport)) {
                pos_info = "All";
            } else if (viewport_at_top(m.viewport)) {
                pos_info = "Top";
            } else if (viewport_at_bottom(m.viewport)) {
                pos_info = "End";
            } else {
                pos_info = std::to_string(percent) + "%";
            }

            std::string line_info =
                "L " + std::to_string(m.viewport.y_offset + 1) + "/" + std::to_string(m.viewport.lines.size());

            std::string help = "q:quit j/k:scroll g/G:top/end";

            std::string status = " " + line_info;

            int pad = m.viewport.width - static_cast<int>(status.length()) - static_cast<int>(pos_info.length()) -
                      static_cast<int>(help.length()) - 4;

            if (pad > 0) {
                int left_pad = pad / 2;
                int right_pad = pad - left_pad;
                status += std::string(left_pad, ' ');
                status += help;
                status += std::string(right_pad, ' ');
            }

            status += " " + pos_info + " ";

            while (static_cast<int>(status.length()) < m.viewport.width) {
                status += " ";
            }
            if (static_cast<int>(status.length()) > m.viewport.width) {
                status = status.substr(0, m.viewport.width);
            }

            view += Style().foreground(m.status_fg).background(m.status_bg).render(status);
        }

        return view;
    }

    class Pager {
      public:
        Pager &content(const std::string &text) {
            viewport_set_content(m_model.viewport, text);
            return *this;
        }

        Pager &title(const std::string &t) {
            m_model.title = t;
            return *this;
        }

        Pager &line_numbers(bool show = true) {
            m_model.show_line_numbers = show;
            return *this;
        }

        Pager &status_bar(bool show = true) {
            m_model.show_status_bar = show;
            return *this;
        }

        Pager &wrap(bool enable) {
            m_model.viewport.wrap = enable;
            return *this;
        }

        Pager &title_color(int r, int g, int b) {
            m_model.title_color = {r, g, b};
            return *this;
        }

        Pager &status_colors(int fg_r, int fg_g, int fg_b, int bg_r, int bg_g, int bg_b) {
            m_model.status_fg = {fg_r, fg_g, fg_b};
            m_model.status_bg = {bg_r, bg_g, bg_b};
            return *this;
        }

        void run() {
            pager_init(m_model);

            auto init = [this]() -> std::pair<PagerModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](PagerModel m, tea::Msg msg) { return pager_update(std::move(m), msg); };
            auto view = [](const PagerModel &m) { return pager_view(m); };

            tea::Program<PagerModel>(init, update, view).with_alt_screen(true).run();
        }

        PagerModel &model() { return m_model; }
        const PagerModel &model() const { return m_model; }

      private:
        PagerModel m_model;
    };

} // namespace scan
