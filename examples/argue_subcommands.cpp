/// @file argue_subcommands.cpp
/// @brief Demonstrates subcommand support in Argue (similar to git, cargo, etc.)

#include <argue/argue.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
    // Variables for 'add' subcommand
    std::string add_name;
    bool add_force = false;

    // Variables for 'remove' subcommand
    std::string remove_name;
    bool remove_recursive = false;

    // Variables for 'list' subcommand
    bool list_all = false;
    std::string list_format = "table";

    // Build 'add' subcommand
    auto add_cmd = argue::Command("add")
                       .about("Add a new item")
                       .arg(argue::Arg("name")
                                .positional()
                                .required()
                                .help("Name of the item to add")
                                .value_of(add_name))
                       .arg(argue::Arg("force")
                                .short_name('f')
                                .long_name("force")
                                .help("Force add even if item exists")
                                .flag(add_force));

    // Build 'remove' subcommand
    auto remove_cmd = argue::Command("remove")
                          .about("Remove an existing item")
                          .alias("rm") // Can use 'rm' as alias
                          .arg(argue::Arg("name")
                                   .positional()
                                   .required()
                                   .help("Name of the item to remove")
                                   .value_of(remove_name))
                          .arg(argue::Arg("recursive")
                                   .short_name('r')
                                   .long_name("recursive")
                                   .help("Remove recursively")
                                   .flag(remove_recursive));

    // Build 'list' subcommand
    auto list_cmd = argue::Command("list")
                        .about("List all items")
                        .alias("ls")
                        .arg(argue::Arg("all")
                                 .short_name('a')
                                 .long_name("all")
                                 .help("Show all items including hidden")
                                 .flag(list_all))
                        .arg(argue::Arg("format")
                                 .long_name("format")
                                 .help("Output format")
                                 .value_of(list_format)
                                 .choices({"table", "json", "csv"})
                                 .default_value("table"));

    // Build main command with subcommands
    bool global_verbose = false;

    auto cmd = argue::Command("argue_subcommands")
                   .version("1.0.0")
                   .about("A demo showing subcommand support (like git, cargo)")
                   .arg(argue::Arg("verbose")
                            .short_name('v')
                            .long_name("verbose")
                            .help("Enable verbose output")
                            .flag(global_verbose)
                            .global()) // Available to all subcommands
                   .subcommand(std::move(add_cmd))
                   .subcommand(std::move(remove_cmd))
                   .subcommand(std::move(list_cmd))
                   .subcommand_required(true);

    // Parse
    auto result = cmd.parse(argc, argv);

    if (result.exit_code() == 0 && !result.message().empty()) {
        std::cout << result.message() << std::endl;
        return 0;
    }

    if (!result) {
        return result.exit();
    }

    // Handle based on which subcommand was used
    auto &matches = cmd.matches();

    if (global_verbose) {
        std::cout << "[verbose] Subcommand: " << matches.subcommand().value_or("none") << "\n";
    }

    if (matches.subcommand() == "add") {
        std::cout << "Adding item: " << add_name;
        if (add_force) std::cout << " (forced)";
        std::cout << std::endl;
    } else if (matches.subcommand() == "remove") {
        std::cout << "Removing item: " << remove_name;
        if (remove_recursive) std::cout << " (recursive)";
        std::cout << std::endl;
    } else if (matches.subcommand() == "list") {
        std::cout << "Listing items";
        if (list_all) std::cout << " (including hidden)";
        std::cout << " in " << list_format << " format" << std::endl;
    }

    return 0;
}
