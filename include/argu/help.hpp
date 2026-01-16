#pragma once

/// @file help.hpp
/// @brief Help text formatting and generation

#include <argu/arg.hpp>
#include <argu/command.hpp>

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
        bool color_enabled = true;
    };

    /// ANSI color codes for help formatting
    namespace colors {
        inline const char *reset = "\033[0m";
        inline const char *bold = "\033[1m";
        inline const char *dim = "\033[2m";
        inline const char *yellow = "\033[33m";
        inline const char *green = "\033[32m";
        inline const char *cyan = "\033[36m";
    } // namespace colors

    /// Help text generator
    class HelpFormatter {
      public:
        explicit HelpFormatter(const Command &cmd, HelpConfig config = {}) : m_cmd(cmd), m_config(config) {}

        /// Generate complete help text
        std::string format() const {
            std::ostringstream oss;

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

            // Footer
            format_footer(oss);

            return oss.str();
        }

      private:
        const Command &m_cmd;
        HelpConfig m_config;

        void format_header(std::ostringstream &oss) const {
            if (m_config.color_enabled) {
                oss << colors::bold;
            }
            oss << m_cmd.name();
            if (m_cmd.get_version()) {
                oss << " " << *m_cmd.get_version();
            }
            if (m_config.color_enabled) {
                oss << colors::reset;
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
            if (m_config.color_enabled) {
                oss << colors::yellow << "Usage:" << colors::reset;
            } else {
                oss << "Usage:";
            }
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
                    if (arg.is_required()) {
                        oss << "<" << arg.value_name_str() << ">";
                    } else {
                        oss << "[" << arg.value_name_str() << "]";
                    }
                    if (arg.value_count().max > 1 || arg.value_count().max == SIZE_MAX) {
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

            // Format positional arguments first
            if (!positionals.empty()) {
                if (m_config.color_enabled) {
                    oss << colors::yellow << "Arguments:" << colors::reset << "\n";
                } else {
                    oss << "Arguments:\n";
                }

                for (const auto *arg : positionals) {
                    format_arg(oss, *arg, true);
                }
                oss << "\n";
            }

            // Format option groups
            for (const auto &[group_name, args] : groups) {
                if (args.empty())
                    continue;

                if (m_config.color_enabled) {
                    oss << colors::yellow << group_name << ":" << colors::reset << "\n";
                } else {
                    oss << group_name << ":\n";
                }

                for (const auto *arg : args) {
                    format_arg(oss, *arg, false);
                }
                oss << "\n";
            }
        }

        void format_arg(std::ostringstream &oss, const Arg &arg, bool is_positional) const {
            std::string indent(m_config.indent, ' ');
            oss << indent;

            std::ostringstream name_oss;
            if (is_positional) {
                if (m_config.color_enabled) {
                    name_oss << colors::green << "<" << arg.name() << ">" << colors::reset;
                } else {
                    name_oss << "<" << arg.name() << ">";
                }
            } else {
                // Short option
                if (arg.short_opt()) {
                    if (m_config.color_enabled) {
                        name_oss << colors::green << "-" << *arg.short_opt() << colors::reset;
                    } else {
                        name_oss << "-" << *arg.short_opt();
                    }
                }

                // Long option
                if (arg.long_opt()) {
                    if (arg.short_opt()) {
                        name_oss << ", ";
                    }
                    if (m_config.color_enabled) {
                        name_oss << colors::green << "--" << *arg.long_opt() << colors::reset;
                    } else {
                        name_oss << "--" << *arg.long_opt();
                    }
                }

                // Value placeholder (only if arg takes values)
                if (!arg.is_flag() && !arg.is_count() && arg.value_count().min > 0) {
                    name_oss << " <" << arg.value_name_str() << ">";
                }
            }

            // Calculate actual display width (without ANSI codes)
            std::string name_str = name_oss.str();
            std::size_t display_len = name_str.length();
            if (m_config.color_enabled) {
                // Remove ANSI code lengths from calculation
                std::string clean = name_str;
                std::size_t pos;
                while ((pos = clean.find("\033[")) != std::string::npos) {
                    std::size_t end = clean.find('m', pos);
                    if (end != std::string::npos) {
                        clean.erase(pos, end - pos + 1);
                    }
                }
                display_len = clean.length();
            }

            oss << name_str;

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

            // Show default
            if (m_config.show_defaults && arg.default_val()) {
                if (m_config.color_enabled) {
                    oss << " " << colors::dim << "[default: " << *arg.default_val() << "]" << colors::reset;
                } else {
                    oss << " [default: " << *arg.default_val() << "]";
                }
            }

            // Show env var
            if (m_config.show_env_vars && arg.get_env()) {
                if (m_config.color_enabled) {
                    oss << " " << colors::dim << "[env: " << *arg.get_env() << "]" << colors::reset;
                } else {
                    oss << " [env: " << *arg.get_env() << "]";
                }
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

            if (m_config.color_enabled) {
                oss << colors::yellow << "Commands:" << colors::reset << "\n";
            } else {
                oss << "Commands:\n";
            }

            for (const auto *sub : visible_commands) {
                std::string indent(m_config.indent, ' ');
                oss << indent;

                if (m_config.color_enabled) {
                    oss << colors::green << sub->name() << colors::reset;
                } else {
                    oss << sub->name();
                }

                std::size_t name_len = sub->name().length();
                std::size_t total_name_space = m_config.description_indent - m_config.indent;

                if (name_len < total_name_space) {
                    oss << std::string(total_name_space - name_len, ' ');
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

        void format_footer(std::ostringstream &oss) const {
            if (!m_cmd.get_subcommands().empty()) {
                if (m_config.color_enabled) {
                    oss << colors::dim;
                }
                oss << "See '" << m_cmd.name() << " <command> --help' for more information on a specific command.";
                if (m_config.color_enabled) {
                    oss << colors::reset;
                }
                oss << "\n";
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
