#pragma once

/// @file list.hpp
/// @brief Selection list component (Bubble Tea style)

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace scan {

    /// A list item with optional title and description (two-line display)
    struct ListItem {
        std::string title;
        std::string description;

        ListItem() = default;
        ListItem(const std::string &t) : title(t) {}
        ListItem(const std::string &t, const std::string &d) : title(t), description(d) {}

        // Allow implicit conversion from string
        ListItem(const char *t) : title(t) {}

        bool has_description() const { return !description.empty(); }
    };

    /// Model for List component
    struct ListModel {
        std::vector<std::string> items;   // Simple items (legacy)
        std::vector<ListItem> rich_items; // Rich items with title + description
        bool use_rich_items = false;

        std::set<size_t> selected;
        size_t cursor = 0;
        size_t offset = 0;
        int height = 10;
        size_t limit = 1; // 1 = single, 0 = unlimited

        // Display strings
        std::string cursor_char = ">";
        std::string selected_prefix = "[x]";
        std::string unselected_prefix = "[ ]";

        // Styling - uses theme
        Color cursor_color;
        Color selected_color;
        Color normal_color;
        Color muted_color;
        Color desc_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        ListModel() {
            auto &t = current_theme();
            cursor_color = t.colors.primary;
            selected_color = t.colors.success;
            normal_color = t.colors.text;
            muted_color = t.colors.text_muted;
            desc_color = t.colors.text_subtle;
        }

        size_t item_count() const { return use_rich_items ? rich_items.size() : items.size(); }

        std::string get_title(size_t i) const {
            if (use_rich_items && i < rich_items.size()) {
                return rich_items[i].title;
            }
            return i < items.size() ? items[i] : "";
        }

        std::string get_description(size_t i) const {
            if (use_rich_items && i < rich_items.size()) {
                return rich_items[i].description;
            }
            return "";
        }

        bool has_description(size_t i) const {
            if (use_rich_items && i < rich_items.size()) {
                return rich_items[i].has_description();
            }
            return false;
        }
    };

    /// Update function for List
    inline std::pair<ListModel, tea::Cmd> list_update(ListModel m, const tea::Msg &msg) {
        size_t count = m.item_count();
        if (count == 0) {
            return {std::move(m), tea::none()};
        }

        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Enter:
                if (m.limit == 1 && m.selected.empty()) {
                    m.selected.insert(m.cursor);
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
                if (m.cursor < count - 1) {
                    m.cursor++;
                    size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
                    if (m.cursor >= m.offset + visible) {
                        m.offset = m.cursor - visible + 1;
                    }
                }
                break;

            case input::Key::Home:
            case input::Key::CtrlA:
                m.cursor = 0;
                m.offset = 0;
                break;

            case input::Key::End:
            case input::Key::CtrlE:
                m.cursor = count - 1;
                {
                    size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
                    if (count > visible) {
                        m.offset = count - visible;
                    }
                }
                break;

            case input::Key::PageUp: {
                size_t page = m.height > 0 ? static_cast<size_t>(m.height) : 10;
                m.cursor = m.cursor >= page ? m.cursor - page : 0;
                if (m.cursor < m.offset) {
                    m.offset = m.cursor;
                }
            } break;

            case input::Key::PageDown: {
                size_t page = m.height > 0 ? static_cast<size_t>(m.height) : 10;
                m.cursor = std::min(m.cursor + page, count - 1);
                size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
                if (m.cursor >= m.offset + visible) {
                    m.offset = m.cursor - visible + 1;
                }
            } break;

            case input::Key::Space:
            case input::Key::Tab:
                if (m.limit != 1) {
                    if (m.selected.count(m.cursor)) {
                        m.selected.erase(m.cursor);
                    } else {
                        if (m.limit == 0 || m.selected.size() < m.limit) {
                            m.selected.insert(m.cursor);
                        }
                    }
                } else {
                    m.selected.clear();
                    m.selected.insert(m.cursor);
                }
                break;

            case input::Key::Rune:
                if (key->rune == 'j' || key->rune == 'J') {
                    if (m.cursor < count - 1) {
                        m.cursor++;
                        size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
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
                    m.cursor = count - 1;
                    size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
                    if (count > visible) {
                        m.offset = count - visible;
                    }
                } else if ((key->rune == 'a' || key->rune == 'A') && m.limit != 1) {
                    if (m.selected.size() == count) {
                        m.selected.clear();
                    } else {
                        m.selected.clear();
                        for (size_t i = 0; i < count; i++) {
                            if (m.limit == 0 || m.selected.size() < m.limit) {
                                m.selected.insert(i);
                            }
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

    /// View function for List
    inline std::string list_view(const ListModel &m) {
        size_t count = m.item_count();
        if (count == 0) {
            return Style().foreground(m.muted_color).italic().render("(no items)");
        }

        std::string view;

        size_t visible = m.height > 0 ? static_cast<size_t>(m.height) : count;
        size_t start = m.offset;
        size_t end = std::min(start + visible, count);

        // Scroll indicator top
        if (start > 0) {
            view += Style().foreground(m.muted_color).faint().render("  ^ more") + "\n";
        }

        for (size_t i = start; i < end; i++) {
            bool is_cursor = (i == m.cursor);
            bool is_selected = m.selected.count(i) > 0;

            std::string line;

            // Cursor indicator
            if (is_cursor) {
                line += Style().foreground(m.cursor_color).bold().render(m.cursor_char + " ");
            } else {
                line += "  ";
            }

            // Selection checkbox (multi-select mode)
            if (m.limit != 1) {
                if (is_selected) {
                    line += Style().foreground(m.selected_color).render(m.selected_prefix + " ");
                } else {
                    line += Style().foreground(m.muted_color).faint().render(m.unselected_prefix + " ");
                }
            }

            // Item title
            Style title_style;
            if (is_cursor) {
                title_style.foreground(m.cursor_color).bold();
            } else if (is_selected) {
                title_style.foreground(m.selected_color);
            } else {
                title_style.foreground(m.normal_color);
            }
            line += title_style.render(m.get_title(i));

            view += line;

            // Description line (if rich items with description)
            if (m.has_description(i)) {
                view += "\n";
                // Indent description to align with title (match cursor indicator)
                std::string desc_line;
                if (is_cursor) {
                    desc_line = "    "; // Extra space when cursor is here
                } else {
                    desc_line = "   "; // Normal indent
                }
                if (m.limit != 1) {
                    desc_line += "    "; // Additional indent for checkbox
                }
                // Same color as title, but faint instead of bold
                Style desc_style;
                if (is_cursor) {
                    desc_style.foreground(m.cursor_color).faint();
                } else if (is_selected) {
                    desc_style.foreground(m.selected_color).faint();
                } else {
                    desc_style.foreground(m.normal_color).faint();
                }
                desc_line += desc_style.render(m.get_description(i));
                view += desc_line;
            }

            if (i < end - 1) {
                view += "\n";
            }
        }

        // Scroll indicator bottom
        if (end < count) {
            view += "\n";
            view += Style().foreground(m.muted_color).faint().render("  v more");
        }

        return view;
    }

    /// Builder class for List component
    class List {
      public:
        /// Set simple string items (single-line)
        List &items(const std::vector<std::string> &items) {
            m_model.items = items;
            m_model.use_rich_items = false;
            return *this;
        }

        /// Set rich items with title and description (two-line)
        List &items(const std::vector<ListItem> &items) {
            m_model.rich_items = items;
            m_model.use_rich_items = true;
            return *this;
        }

        /// Set rich items using initializer list
        List &items(std::initializer_list<ListItem> items) {
            m_model.rich_items = std::vector<ListItem>(items);
            m_model.use_rich_items = true;
            return *this;
        }

        List &selected(const std::vector<size_t> &indices) {
            m_model.selected = std::set<size_t>(indices.begin(), indices.end());
            return *this;
        }

        List &limit(size_t max) {
            m_model.limit = max;
            return *this;
        }

        List &no_limit() {
            m_model.limit = 0;
            return *this;
        }

        List &height(int h) {
            m_model.height = h;
            return *this;
        }

        List &cursor(const std::string &c) {
            m_model.cursor_char = c;
            return *this;
        }

        List &selected_prefix(const std::string &prefix) {
            m_model.selected_prefix = prefix;
            return *this;
        }

        List &unselected_prefix(const std::string &prefix) {
            m_model.unselected_prefix = prefix;
            return *this;
        }

        List &cursor_color(int r, int g, int b) {
            m_model.cursor_color = {r, g, b};
            return *this;
        }

        List &selected_color(int r, int g, int b) {
            m_model.selected_color = {r, g, b};
            return *this;
        }

        List &desc_color(int r, int g, int b) {
            m_model.desc_color = {r, g, b};
            return *this;
        }

        std::optional<std::string> run() {
            m_model.limit = 1;

            auto init = [this]() -> std::pair<ListModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](ListModel m, tea::Msg msg) { return list_update(std::move(m), msg); };
            auto view = [](const ListModel &m) { return list_view(m); };

            auto final_model = tea::Program<ListModel>(init, update, view).run();

            if (final_model.cancelled || final_model.selected.empty()) {
                return std::nullopt;
            }

            size_t idx = *final_model.selected.begin();
            return idx < final_model.item_count() ? std::optional{final_model.get_title(idx)} : std::nullopt;
        }

        std::optional<std::vector<std::string>> run_multi() {
            if (m_model.limit == 1) {
                m_model.limit = 0;
            }

            auto init = [this]() -> std::pair<ListModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](ListModel m, tea::Msg msg) { return list_update(std::move(m), msg); };
            auto view = [](const ListModel &m) { return list_view(m); };

            auto final_model = tea::Program<ListModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            std::vector<std::string> result;
            for (size_t idx : final_model.selected) {
                if (idx < final_model.item_count()) {
                    result.push_back(final_model.get_title(idx));
                }
            }

            return result;
        }

        /// Run and return full ListItem (with description) for single selection
        std::optional<ListItem> run_item() {
            m_model.limit = 1;

            auto init = [this]() -> std::pair<ListModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](ListModel m, tea::Msg msg) { return list_update(std::move(m), msg); };
            auto view = [](const ListModel &m) { return list_view(m); };

            auto final_model = tea::Program<ListModel>(init, update, view).run();

            if (final_model.cancelled || final_model.selected.empty()) {
                return std::nullopt;
            }

            size_t idx = *final_model.selected.begin();
            if (idx < final_model.item_count()) {
                if (final_model.use_rich_items) {
                    return final_model.rich_items[idx];
                }
                return ListItem{final_model.items[idx]};
            }
            return std::nullopt;
        }

        /// Run and return full ListItems (with descriptions) for multi selection
        std::optional<std::vector<ListItem>> run_items() {
            if (m_model.limit == 1) {
                m_model.limit = 0;
            }

            auto init = [this]() -> std::pair<ListModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](ListModel m, tea::Msg msg) { return list_update(std::move(m), msg); };
            auto view = [](const ListModel &m) { return list_view(m); };

            auto final_model = tea::Program<ListModel>(init, update, view).run();

            if (final_model.cancelled) {
                return std::nullopt;
            }

            std::vector<ListItem> result;
            for (size_t idx : final_model.selected) {
                if (idx < final_model.item_count()) {
                    if (final_model.use_rich_items) {
                        result.push_back(final_model.rich_items[idx]);
                    } else {
                        result.push_back(ListItem{final_model.items[idx]});
                    }
                }
            }

            return result;
        }

        ListModel model() const { return m_model; }

      private:
        ListModel m_model;
    };

} // namespace scan
