#pragma once

/// @file argu/completion/generator.hpp
/// @brief Shell completion script generation

#include <argu/core/types.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace argu {

    // Forward declarations
    class Command;
    class Arg;

    /// Shell completion generator
    class CompletionGenerator {
      public:
        explicit CompletionGenerator(const Command &cmd) : m_cmd(cmd) {}

        /// Generate completion script for the specified shell
        std::string generate(Shell shell) const {
            switch (shell) {
            case Shell::Bash:
                return generate_bash();
            case Shell::Zsh:
                return generate_zsh();
            case Shell::Fish:
                return generate_fish();
            case Shell::PowerShell:
                return generate_powershell();
            case Shell::Elvish:
                return generate_elvish();
            default:
                return "";
            }
        }

        /// Get shell from string name
        static std::optional<Shell> parse_shell(const std::string &name) {
            std::string lower = name;
            for (auto &c : lower)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

            if (lower == "bash")
                return Shell::Bash;
            if (lower == "zsh")
                return Shell::Zsh;
            if (lower == "fish")
                return Shell::Fish;
            if (lower == "powershell" || lower == "pwsh")
                return Shell::PowerShell;
            if (lower == "elvish")
                return Shell::Elvish;
            return std::nullopt;
        }

      private:
        const Command &m_cmd;

        std::string generate_bash() const;
        std::string generate_zsh() const;
        std::string generate_fish() const;
        std::string generate_powershell() const;
        std::string generate_elvish() const;

        // Helper to escape strings for shell scripts
        static std::string escape_bash(const std::string &s) {
            std::string result;
            for (char c : s) {
                if (c == '\'' || c == '\\' || c == '"' || c == '$' || c == '`') {
                    result += '\\';
                }
                result += c;
            }
            return result;
        }

        static std::string escape_fish(const std::string &s) {
            std::string result;
            for (char c : s) {
                if (c == '\'' || c == '\\') {
                    result += '\\';
                }
                result += c;
            }
            return result;
        }
    };

    // Implementation in separate header to avoid circular dependencies
    // These will be implemented after Command is fully defined

} // namespace argu
