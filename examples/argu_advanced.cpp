/// @file argu_advanced.cpp
/// @brief Advanced features demo: deep subcommands, groups, completions, config files

#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
    // Variables for global options
    bool verbose = false;
    std::string config_file;

    // Variables for 'remote add' subcommand
    std::string remote_add_name;
    std::string remote_add_url;
    bool remote_add_fetch = false;

    // Variables for 'remote remove' subcommand
    std::string remote_remove_name;

    // Variables for 'remote list' subcommand
    bool remote_list_verbose = false;

    // Variables for 'config get' subcommand
    std::string config_get_key;
    bool config_get_global = false;

    // Variables for 'config set' subcommand
    std::string config_set_key;
    std::string config_set_value;
    bool config_set_global = false;

    // Build 'remote add' sub-subcommand
    auto remote_add_cmd = argu::Command("add")
        .about("Add a new remote repository")
        .arg(argu::Arg("name")
            .positional()
            .required()
            .help("Name for the remote")
            .value_of(remote_add_name)
            .validate(argu::validators::identifier()))
        .arg(argu::Arg("url")
            .positional()
            .required()
            .help("URL of the remote repository")
            .value_of(remote_add_url)
            .validate(argu::validators::url()))
        .arg(argu::Arg("fetch")
            .short_name('f')
            .long_name("fetch")
            .help("Immediately fetch from the remote")
            .flag(remote_add_fetch));

    // Build 'remote remove' sub-subcommand
    auto remote_remove_cmd = argu::Command("remove")
        .about("Remove a remote")
        .alias("rm")
        .arg(argu::Arg("name")
            .positional()
            .required()
            .help("Name of the remote to remove")
            .value_of(remote_remove_name));

    // Build 'remote list' sub-subcommand
    auto remote_list_cmd = argu::Command("list")
        .about("List configured remotes")
        .alias("ls")
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Show remote URLs")
            .flag(remote_list_verbose));

    // Build 'remote' subcommand (contains sub-subcommands)
    auto remote_cmd = argu::Command("remote")
        .about("Manage remote repositories")
        .subcommand(std::move(remote_add_cmd))
        .subcommand(std::move(remote_remove_cmd))
        .subcommand(std::move(remote_list_cmd))
        .subcommand_required(true);

    // Build 'config get' sub-subcommand
    auto config_get_cmd = argu::Command("get")
        .about("Get a configuration value")
        .arg(argu::Arg("key")
            .positional()
            .required()
            .help("Configuration key")
            .value_of(config_get_key))
        .arg(argu::Arg("global")
            .long_name("global")
            .help("Use global configuration")
            .flag(config_get_global));

    // Build 'config set' sub-subcommand
    auto config_set_cmd = argu::Command("set")
        .about("Set a configuration value")
        .arg(argu::Arg("key")
            .positional()
            .required()
            .help("Configuration key")
            .value_of(config_set_key))
        .arg(argu::Arg("value")
            .positional()
            .required()
            .help("Configuration value")
            .value_of(config_set_value))
        .arg(argu::Arg("global")
            .long_name("global")
            .help("Use global configuration")
            .flag(config_set_global));

    // Build 'config' subcommand (contains sub-subcommands)
    auto config_cmd = argu::Command("config")
        .about("Manage configuration")
        .subcommand(std::move(config_get_cmd))
        .subcommand(std::move(config_set_cmd))
        .subcommand_required(true);

    // Build 'completions' subcommand
    std::string shell_name;
    auto completions_cmd = argu::Command("completions")
        .about("Generate shell completion scripts")
        .arg(argu::Arg("shell")
            .positional()
            .required()
            .help("Target shell")
            .value_of(shell_name)
            .choices({"bash", "zsh", "fish", "powershell", "elvish"}));

    // Build main command with theme and color support
    auto cmd = argu::Command("argu_advanced")
        .version("2.0.0")
        .about("Advanced argu features demonstration")
        .author("argu developers")
        .color(argu::ColorMode::Auto)
        .theme(argu::HelpTheme::colorful())
        .suggest_threshold(3)  // Suggest alternatives within edit distance 3
        .after_help("Examples:\n  argu_advanced remote add origin https://example.com\n  argu_advanced config set user.name \"John Doe\"")
        // Global options
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Enable verbose output")
            .flag(verbose)
            .global())
        .arg(argu::Arg("config")
            .short_name('c')
            .long_name("config")
            .help("Path to config file")
            .value_of(config_file)
            .value_name("FILE")
            .env("ARGU_CONFIG")
            .global())
        // Subcommands
        .subcommand(std::move(remote_cmd))
        .subcommand(std::move(config_cmd))
        .subcommand(std::move(completions_cmd));

    // Parse arguments
    auto result = cmd.parse(argc, argv);

    // Handle help/version/errors
    if (!result) {
        return result.exit();
    }

    auto &matches = cmd.matches();

    if (verbose) {
        std::cout << "[verbose] Subcommand chain: ";
        auto chain = matches.subcommand_chain();
        for (size_t i = 0; i < chain.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << chain[i];
        }
        std::cout << "\n\n";
    }

    // Handle subcommands
    if (matches.subcommand() == "remote") {
        auto *remote_matches = matches.subcommand_matches();
        if (remote_matches && remote_matches->subcommand() == "add") {
            std::cout << "Adding remote '" << remote_add_name << "' with URL: " << remote_add_url;
            if (remote_add_fetch) std::cout << " (will fetch)";
            std::cout << "\n";
        } else if (remote_matches && remote_matches->subcommand() == "remove") {
            std::cout << "Removing remote: " << remote_remove_name << "\n";
        } else if (remote_matches && remote_matches->subcommand() == "list") {
            std::cout << "Listing remotes";
            if (remote_list_verbose) std::cout << " (verbose)";
            std::cout << "\n";
        }
    } else if (matches.subcommand() == "config") {
        auto *config_matches = matches.subcommand_matches();
        if (config_matches && config_matches->subcommand() == "get") {
            std::cout << "Getting config key: " << config_get_key;
            if (config_get_global) std::cout << " (global)";
            std::cout << "\n";
        } else if (config_matches && config_matches->subcommand() == "set") {
            std::cout << "Setting " << config_set_key << " = " << config_set_value;
            if (config_set_global) std::cout << " (global)";
            std::cout << "\n";
        }
    } else if (matches.subcommand() == "completions") {
        auto shell = argu::CompletionGenerator::parse_shell(shell_name);
        if (shell) {
            std::cout << cmd.generate_completions(*shell);
        }
    }

    return 0;
}
