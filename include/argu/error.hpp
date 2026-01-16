#pragma once

/// @file error.hpp
/// @brief Error types for argument parsing

#include <exception>
#include <string>
#include <utility>

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

    /// Error: Unknown argument provided
    class UnknownArgumentError : public Error {
      public:
        explicit UnknownArgumentError(const std::string &arg) : Error("Unknown argument: " + arg), m_arg(arg) {}

        const std::string &arg() const noexcept { return m_arg; }

      private:
        std::string m_arg;
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
        MissingSubcommandError() : Error("A subcommand is required") {}
    };

    /// Error: Unknown subcommand
    class UnknownSubcommandError : public Error {
      public:
        explicit UnknownSubcommandError(const std::string &name) : Error("Unknown subcommand: " + name), m_name(name) {}

        const std::string &name() const noexcept { return m_name; }

      private:
        std::string m_name;
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

} // namespace argu
