#pragma once

/// @file argu/argu.hpp
/// @brief Main include file for the argu argument parsing library
///
/// argu - A modern, feature-complete C++20 argument parsing library
/// Inspired by CLAP (Rust) and CLI11 (C++)
///
/// Features:
/// - CLAP-style builder API
/// - Deep nested subcommand chains (git remote add style)
/// - Automatic help generation with color support
/// - Shell completion generation (Bash, Zsh, Fish, PowerShell, Elvish)
/// - Typo suggestions using Levenshtein distance
/// - Argument groups (mutually exclusive, required together)
/// - Config file support (INI/TOML/JSON)
/// - Environment variable support
/// - Rich validators and transformers
/// - Direct variable binding
///
/// Example:
/// @code
/// #include <argu/argu.hpp>
///
/// int main(int argc, char* argv[]) {
///     bool verbose = false;
///     std::string output;
///     std::vector<std::string> files;
///
///     auto cmd = argu::Command("myapp")
///         .version("1.0.0")
///         .about("My awesome application")
///         .arg(argu::Arg("verbose")
///             .short_name('v')
///             .long_name("verbose")
///             .help("Enable verbose output")
///             .flag(verbose))
///         .arg(argu::Arg("output")
///             .short_name('o')
///             .long_name("output")
///             .help("Output file path")
///             .value_name("FILE")
///             .value_of(output)
///             .validate(argu::validators::parent_exists()))
///         .arg(argu::Arg("files")
///             .help("Input files to process")
///             .positional()
///             .takes_one_or_more()
///             .required()
///             .value_of(files));
///
///     auto result = cmd.parse(argc, argv);
///     if (!result) return result.exit();
///
///     // Use verbose, output, files...
///     return 0;
/// }
/// @endcode

// Core types and utilities
#include <argu/core/arg.hpp>
#include <argu/core/command.hpp>
#include <argu/core/error.hpp>
#include <argu/core/group.hpp>
#include <argu/core/levenshtein.hpp>
#include <argu/core/types.hpp>
#include <argu/core/validators.hpp>

// Style and formatting
#include <argu/style/colors.hpp>
#include <argu/style/help_formatter.hpp>

// Completion generation
#include <argu/completion/completions.hpp>
#include <argu/completion/generator.hpp>

// Config file support
#include <argu/config/config_parser.hpp>

// Human-friendly parsers
#include <argu/parsers/duration.hpp>
#include <argu/parsers/size.hpp>

// Parser (must be last as it implements Command methods)
#include <argu/core/parser.hpp>

namespace argu {

    /// Library version
    constexpr const char *VERSION = "2.0.0";

    /// Library version components
    constexpr int VERSION_MAJOR = 2;
    constexpr int VERSION_MINOR = 0;
    constexpr int VERSION_PATCH = 0;

} // namespace argu
