#pragma once

/// @file argu/style/help_formatter.hpp
/// @brief Help text formatting and generation with color support

#include <argu/core/arg.hpp>
#include <argu/core/command.hpp>
#include <argu/core/group.hpp>
#include <argu/style/colors.hpp>

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace argu {

    /// Help formatter configuration
    struct HelpConfig {
        std::size_t max_width = 80;
        std::size_t indent = 2;
        std::size_t description_indent = 24;
        bool show_env_vars = true;
        bool show_defaults = true;
        bool show_choices = true;
        bool show_aliases = true;
        bool use_colors = true;
    };

    /// Help text generator with color support
    class HelpFormatter {
      public:
        explicit HelpFormatter(const Command &cmd, HelpConfig config = {})
            : m_cmd(cmd), m_config(config), m_theme(cmd.get_theme()) {
            // Determine if colors should be used
            switch (cmd.get_color_mode()) {
            case ColorMode::Always:
                m_colors_enabled = true;
                break;
            case ColorMode::Never:
                m_colors_enabled = false;
                break;
            case ColorMode::Auto:
            default:
                m_colors_enabled = m_config.use_colors && ansi::supports_color();
                break;
            }
        }

        /// Generate complete help text
        std::string format() const {
            std::ostringstream oss;

            // Before help text
            if (m_cmd.get_before_help()) {
                oss << *m_cmd.get_before_help() << "\n\n";
            }

            // Name and version
            format_header(oss);

            // Description
            format_description(oss);

            // Usage
            format_usage(oss);

            // Arguments by group
            format_arguments(oss);

            // Subcommands
            format_subcommands(oss);

            // Custom sections (sorted by order)
            format_custom_sections(oss);

            // After help text
            if (m_cmd.get_after_help()) {
                oss << *m_cmd.get_after_help() << "\n";
            }

            // Footer
            format_footer(oss);

            return oss.str();
        }

      private:
        const Command &m_cmd;
        HelpConfig m_config;
        HelpTheme m_theme;
        bool m_colors_enabled;

        std::string colorize(const std::string &text, const Color &color, bool bold = false) const {
            if (!m_colors_enabled)
                return text;
            return StyledText(text).fg(color).bold(bold).render(true);
        }

        std::string dim(const std::string &text) const {
            if (!m_colors_enabled)
                return text;
            return StyledText(text).dim(true).render(true);
        }

        void format_header(std::ostringstream &oss) const {
            oss << colorize(m_cmd.name(), m_theme.command_name, m_theme.use_bold_headers);
            if (m_cmd.get_version()) {
                oss << " " << *m_cmd.get_version();
            }
            oss << "\n";
        }

        void format_description(std::ostringstream &oss) const {
            if (m_cmd.get_about()) {
                oss << *m_cmd.get_about() << "\n";
            }
            oss << "\n";
        }

        void format_usage(std::ostringstream &oss) const {
            oss << colorize("Usage:", m_theme.section_header, m_theme.use_bold_headers);
            oss << " " << m_cmd.name();

            // Add options indicator
            bool has_options = false;
            for (const auto &arg : m_cmd.get_args()) {
                if (!arg.is_positional() && !arg.is_hidden()) {
                    has_options = true;
                    break;
                }
            }
            if (has_options) {
                oss << " [OPTIONS]";
            }

            // Add subcommands indicator
            if (!m_cmd.get_subcommands().empty()) {
                if (m_cmd.is_subcommand_required()) {
                    oss << " <COMMAND>";
                } else {
                    oss << " [COMMAND]";
                }
            }

            // Add positional arguments
            for (const auto &arg : m_cmd.get_args()) {
                if (arg.is_positional() && !arg.is_hidden()) {
                    oss << " ";
                    std::string name = arg.value_name_str().empty() ? arg.name() : arg.value_name_str();
                    if (arg.is_required()) {
                        oss << "<" << name << ">";
                    } else {
                        oss << "[" << name << "]";
                    }
                    if (arg.value_count().max > 1 || arg.value_count().max == ValueCount::unlimited) {
                        oss << "...";
                    }
                }
            }

            oss << "\n\n";
        }

        void format_arguments(std::ostringstream &oss) const {
            // Group arguments
            std::map<std::string, std::vector<const Arg *>> groups;
            std::vector<const Arg *> positionals;

            for (const auto &arg : m_cmd.get_args()) {
                if (arg.is_hidden())
                    continue;

                if (arg.is_positional()) {
                    positionals.push_back(&arg);
                } else {
                    groups[arg.get_group()].push_back(&arg);
                }
            }

            // Sort arguments within each group by display_order
            for (auto &[group_name, args] : groups) {
                std::stable_sort(args.begin(), args.end(), [](const Arg *a, const Arg *b) {
                    return a->get_display_order() < b->get_display_order();
                });
            }

            // Format positional arguments first
            if (!positionals.empty()) {
                // Sort positionals by display order
                std::stable_sort(positionals.begin(), positionals.end(), [](const Arg *a, const Arg *b) {
                    return a->get_display_order() < b->get_display_order();
                });

                oss << colorize("Arguments:", m_theme.section_header, m_theme.use_bold_headers) << "\n";

                for (const auto *arg : positionals) {
                    format_arg(oss, *arg, true);
                }
                oss << "\n";
            }

            // Sort groups by command's group order, then alphabetically
            std::vector<std::pair<std::string, std::vector<const Arg *>>> sorted_groups(groups.begin(), groups.end());
            std::stable_sort(sorted_groups.begin(), sorted_groups.end(), [this](const auto &a, const auto &b) {
                int order_a = m_cmd.get_group_order(a.first);
                int order_b = m_cmd.get_group_order(b.first);
                if (order_a != order_b)
                    return order_a < order_b;
                return a.first < b.first;
            });

            // Format option groups in sorted order
            for (const auto &[group_name, args] : sorted_groups) {
                if (args.empty())
                    continue;

                oss << colorize(group_name + ":", m_theme.section_header, m_theme.use_bold_headers) << "\n";

                for (const auto *arg : args) {
                    format_arg(oss, *arg, false);
                }
                oss << "\n";
            }

            // Format argument groups with constraints
            for (const auto &group : m_cmd.get_groups()) {
                if (group.is_hidden())
                    continue;

                std::string group_label = group.get_name();
                if (group.get_type() == GroupType::MutuallyExclusive) {
                    group_label += " (mutually exclusive)";
                } else if (group.get_type() == GroupType::RequiredTogether) {
                    group_label += " (all required together)";
                } else if (group.get_type() == GroupType::AtLeastOne) {
                    group_label += " (at least one required)";
                }

                oss << colorize(group_label + ":", m_theme.section_header, m_theme.use_bold_headers) << "\n";

                for (const auto &arg_name : group.get_args()) {
                    if (auto *arg = m_cmd.find_arg_by_name(arg_name)) {
                        format_arg(oss, *arg, arg->is_positional());
                    }
                }
                oss << "\n";
            }
        }

        void format_arg(std::ostringstream &oss, const Arg &arg, bool is_positional) const {
            std::string indent_str(m_config.indent, ' ');
            oss << indent_str;

            std::ostringstream name_oss;
            std::size_t display_len = 0;

            if (is_positional) {
                std::string name = "<" + arg.name() + ">";
                name_oss << colorize(name, m_theme.positional, m_theme.use_bold_options);
                display_len = name.size();
            } else {
                // Short option
                if (arg.short_opt()) {
                    std::string short_name = "-" + std::string(1, *arg.short_opt());
                    name_oss << colorize(short_name, m_theme.option_short, m_theme.use_bold_options);
                    display_len += short_name.size();
                }

                // Long option
                if (arg.long_opt()) {
                    if (arg.short_opt()) {
                        name_oss << ", ";
                        display_len += 2;
                    }
                    std::string long_name = "--" + *arg.long_opt();
                    name_oss << colorize(long_name, m_theme.option_name, m_theme.use_bold_options);
                    display_len += long_name.size();
                }

                // Value placeholder
                if (!arg.is_flag() && !arg.is_count() && arg.value_count().min > 0) {
                    std::string val_name = " <" + arg.value_name_str() + ">";
                    name_oss << val_name;
                    display_len += val_name.size();
                }
            }

            oss << name_oss.str();

            // Add padding
            std::size_t total_name_space = m_config.description_indent - m_config.indent;
            if (display_len < total_name_space) {
                oss << std::string(total_name_space - display_len, ' ');
            } else {
                oss << "\n" << std::string(m_config.description_indent, ' ');
            }

            // Description
            oss << arg.help_text();

            // Show choices
            if (m_config.show_choices) {
                const auto &choices = arg.get_choices();
                if (!choices.empty()) {
                    oss << " [possible values: ";
                    for (std::size_t i = 0; i < choices.size(); ++i) {
                        if (i > 0)
                            oss << ", ";
                        oss << choices[i];
                    }
                    oss << "]";
                }
            }

            // Show default
            if (m_config.show_defaults && arg.default_val()) {
                std::string def_text = " [default: " + *arg.default_val() + "]";
                oss << (m_theme.use_dim_defaults ? dim(def_text) : def_text);
            }

            // Show env var
            if (m_config.show_env_vars && arg.get_env()) {
                std::string env_text = " [env: " + *arg.get_env() + "]";
                oss << (m_theme.use_dim_defaults ? dim(env_text) : env_text);
            }

            oss << "\n";
        }

        void format_subcommands(std::ostringstream &oss) const {
            std::vector<const Command *> visible_commands;
            for (const auto &sub : m_cmd.get_subcommands()) {
                if (!sub->is_hidden()) {
                    visible_commands.push_back(sub.get());
                }
            }

            if (visible_commands.empty())
                return;

            oss << colorize("Commands:", m_theme.section_header, m_theme.use_bold_headers) << "\n";

            for (const auto *sub : visible_commands) {
                std::string indent_str(m_config.indent, ' ');
                oss << indent_str;

                std::string name = sub->name();
                oss << colorize(name, m_theme.subcommand, m_theme.use_bold_options);

                // Show aliases
                if (m_config.show_aliases && !sub->get_visible_aliases().empty()) {
                    oss << " (";
                    for (std::size_t i = 0; i < sub->get_visible_aliases().size(); ++i) {
                        if (i > 0)
                            oss << ", ";
                        oss << sub->get_visible_aliases()[i];
                    }
                    oss << ")";
                }

                std::size_t display_len = name.size();
                std::size_t total_name_space = m_config.description_indent - m_config.indent;

                if (display_len < total_name_space) {
                    oss << std::string(total_name_space - display_len, ' ');
                } else {
                    oss << "\n" << std::string(m_config.description_indent, ' ');
                }

                if (sub->get_about()) {
                    oss << *sub->get_about();
                }

                oss << "\n";
            }
            oss << "\n";
        }

        void format_custom_sections(std::ostringstream &oss) const {
            auto sections = m_cmd.get_custom_sections();
            if (sections.empty())
                return;

            // Sort by order
            std::stable_sort(
                sections.begin(), sections.end(),
                [](const Command::HelpSection &a, const Command::HelpSection &b) { return a.order < b.order; });

            for (const auto &section : sections) {
                oss << colorize(section.title + ":", m_theme.section_header, m_theme.use_bold_headers) << "\n";
                // Indent content
                std::istringstream content_stream(section.content);
                std::string line;
                while (std::getline(content_stream, line)) {
                    oss << std::string(m_config.indent, ' ') << line << "\n";
                }
                oss << "\n";
            }
        }

        void format_footer(std::ostringstream &oss) const {
            if (!m_cmd.get_subcommands().empty()) {
                std::string footer =
                    "See '" + m_cmd.name() + " <command> --help' for more information on a specific command.";
                oss << dim(footer) << "\n";
            }
        }
    };

    // Implementation of Command::help
    inline std::string Command::help() const {
        HelpFormatter formatter(*this);
        return formatter.format();
    }

    // Implementation of Command::version_string
    inline std::string Command::version_string() const {
        std::ostringstream oss;
        oss << m_name;
        if (m_version) {
            oss << " " << *m_version;
        }
        return oss.str();
    }

} // namespace argu
