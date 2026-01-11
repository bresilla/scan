#pragma once

/// @file table.hpp
/// @brief Table display component with optional selection

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>

#include <algorithm>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace scan {

    /// Model for Table component
    struct TableModel {
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
        std::vector<int> widths;
        size_t cursor = 0;
        size_t offset = 0;
        int height = 10;
        bool selectable = false;
        BorderStyle border_style = BorderStyle::Rounded;

        // Styling - uses theme
        Color header_fg;
        Color header_bg;
        Color selected_fg;
        Color selected_bg;
        Color row_fg;
        Color border_color;
        Color muted_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        TableModel() {
            auto &t = current_theme();
            header_fg = t.colors.text;
            header_bg = t.colors.bg_muted;
            selected_fg = t.colors.text_inverted;
            selected_bg = t.colors.primary;
            row_fg = t.colors.text;
            border_color = t.colors.border;
            muted_color = t.colors.text_muted;
        }
    };

    /// Calculate column widths based on content
    inline std::vector<size_t> calculate_widths(const TableModel &m) {
        size_t num_cols = m.headers.size();
        std::vector<size_t> widths(num_cols, 0);

        for (size_t i = 0; i < num_cols; i++) {
            widths[i] = visible_width(m.headers[i]);
        }

        for (const auto &row : m.rows) {
            for (size_t i = 0; i < std::min(row.size(), num_cols); i++) {
                widths[i] = std::max(widths[i], visible_width(row[i]));
            }
        }

        for (size_t i = 0; i < std::min(m.widths.size(), num_cols); i++) {
            if (m.widths[i] > 0) {
                widths[i] = static_cast<size_t>(m.widths[i]);
            }
        }

        return widths;
    }

    /// Update function for Table
    inline std::pair<TableModel, tea::Cmd> table_update(TableModel m, const tea::Msg &msg) {
        if (!m.selectable) {
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

            case input::Key::Up:
            case input::Key::CtrlP:
                if (m.cursor > 0) {
                    m.cursor--;
                    if (m.cursor < m.offset) {
                        m.offset = m.cursor;
                    }
                }
                break;

            case input::Key::Down:
            case input::Key::CtrlN:
                if (!m.rows.empty() && m.cursor < m.rows.size() - 1) {
                    m.cursor++;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.cursor >= m.offset + visible) {
                        m.offset = m.cursor - visible + 1;
                    }
                }
                break;

            case input::Key::Home:
                m.cursor = 0;
                m.offset = 0;
                break;

            case input::Key::End:
                if (!m.rows.empty()) {
                    m.cursor = m.rows.size() - 1;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.rows.size() > visible) {
                        m.offset = m.rows.size() - visible;
                    }
                }
                break;

            case input::Key::Rune:
                if (key->rune == 'j' || key->rune == 'J') {
                    if (!m.rows.empty() && m.cursor < m.rows.size() - 1) {
                        m.cursor++;
                        size_t visible = static_cast<size_t>(m.height);
                        if (m.cursor >= m.offset + visible) {
                            m.offset = m.cursor - visible + 1;
                        }
                    }
                } else if (key->rune == 'k' || key->rune == 'K') {
                    if (m.cursor > 0) {
                        m.cursor--;
                        if (m.cursor < m.offset) {
                            m.offset = m.cursor;
                        }
                    }
                } else if (key->rune == 'g') {
                    m.cursor = 0;
                    m.offset = 0;
                } else if (key->rune == 'G') {
                    if (!m.rows.empty()) {
                        m.cursor = m.rows.size() - 1;
                        size_t visible = static_cast<size_t>(m.height);
                        if (m.rows.size() > visible) {
                            m.offset = m.rows.size() - visible;
                        }
                    }
                }
                break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    /// View function for Table
    inline std::string table_view(const TableModel &m) {
        if (m.headers.empty()) {
            return Style().foreground(m.muted_color).italic().render("(no columns)");
        }

        std::string view;
        auto widths = calculate_widths(m);
        auto chars = get_border_chars(m.border_style);

        auto sb = [&m](const std::string &s) { return Style().foreground(m.border_color).render(s); };

        // Top border
        view += sb(chars.top_left);
        for (size_t i = 0; i < widths.size(); i++) {
            view += sb(repeat(chars.horizontal, widths[i] + 2));
            view += sb(i < widths.size() - 1 ? chars.t_down : chars.top_right);
        }
        view += "\n";

        // Header row
        view += sb(chars.vertical);
        for (size_t i = 0; i < m.headers.size(); i++) {
            std::string cell = " " + pad_right(m.headers[i], widths[i]) + " ";
            view += Style().foreground(m.header_fg).bold().render(cell);
            view += sb(chars.vertical);
        }
        view += "\n";

        // Header separator
        view += sb(chars.t_right);
        for (size_t i = 0; i < widths.size(); i++) {
            view += sb(repeat(chars.horizontal, widths[i] + 2));
            view += sb(i < widths.size() - 1 ? chars.cross : chars.t_left);
        }
        view += "\n";

        // Data rows
        if (m.rows.empty()) {
            view += sb(chars.vertical);
            size_t total_w = 0;
            for (auto w : widths)
                total_w += w + 3;
            total_w -= 1;
            view += Style().foreground(m.muted_color).italic().render(pad_center("(no data)", total_w));
            view += sb(chars.vertical) + "\n";
        } else {
            size_t visible = static_cast<size_t>(m.height);
            size_t start = m.offset;
            size_t end = std::min(start + visible, m.rows.size());

            for (size_t r = start; r < end; r++) {
                bool is_selected = m.selectable && (r == m.cursor);

                view += sb(chars.vertical);
                for (size_t c = 0; c < m.headers.size(); c++) {
                    std::string content = (c < m.rows[r].size()) ? m.rows[r][c] : "";
                    std::string cell = " " + pad_right(content, widths[c]) + " ";

                    Style s;
                    if (is_selected) {
                        s.foreground(m.selected_fg).background(m.selected_bg);
                    } else {
                        s.foreground(m.row_fg);
                    }
                    view += s.render(cell);
                    view += sb(chars.vertical);
                }
                view += "\n";
            }
        }

        // Bottom border
        view += sb(chars.bottom_left);
        for (size_t i = 0; i < widths.size(); i++) {
            view += sb(repeat(chars.horizontal, widths[i] + 2));
            view += sb(i < widths.size() - 1 ? chars.t_up : chars.bottom_right);
        }

        // Scroll indicators
        if (m.selectable && !m.rows.empty()) {
            std::string ind;
            if (m.offset > 0)
                ind += "^";
            if (m.offset + static_cast<size_t>(m.height) < m.rows.size()) {
                if (!ind.empty())
                    ind += " ";
                ind += "v";
            }
            if (!ind.empty()) {
                view += "\n" + Style().foreground(m.muted_color).faint().render(ind);
            }
        }

        return view;
    }

    /// Builder class for Table component
    class Table {
      public:
        Table &headers(const std::vector<std::string> &h) {
            m_model.headers = h;
            return *this;
        }

        Table &rows(const std::vector<std::vector<std::string>> &r) {
            m_model.rows = r;
            return *this;
        }

        Table &widths(const std::vector<int> &w) {
            m_model.widths = w;
            return *this;
        }

        Table &border(BorderStyle style) {
            m_model.border_style = style;
            return *this;
        }

        Table &height(int h) {
            m_model.height = h;
            return *this;
        }

        Table &selectable(bool enable = true) {
            m_model.selectable = enable;
            return *this;
        }

        Table &header_style(int fg_r, int fg_g, int fg_b) {
            m_model.header_fg = {fg_r, fg_g, fg_b};
            return *this;
        }

        Table &selected_style(int fg_r, int fg_g, int fg_b, int bg_r, int bg_g, int bg_b) {
            m_model.selected_fg = {fg_r, fg_g, fg_b};
            m_model.selected_bg = {bg_r, bg_g, bg_b};
            return *this;
        }

        Table &border_color(int r, int g, int b) {
            m_model.border_color = {r, g, b};
            return *this;
        }

        std::string render() const { return table_view(m_model); }

        void print() const { std::cout << render() << "\n"; }

        std::optional<size_t> run() {
            m_model.selectable = true;

            auto init = [this]() -> std::pair<TableModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](TableModel m, tea::Msg msg) { return table_update(std::move(m), msg); };
            auto view = [](const TableModel &m) { return table_view(m); };

            auto final_model = tea::Program<TableModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            return final_model.cursor;
        }

        TableModel model() const { return m_model; }

      private:
        TableModel m_model;
    };

} // namespace scan
