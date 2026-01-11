#pragma once

/// @file filter.hpp
/// @brief Fuzzy filter component for searching and selecting from a list

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>
#include <scan/util/fuzzy.hpp>
#include <scan/util/utf8.hpp>

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace scan {

    /// Model for Filter component
    struct FilterModel {
        std::vector<std::string> items;
        std::vector<size_t> filtered;
        std::set<size_t> selected;
        std::string query;
        size_t cursor = 0;
        size_t offset = 0;
        int height = 10;
        size_t limit = 1;
        bool case_sensitive = false;

        // Display
        std::string prompt = "> ";
        std::string placeholder = "Type to filter...";
        std::string selected_prefix = "[x]";
        std::string unselected_prefix = "[ ]";

        // Styling - uses theme
        Color prompt_color;
        Color text_color;
        Color match_color;
        Color cursor_color;
        Color selected_color;
        Color muted_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        FilterModel() {
            auto &t = current_theme();
            prompt_color = t.colors.primary;
            text_color = t.colors.text;
            match_color = t.colors.match_highlight;
            cursor_color = t.colors.primary;
            selected_color = t.colors.success;
            muted_color = t.colors.text_muted;
        }
    };

    /// Update function for Filter
    inline std::pair<FilterModel, tea::Cmd> filter_update(FilterModel m, const tea::Msg &msg) {
        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Enter:
                if (m.limit == 1 && m.selected.empty() && !m.filtered.empty()) {
                    m.selected.insert(m.filtered[m.cursor]);
                }
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
                if (!m.filtered.empty() && m.cursor < m.filtered.size() - 1) {
                    m.cursor++;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.cursor >= m.offset + visible) {
                        m.offset = m.cursor - visible + 1;
                    }
                }
                break;

            case input::Key::Tab:
                if (m.limit != 1 && !m.filtered.empty()) {
                    size_t orig_idx = m.filtered[m.cursor];
                    if (m.selected.count(orig_idx)) {
                        m.selected.erase(orig_idx);
                    } else {
                        if (m.limit == 0 || m.selected.size() < m.limit) {
                            m.selected.insert(orig_idx);
                        }
                    }
                }
                break;

            case input::Key::Backspace:
            case input::Key::CtrlH:
                if (!m.query.empty()) {
                    m.query = utf8::erase(m.query, utf8::length(m.query) - 1, 1);
                    m.filtered = fuzzy::filter(m.items, m.query, m.case_sensitive);
                    m.cursor = 0;
                    m.offset = 0;
                }
                break;

            case input::Key::CtrlU:
                m.query.clear();
                m.filtered = fuzzy::filter(m.items, m.query, m.case_sensitive);
                m.cursor = 0;
                m.offset = 0;
                break;

            case input::Key::Space:
            case input::Key::Rune: {
                std::string ch = utf8::encode(key->rune);
                m.query += ch;
                m.filtered = fuzzy::filter(m.items, m.query, m.case_sensitive);
                m.cursor = 0;
                m.offset = 0;
            } break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    /// Highlight matching characters in a string
    inline std::string highlight_matches(const std::string &text, const std::string &query, const Color &match_color,
                                         const Color &normal_color, bool case_sensitive = false) {
        if (query.empty()) {
            return Style().foreground(normal_color).render(text);
        }

        auto positions = fuzzy::get_match_positions(query, text, case_sensitive);
        std::set<size_t> pos_set(positions.begin(), positions.end());

        std::string result;
        auto codepoints = utf8::decode(text);

        for (size_t i = 0; i < codepoints.size(); i++) {
            std::string ch = utf8::encode(codepoints[i]);
            Style s;
            if (pos_set.count(i)) {
                s.foreground(match_color).bold().underline();
            } else {
                s.foreground(normal_color);
            }
            result += s.render(ch);
        }

        return result;
    }

    /// View function for Filter
    inline std::string filter_view(const FilterModel &m) {
        std::string view;

        // Search input line
        view += Style().foreground(m.prompt_color).bold().render(m.prompt);

        if (m.query.empty()) {
            view += Style().foreground(m.muted_color).italic().render(m.placeholder);
        } else {
            view += Style().foreground(m.text_color).render(m.query);
        }
        view += Style().reverse().render(" "); // Cursor
        view += "\n";

        // Results count
        std::string count_str = std::to_string(m.filtered.size()) + "/" + std::to_string(m.items.size());
        view += Style().foreground(m.muted_color).faint().render("  " + count_str) + "\n\n";

        if (m.filtered.empty()) {
            view += Style().foreground(m.muted_color).italic().render("  No matches");
            return view;
        }

        // Filtered items
        size_t visible = static_cast<size_t>(m.height);
        size_t start = m.offset;
        size_t end = std::min(start + visible, m.filtered.size());

        // Scroll indicator top
        if (start > 0) {
            view += Style().foreground(m.muted_color).faint().render("  ^ more") + "\n";
        }

        for (size_t i = start; i < end; i++) {
            size_t orig_idx = m.filtered[i];
            bool is_cursor = (i == m.cursor);
            bool is_selected = m.selected.count(orig_idx) > 0;

            std::string line;

            // Cursor indicator
            if (is_cursor) {
                line += Style().foreground(m.cursor_color).bold().render("> ");
            } else {
                line += "  ";
            }

            // Selection checkbox (multi-select)
            if (m.limit != 1) {
                if (is_selected) {
                    line += Style().foreground(m.selected_color).render(m.selected_prefix + " ");
                } else {
                    line += Style().foreground(m.muted_color).faint().render(m.unselected_prefix + " ");
                }
            }

            // Item text with highlighting
            Color text_col = is_cursor ? m.cursor_color : is_selected ? m.selected_color : m.text_color;
            line += highlight_matches(m.items[orig_idx], m.query, m.match_color, text_col, m.case_sensitive);

            view += line;
            if (i < end - 1) {
                view += "\n";
            }
        }

        // Scroll indicator bottom
        if (end < m.filtered.size()) {
            view += "\n";
            view += Style().foreground(m.muted_color).faint().render("  v more");
        }

        return view;
    }

    /// Builder class for Filter component
    class Filter {
      public:
        Filter &items(const std::vector<std::string> &items) {
            m_model.items = items;
            m_model.filtered = fuzzy::filter(items, "", false);
            return *this;
        }

        Filter &placeholder(const std::string &text) {
            m_model.placeholder = text;
            return *this;
        }

        Filter &prompt(const std::string &text) {
            m_model.prompt = text;
            return *this;
        }

        Filter &query(const std::string &text) {
            m_model.query = text;
            m_model.filtered = fuzzy::filter(m_model.items, text, m_model.case_sensitive);
            return *this;
        }

        Filter &limit(size_t max) {
            m_model.limit = max;
            return *this;
        }

        Filter &no_limit() {
            m_model.limit = 0;
            return *this;
        }

        Filter &height(int h) {
            m_model.height = h;
            return *this;
        }

        Filter &case_sensitive(bool enabled) {
            m_model.case_sensitive = enabled;
            return *this;
        }

        Filter &match_color(int r, int g, int b) {
            m_model.match_color = {r, g, b};
            return *this;
        }

        Filter &prompt_color(int r, int g, int b) {
            m_model.prompt_color = {r, g, b};
            return *this;
        }

        std::optional<std::string> run() {
            m_model.limit = 1;

            auto init = [this]() -> std::pair<FilterModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](FilterModel m, tea::Msg msg) { return filter_update(std::move(m), msg); };
            auto view = [](const FilterModel &m) { return filter_view(m); };

            auto final_model = tea::Program<FilterModel>(init, update, view).run();

            if (final_model.cancelled || final_model.selected.empty()) {
                return std::nullopt;
            }

            size_t idx = *final_model.selected.begin();
            return idx < final_model.items.size() ? std::optional{final_model.items[idx]} : std::nullopt;
        }

        std::optional<std::vector<std::string>> run_multi() {
            if (m_model.limit == 1) {
                m_model.limit = 0;
            }

            auto init = [this]() -> std::pair<FilterModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](FilterModel m, tea::Msg msg) { return filter_update(std::move(m), msg); };
            auto view = [](const FilterModel &m) { return filter_view(m); };

            auto final_model = tea::Program<FilterModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            std::vector<std::string> result;
            for (size_t idx : final_model.selected) {
                if (idx < final_model.items.size()) {
                    result.push_back(final_model.items[idx]);
                }
            }

            return result;
        }

        FilterModel model() const { return m_model; }

      private:
        FilterModel m_model;
    };

} // namespace scan
