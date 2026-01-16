#pragma once

/// @file argu/core/error.hpp
/// @brief Error types for argument parsing

#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace argu {

    /// Exit codes for argument parsing
    enum class ExitCode : int {
        Success = 0,
        UsageError = 1,
        DataError = 65,
        ConfigError = 78,
    };

    /// Base class for all argu errors
    class Error : public std::exception {
      public:
        explicit Error(std::string message, ExitCode code = ExitCode::UsageError)
            : m_message(std::move(message)), m_code(code) {}

        const char *what() const noexcept override { return m_message.c_str(); }

        ExitCode exit_code() const noexcept { return m_code; }
        int exit_code_int() const noexcept { return static_cast<int>(m_code); }

        const std::string &message() const noexcept { return m_message; }

      protected:
        std::string m_message;
        ExitCode m_code;
    };

    /// Error: Required argument is missing
    class MissingRequiredError : public Error {
      public:
        explicit MissingRequiredError(const std::string &arg_name)
            : Error("Missing required argument: " + arg_name), m_arg_name(arg_name) {}

        const std::string &arg_name() const noexcept { return m_arg_name; }

      private:
        std::string m_arg_name;
    };

    /// Error: Invalid value for argument
    class InvalidValueError : public Error {
      public:
        InvalidValueError(const std::string &arg_name, const std::string &value, const std::string &reason = "")
            : Error("Invalid value '" + value + "' for argument '" + arg_name + "'" +
                    (reason.empty() ? "" : ": " + reason)),
              m_arg_name(arg_name), m_value(value) {}

        const std::string &arg_name() const noexcept { return m_arg_name; }
        const std::string &value() const noexcept { return m_value; }

      private:
        std::string m_arg_name;
        std::string m_value;
    };

    /// Error: Unknown argument provided (with optional suggestions)
    class UnknownArgumentError : public Error {
      public:
        explicit UnknownArgumentError(const std::string &arg, const std::vector<std::string> &suggestions = {})
            : Error(format_message(arg, suggestions)), m_arg(arg), m_suggestions(suggestions) {}

        const std::string &arg() const noexcept { return m_arg; }
        const std::vector<std::string> &suggestions() const noexcept { return m_suggestions; }

      private:
        std::string m_arg;
        std::vector<std::string> m_suggestions;

        static std::string format_message(const std::string &arg, const std::vector<std::string> &suggestions) {
            std::string msg = "Unknown argument: " + arg;
            if (!suggestions.empty()) {
                msg += "\n\nDid you mean";
                if (suggestions.size() == 1) {
                    msg += " '" + suggestions[0] + "'?";
                } else {
                    msg += " one of these?\n";
                    for (const auto &s : suggestions) {
                        msg += "    " + s + "\n";
                    }
                }
            }
            return msg;
        }
    };

    /// Error: Too many positional arguments
    class TooManyArgumentsError : public Error {
      public:
        explicit TooManyArgumentsError(const std::string &extra_arg)
            : Error("Unexpected positional argument: " + extra_arg), m_extra_arg(extra_arg) {}

        const std::string &extra_arg() const noexcept { return m_extra_arg; }

      private:
        std::string m_extra_arg;
    };

    /// Error: Argument requires a value but none was provided
    class MissingValueError : public Error {
      public:
        explicit MissingValueError(const std::string &arg_name)
            : Error("Argument '" + arg_name + "' requires a value"), m_arg_name(arg_name) {}

        const std::string &arg_name() const noexcept { return m_arg_name; }

      private:
        std::string m_arg_name;
    };

    /// Error: Subcommand required but not provided
    class MissingSubcommandError : public Error {
      public:
        explicit MissingSubcommandError(const std::vector<std::string> &available = {})
            : Error(format_message(available)), m_available(available) {}

        const std::vector<std::string> &available() const noexcept { return m_available; }

      private:
        std::vector<std::string> m_available;

        static std::string format_message(const std::vector<std::string> &available) {
            std::string msg = "A subcommand is required";
            if (!available.empty()) {
                msg += ". Available subcommands:\n";
                for (const auto &cmd : available) {
                    msg += "    " + cmd + "\n";
                }
            }
            return msg;
        }
    };

    /// Error: Unknown subcommand (with optional suggestions)
    class UnknownSubcommandError : public Error {
      public:
        explicit UnknownSubcommandError(const std::string &name, const std::vector<std::string> &suggestions = {})
            : Error(format_message(name, suggestions)), m_name(name), m_suggestions(suggestions) {}

        const std::string &name() const noexcept { return m_name; }
        const std::vector<std::string> &suggestions() const noexcept { return m_suggestions; }

      private:
        std::string m_name;
        std::vector<std::string> m_suggestions;

        static std::string format_message(const std::string &name, const std::vector<std::string> &suggestions) {
            std::string msg = "Unknown subcommand: " + name;
            if (!suggestions.empty()) {
                msg += "\n\nDid you mean";
                if (suggestions.size() == 1) {
                    msg += " '" + suggestions[0] + "'?";
                } else {
                    msg += " one of these?\n";
                    for (const auto &s : suggestions) {
                        msg += "    " + s + "\n";
                    }
                }
            }
            return msg;
        }
    };

    /// Error: Conflicting arguments
    class ConflictError : public Error {
      public:
        ConflictError(const std::string &arg1, const std::string &arg2)
            : Error("Arguments '" + arg1 + "' and '" + arg2 + "' cannot be used together"), m_arg1(arg1), m_arg2(arg2) {
        }

        const std::string &arg1() const noexcept { return m_arg1; }
        const std::string &arg2() const noexcept { return m_arg2; }

      private:
        std::string m_arg1;
        std::string m_arg2;
    };

    /// Error: Argument group constraint violated
    class GroupConstraintError : public Error {
      public:
        GroupConstraintError(const std::string &group_name, const std::string &reason)
            : Error("Argument group '" + group_name + "': " + reason), m_group_name(group_name) {}

        const std::string &group_name() const noexcept { return m_group_name; }

      private:
        std::string m_group_name;
    };

    /// Error: Mutex group violation (multiple args from same group used)
    class MutexGroupError : public Error {
      public:
        MutexGroupError(const std::string &group_name, const std::vector<std::string> &used_args)
            : Error(format_message(group_name, used_args)), m_group_name(group_name), m_used_args(used_args) {}

        const std::string &group_name() const noexcept { return m_group_name; }
        const std::vector<std::string> &used_args() const noexcept { return m_used_args; }

      private:
        std::string m_group_name;
        std::vector<std::string> m_used_args;

        static std::string format_message(const std::string &group_name, const std::vector<std::string> &used_args) {
            std::string msg = "The following arguments cannot be used together (mutually exclusive group '" + group_name + "'):\n";
            for (const auto &arg : used_args) {
                msg += "    --" + arg + "\n";
            }
            msg += "\nPlease use only one of these options.";
            return msg;
        }
    };

    /// Error: Required together group - some but not all args provided
    class RequiredTogetherError : public Error {
      public:
        RequiredTogetherError(const std::string &group_name, const std::vector<std::string> &provided,
                              const std::vector<std::string> &missing)
            : Error(format_message(group_name, provided, missing)), m_group_name(group_name), m_provided(provided),
              m_missing(missing) {}

        const std::string &group_name() const noexcept { return m_group_name; }
        const std::vector<std::string> &provided() const noexcept { return m_provided; }
        const std::vector<std::string> &missing() const noexcept { return m_missing; }

      private:
        std::string m_group_name;
        std::vector<std::string> m_provided;
        std::vector<std::string> m_missing;

        static std::string format_message(const std::string &group_name, const std::vector<std::string> &provided,
                                          const std::vector<std::string> &missing) {
            std::string msg = "Arguments in group '" + group_name + "' must be used together.\n";
            msg += "You provided:\n";
            for (const auto &arg : provided) {
                msg += "    --" + arg + "\n";
            }
            msg += "But you are missing:\n";
            for (const auto &arg : missing) {
                msg += "    --" + arg + "\n";
            }
            return msg;
        }
    };

    /// Error: At least one required from group
    class AtLeastOneRequiredError : public Error {
      public:
        AtLeastOneRequiredError(const std::string &group_name, const std::vector<std::string> &options)
            : Error(format_message(group_name, options)), m_group_name(group_name), m_options(options) {}

        const std::string &group_name() const noexcept { return m_group_name; }
        const std::vector<std::string> &options() const noexcept { return m_options; }

      private:
        std::string m_group_name;
        std::vector<std::string> m_options;

        static std::string format_message(const std::string &group_name, const std::vector<std::string> &options) {
            std::string msg = "At least one of the following arguments is required (group '" + group_name + "'):\n";
            for (const auto &opt : options) {
                msg += "    --" + opt + "\n";
            }
            return msg;
        }
    };

    /// Error: Validation failed
    class ValidationError : public Error {
      public:
        ValidationError(const std::string &arg_name, const std::string &reason)
            : Error("Validation failed for '" + arg_name + "': " + reason), m_arg_name(arg_name), m_reason(reason) {}

        const std::string &arg_name() const noexcept { return m_arg_name; }
        const std::string &reason() const noexcept { return m_reason; }

      private:
        std::string m_arg_name;
        std::string m_reason;
    };

    /// Error: Dependency not satisfied
    class DependencyError : public Error {
      public:
        DependencyError(const std::string &arg, const std::string &required)
            : Error("Argument '" + arg + "' requires '" + required + "' to be specified"), m_arg(arg),
              m_required(required) {}

        const std::string &arg() const noexcept { return m_arg; }
        const std::string &required() const noexcept { return m_required; }

      private:
        std::string m_arg;
        std::string m_required;
    };

    /// Error: Config file error
    class ConfigFileError : public Error {
      public:
        ConfigFileError(const std::string &file, const std::string &reason)
            : Error("Error in config file '" + file + "': " + reason, ExitCode::ConfigError), m_file(file),
              m_reason(reason) {}

        const std::string &file() const noexcept { return m_file; }
        const std::string &reason() const noexcept { return m_reason; }

      private:
        std::string m_file;
        std::string m_reason;
    };

    /// Special "error" for help/version display (exit code 0)
    class HelpRequested : public Error {
      public:
        explicit HelpRequested(std::string help_text) : Error(std::move(help_text), ExitCode::Success) {}
    };

    /// Special "error" for version display (exit code 0)
    class VersionRequested : public Error {
      public:
        explicit VersionRequested(std::string version_text) : Error(std::move(version_text), ExitCode::Success) {}
    };

    /// Special "error" for completion generation (exit code 0)
    class CompletionRequested : public Error {
      public:
        explicit CompletionRequested(std::string completion_script)
            : Error(std::move(completion_script), ExitCode::Success) {}
    };

} // namespace argu
