#pragma once

/// @file filepicker.hpp
/// @brief File picker component for navigating and selecting files

#include <scan/input/key.hpp>
#include <scan/style/style.hpp>
#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace scan {

    namespace fs = std::filesystem;

    enum class EntryType { Directory, File, Symlink };

    struct FileEntry {
        std::string name;
        fs::path path;
        EntryType type;
        bool is_hidden;
    };

    struct FilePickerModel {
        fs::path current_dir;
        std::vector<FileEntry> entries;
        size_t cursor = 0;
        size_t offset = 0;
        int height = 15;
        bool show_hidden = false;
        bool file_allowed = true;
        bool dir_allowed = false;
        std::set<std::string> allowed_extensions;

        // Styling - uses theme
        Color dir_color;
        Color file_color;
        Color symlink_color;
        Color cursor_color;
        Color hidden_color;
        Color path_color;
        Color muted_color;

        // State
        bool submitted = false;
        bool cancelled = false;

        FilePickerModel() {
            auto &t = current_theme();
            dir_color = t.colors.secondary;
            file_color = t.colors.text;
            symlink_color = {0, 255, 255};
            cursor_color = t.colors.primary;
            hidden_color = t.colors.text_subtle;
            path_color = t.colors.primary;
            muted_color = t.colors.text_muted;
        }
    };

    inline bool is_hidden_file(const std::string &name) { return !name.empty() && name[0] == '.'; }

    inline std::vector<FileEntry> get_entries(const fs::path &dir, bool show_hidden, bool file_allowed,
                                              bool dir_allowed, const std::set<std::string> &extensions) {
        std::vector<FileEntry> entries;

        if (dir.has_parent_path() && dir != dir.root_path()) {
            entries.push_back({"..", dir.parent_path(), EntryType::Directory, false});
        }

        try {
            std::vector<FileEntry> dirs;
            std::vector<FileEntry> files;

            for (const auto &entry : fs::directory_iterator(dir)) {
                std::string name = entry.path().filename().string();
                bool hidden = is_hidden_file(name);

                if (hidden && !show_hidden)
                    continue;

                FileEntry fe;
                fe.name = name;
                fe.path = entry.path();
                fe.is_hidden = hidden;

                if (fs::is_symlink(entry.status())) {
                    fe.type = EntryType::Symlink;
                    try {
                        if (fs::is_directory(fs::read_symlink(entry.path()))) {
                            dirs.push_back(fe);
                        } else if (file_allowed) {
                            files.push_back(fe);
                        }
                    } catch (...) {
                        if (file_allowed)
                            files.push_back(fe);
                    }
                } else if (fs::is_directory(entry.status())) {
                    fe.type = EntryType::Directory;
                    dirs.push_back(fe);
                } else if (fs::is_regular_file(entry.status())) {
                    fe.type = EntryType::File;

                    if (!extensions.empty()) {
                        std::string ext = entry.path().extension().string();
                        if (extensions.find(ext) == extensions.end())
                            continue;
                    }

                    if (file_allowed)
                        files.push_back(fe);
                }
            }

            std::sort(dirs.begin(), dirs.end(), [](const FileEntry &a, const FileEntry &b) { return a.name < b.name; });
            std::sort(files.begin(), files.end(),
                      [](const FileEntry &a, const FileEntry &b) { return a.name < b.name; });

            for (const auto &d : dirs)
                entries.push_back(d);
            for (const auto &f : files)
                entries.push_back(f);

        } catch (const fs::filesystem_error &) {
        }

        return entries;
    }

    inline void refresh_entries(FilePickerModel &m) {
        m.entries = get_entries(m.current_dir, m.show_hidden, m.file_allowed, m.dir_allowed, m.allowed_extensions);
        m.cursor = 0;
        m.offset = 0;
    }

    inline std::pair<FilePickerModel, tea::Cmd> filepicker_update(FilePickerModel m, const tea::Msg &msg) {
        if (auto *key = tea::try_as<tea::KeyMsg>(msg)) {
            switch (key->key) {
            case input::Key::Enter:
                if (!m.entries.empty()) {
                    const auto &entry = m.entries[m.cursor];
                    if (entry.type == EntryType::Directory) {
                        if (entry.name == "..") {
                            m.current_dir = entry.path;
                            refresh_entries(m);
                        } else if (m.dir_allowed) {
                            m.submitted = true;
                            return {std::move(m), tea::quit()};
                        } else {
                            m.current_dir = entry.path;
                            refresh_entries(m);
                        }
                    } else {
                        m.submitted = true;
                        return {std::move(m), tea::quit()};
                    }
                }
                break;

            case input::Key::Escape:
            case input::Key::CtrlC:
                m.cancelled = true;
                return {std::move(m), tea::quit()};

            case input::Key::Backspace:
            case input::Key::Left:
                if (m.current_dir.has_parent_path() && m.current_dir != m.current_dir.root_path()) {
                    m.current_dir = m.current_dir.parent_path();
                    refresh_entries(m);
                }
                break;

            case input::Key::Right:
                if (!m.entries.empty()) {
                    const auto &entry = m.entries[m.cursor];
                    if (entry.type == EntryType::Directory && entry.name != "..") {
                        m.current_dir = entry.path;
                        refresh_entries(m);
                    }
                }
                break;

            case input::Key::Up:
            case input::Key::CtrlP:
                if (m.cursor > 0) {
                    m.cursor--;
                    if (m.cursor < m.offset)
                        m.offset = m.cursor;
                }
                break;

            case input::Key::Down:
            case input::Key::CtrlN:
                if (!m.entries.empty() && m.cursor < m.entries.size() - 1) {
                    m.cursor++;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.cursor >= m.offset + visible)
                        m.offset = m.cursor - visible + 1;
                }
                break;

            case input::Key::Home:
                m.cursor = 0;
                m.offset = 0;
                break;

            case input::Key::End:
                if (!m.entries.empty()) {
                    m.cursor = m.entries.size() - 1;
                    size_t visible = static_cast<size_t>(m.height);
                    if (m.entries.size() > visible)
                        m.offset = m.entries.size() - visible;
                }
                break;

            case input::Key::Rune:
                if (key->rune == '.') {
                    m.show_hidden = !m.show_hidden;
                    refresh_entries(m);
                } else if (key->rune == 'j' || key->rune == 'J') {
                    if (!m.entries.empty() && m.cursor < m.entries.size() - 1) {
                        m.cursor++;
                        size_t visible = static_cast<size_t>(m.height);
                        if (m.cursor >= m.offset + visible)
                            m.offset = m.cursor - visible + 1;
                    }
                } else if (key->rune == 'k' || key->rune == 'K') {
                    if (m.cursor > 0) {
                        m.cursor--;
                        if (m.cursor < m.offset)
                            m.offset = m.cursor;
                    }
                } else if (key->rune == 'h' || key->rune == 'H') {
                    if (m.current_dir.has_parent_path() && m.current_dir != m.current_dir.root_path()) {
                        m.current_dir = m.current_dir.parent_path();
                        refresh_entries(m);
                    }
                } else if (key->rune == 'l' || key->rune == 'L') {
                    if (!m.entries.empty()) {
                        const auto &entry = m.entries[m.cursor];
                        if (entry.type == EntryType::Directory && entry.name != "..") {
                            m.current_dir = entry.path;
                            refresh_entries(m);
                        }
                    }
                } else if (key->rune == 'g') {
                    m.cursor = 0;
                    m.offset = 0;
                } else if (key->rune == 'G') {
                    if (!m.entries.empty()) {
                        m.cursor = m.entries.size() - 1;
                        size_t visible = static_cast<size_t>(m.height);
                        if (m.entries.size() > visible)
                            m.offset = m.entries.size() - visible;
                    }
                }
                break;

            default:
                break;
            }
        }

        return {std::move(m), tea::none()};
    }

    inline std::string filepicker_view(const FilePickerModel &m) {
        std::string view;

        // Current path
        view += Style().foreground(m.path_color).bold().render(m.current_dir.string());
        view += "\n";

        // Separator
        view += Style().foreground(m.muted_color).faint().render(repeat("-", 40));
        view += "\n";

        if (m.entries.empty()) {
            view += Style().foreground(m.muted_color).italic().render("  (empty directory)");
            return view;
        }

        size_t visible = static_cast<size_t>(m.height);
        size_t start = m.offset;
        size_t end = std::min(start + visible, m.entries.size());

        if (start > 0) {
            view += Style().foreground(m.muted_color).faint().render("  ^ more") + "\n";
        }

        for (size_t i = start; i < end; i++) {
            const auto &entry = m.entries[i];
            bool is_cursor = (i == m.cursor);

            std::string line;

            // Cursor
            if (is_cursor) {
                line += Style().foreground(m.cursor_color).bold().render("> ");
            } else {
                line += "  ";
            }

            // Icon
            std::string icon;
            Color color;
            switch (entry.type) {
            case EntryType::Directory:
                icon = "[D] ";
                color = m.dir_color;
                break;
            case EntryType::Symlink:
                icon = "[L] ";
                color = m.symlink_color;
                break;
            default:
                icon = "    ";
                color = m.file_color;
                break;
            }

            if (entry.is_hidden)
                color = m.hidden_color;

            Style icon_style;
            icon_style.foreground(color);
            line += icon_style.render(icon);

            // Name
            Style name_style;
            if (is_cursor) {
                name_style.foreground(m.cursor_color).bold();
            } else {
                name_style.foreground(color);
            }
            line += name_style.render(entry.name);

            if (entry.type == EntryType::Directory && entry.name != "..") {
                line += Style().foreground(m.muted_color).render("/");
            }

            view += line;
            if (i < end - 1)
                view += "\n";
        }

        if (end < m.entries.size()) {
            view += "\n";
            view += Style().foreground(m.muted_color).faint().render("  v more");
        }

        // Help
        view += "\n\n";
        view += Style()
                    .foreground(m.muted_color)
                    .italic()
                    .render("j/k or arrows, Enter select, . toggle hidden, Esc cancel");

        return view;
    }

    class FilePicker {
      public:
        FilePicker &path(const fs::path &p) {
            m_model.current_dir = fs::absolute(p);
            return *this;
        }

        FilePicker &show_hidden(bool show = true) {
            m_model.show_hidden = show;
            return *this;
        }

        FilePicker &file_allowed(bool allow = true) {
            m_model.file_allowed = allow;
            return *this;
        }

        FilePicker &dir_allowed(bool allow = true) {
            m_model.dir_allowed = allow;
            return *this;
        }

        FilePicker &extensions(const std::vector<std::string> &exts) {
            m_model.allowed_extensions = std::set<std::string>(exts.begin(), exts.end());
            return *this;
        }

        FilePicker &height(int h) {
            m_model.height = h;
            return *this;
        }

        FilePicker &dir_color(int r, int g, int b) {
            m_model.dir_color = {r, g, b};
            return *this;
        }

        FilePicker &file_color(int r, int g, int b) {
            m_model.file_color = {r, g, b};
            return *this;
        }

        std::optional<fs::path> run() {
            if (m_model.current_dir.empty()) {
                m_model.current_dir = fs::current_path();
            }
            refresh_entries(m_model);

            auto init = [this]() -> std::pair<FilePickerModel, tea::Cmd> { return {m_model, tea::none()}; };
            auto update = [](FilePickerModel m, tea::Msg msg) { return filepicker_update(std::move(m), msg); };
            auto view = [](const FilePickerModel &m) { return filepicker_view(m); };

            auto final_model = tea::Program<FilePickerModel>(init, update, view).run();

            if (final_model.cancelled || final_model.entries.empty()) {
                return std::nullopt;
            }

            return final_model.entries[final_model.cursor].path;
        }

        FilePickerModel model() const { return m_model; }

      private:
        FilePickerModel m_model;
    };

} // namespace scan
