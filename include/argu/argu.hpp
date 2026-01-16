#pragma once

/// @file argu.hpp
/// @brief Main include file for the Argu argument parsing library
///
/// Argu is a modern C++20 header-only command-line argument parsing library
/// inspired by Rust's CLAP and CLI11, designed to work seamlessly with Scan.
///
/// Usage:
///   #include <argu/argu.hpp>
///
///   int main(int argc, char* argv[]) {
///       std::string name;
///       int count = 1;
///       bool verbose = false;
///
///       auto cmd = argu::Command("myapp")
///           .version("1.0.0")
///           .about("My awesome application")
///           .arg(argu::Arg("name")
///               .help("Your name")
///               .required())
///           .arg(argu::Arg("count")
///               .short_name('c')
///               .long_name("count")
///               .help("Number of times")
///               .value_of(count)
///               .default_value("1"))
///           .arg(argu::Arg("verbose")
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

#include <argu/arg.hpp>
#include <argu/command.hpp>
#include <argu/error.hpp>
#include <argu/help.hpp>
#include <argu/parser.hpp>
#include <argu/validators.hpp>

namespace argu {

    /// Library version
    inline constexpr const char *version = "0.1.0";

} // namespace argu
