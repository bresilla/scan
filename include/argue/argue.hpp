#pragma once

/// @file argue.hpp
/// @brief Main include file for the Argue argument parsing library
///
/// Argue is a modern C++20 header-only command-line argument parsing library
/// inspired by Rust's CLAP and CLI11, designed to work seamlessly with Scan.
///
/// Usage:
///   #include <argue/argue.hpp>
///
///   int main(int argc, char* argv[]) {
///       std::string name;
///       int count = 1;
///       bool verbose = false;
///
///       auto cmd = argue::Command("myapp")
///           .version("1.0.0")
///           .about("My awesome application")
///           .arg(argue::Arg("name")
///               .help("Your name")
///               .required())
///           .arg(argue::Arg("count")
///               .short_name('c')
///               .long_name("count")
///               .help("Number of times")
///               .value_of(count)
///               .default_value("1"))
///           .arg(argue::Arg("verbose")
///               .short_name('v')
///               .long_name("verbose")
///               .help("Enable verbose output")
///               .flag(verbose));
///
///       auto result = cmd.parse(argc, argv);
///       if (!result) {
///           return result.exit_code();
///       }
///
///       // Use name, count, verbose...
///   }

#include <argue/arg.hpp>
#include <argue/command.hpp>
#include <argue/error.hpp>
#include <argue/help.hpp>
#include <argue/parser.hpp>
#include <argue/validators.hpp>

namespace argue {

    /// Library version
    inline constexpr const char *version = "0.1.0";

} // namespace argue
