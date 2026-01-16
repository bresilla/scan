/// @file argu_showcase.cpp
/// @brief Comprehensive showcase of argu CLI library features
///
/// This example demonstrates a realistic CLI tool (a file processing utility)
/// showcasing all major features of the argu library.

#include <argu/argu.hpp>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    // =========================================================================
    // Variables to bind to arguments
    // =========================================================================

    // Global options (available in all subcommands)
    bool verbose = false;
    bool quiet = false;
    bool color = true;  // Negatable flag: --color / --no-color
    std::string config_path;
    int log_level = 0;

    // Process subcommand options
    std::string input_file;
    std::string output_file = "output.txt";
    std::string format = "auto";
    bool force = false;
    bool dry_run = false;
    std::vector<std::string> filters;
    int threads = 4;

    // Analyze subcommand options
    bool deep_scan = false;
    std::string report_format = "text";
    std::string output_dir = ".";

    // Config subcommand options
    std::string config_key;
    std::string config_value;

    // =========================================================================
    // Build the main command with all features
    // =========================================================================

    auto cmd = argu::Command("fileutil")
        .version("2.0.0")
        .about("A powerful file processing utility")
        .long_about(R"(
FileUtil is a comprehensive file processing tool that demonstrates
all features of the argu argument parsing library.

It supports multiple subcommands, environment variable configuration,
config file layering, and advanced argument handling features.
)")
        .author("Argu Library Team")

        // =====================================================================
        // Styling and behavior configuration
        // =====================================================================
        .color(argu::ColorMode::Auto)
        .theme(argu::HelpTheme::colorful())

        // Enable powerful features
        .allow_partial_matching(true)      // --verb matches --verbose
        .allow_subcommand_prefix(true)     // "proc" matches "process"
        .allow_overrides(true)             // Last option wins (POSIX-style)

        // Environment variable prefix (FILEUTIL_VERBOSE, FILEUTIL_CONFIG, etc.)
        .env_prefix("FILEUTIL_")

        // Config file layering (loaded in order, later overrides earlier)
        .add_config_layer("/etc/fileutil/config.toml", false)  // System config (optional)
        .add_config_layer("~/.config/fileutil/config.toml", false)  // User config (optional)
        .config_file("config", ".fileutil.toml")  // Project config via --config

        // Typo suggestion threshold
        .suggest_threshold(3)

        // =====================================================================
        // Custom help sections (appear in order specified)
        // =====================================================================
        .help_section("ENVIRONMENT VARIABLES", R"(
    FILEUTIL_VERBOSE    Set to 1 to enable verbose output
    FILEUTIL_COLOR      Set to 0 to disable colors
    FILEUTIL_CONFIG     Path to config file
    FILEUTIL_LOG_LEVEL  Logging verbosity (0-3)
)", 90)
        .help_section("EXAMPLES", R"(
    Process a file:
        fileutil process -i input.txt -o output.txt

    Analyze with deep scan:
        fileutil analyze --deep ./src

    Use abbreviated commands (prefix matching):
        fileutil proc -i file.txt    # matches 'process'
        fileutil ana --deep .        # matches 'analyze'

    Override config with environment:
        FILEUTIL_VERBOSE=1 fileutil process -i file.txt
)", 95)
        .after_help(R"(
For more information, visit: https://github.com/example/fileutil
Report bugs to: bugs@example.com
)")

        // =====================================================================
        // Global arguments (propagate to all subcommands)
        // =====================================================================
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Enable verbose output")
            .flag(verbose)
            .env("FILEUTIL_VERBOSE")
            .global())

        .arg(argu::Arg("quiet")
            .short_name('q')
            .long_name("quiet")
            .help("Suppress all output except errors")
            .flag(quiet)
            .conflicts_with("verbose")
            .global())

        // Negatable flag: --color (default) vs --no-color
        .arg(argu::Arg("color")
            .long_name("color")
            .help("Enable colored output (use --no-color to disable)")
            .flag(color)
            .negatable()  // Allows --no-color
            .env("FILEUTIL_COLOR")
            .global())

        .arg(argu::Arg("config")
            .short_name('c')
            .long_name("config")
            .help("Path to config file")
            .value_of(config_path)
            .value_name("FILE")
            .hint_file()
            .global())

        .arg(argu::Arg("log-level")
            .short_name('l')
            .long_name("log-level")
            .help("Logging level (0=off, 1=error, 2=warn, 3=debug)")
            .count(log_level)
            .env("FILEUTIL_LOG_LEVEL")
            .global())

        // =====================================================================
        // Subcommands
        // =====================================================================

        // --- PROCESS subcommand ---
        .subcommand(
            argu::Command("process")
                .about("Process input files")
                .alias("proc")  // Short alias
                .visible_alias("p")  // Shown in help

                // Argument groups for display and constraints
                .group(argu::ArgGroup("output-format")
                    .args({"json", "xml", "csv"})
                    .mutually_exclusive()
                    .help("Output format (pick one)"))

                .group(argu::ArgGroup("execution-mode")
                    .args({"force", "dry-run"})
                    .mutually_exclusive()
                    .help("Execution mode"))

                // Input file (required)
                .arg(argu::Arg("input")
                    .short_name('i')
                    .long_name("input")
                    .help("Input file to process")
                    .value_of(input_file)
                    .value_name("FILE")
                    .hint_file()
                    .required()
                    .validate(argu::validators::file_exists()))

                // Output file
                .arg(argu::Arg("output")
                    .short_name('o')
                    .long_name("output")
                    .help("Output file path")
                    .value_of(output_file)
                    .value_name("FILE")
                    .hint_file()
                    .default_value("output.txt"))

                // Format with choices
                .arg(argu::Arg("format")
                    .short_name('f')
                    .long_name("format")
                    .help("Output format")
                    .value_of(format)
                    .choices({"auto", "json", "xml", "csv", "yaml"})
                    .default_value("auto"))

                // Mutex group: force vs dry-run
                .arg(argu::Arg("force")
                    .long_name("force")
                    .help("Overwrite existing files")
                    .flag(force)
                    .group("execution-mode"))

                .arg(argu::Arg("dry-run")
                    .short_name('n')
                    .long_name("dry-run")
                    .help("Show what would be done without doing it")
                    .flag(dry_run)
                    .group("execution-mode"))

                // Filters with value delimiter
                .arg(argu::Arg("filter")
                    .short_name('F')
                    .long_name("filter")
                    .help("Filters to apply (comma-separated)")
                    .value_of(filters)
                    .value_delimiter(',')
                    .value_name("FILTERS"))

                // Threads with validator
                .arg(argu::Arg("threads")
                    .short_name('t')
                    .long_name("threads")
                    .help("Number of processing threads (1-32)")
                    .value_of(threads)
                    .default_value("4")
                    .validate(argu::validators::range(1, 32)))

                // Callback for when processing is requested
                .callback([&](const argu::Matches& m) {
                    (void)m;
                    if (verbose) {
                        std::cout << "[process] Processing " << input_file
                                  << " -> " << output_file << "\n";
                    }
                })
        )

        // --- ANALYZE subcommand ---
        .subcommand(
            argu::Command("analyze")
                .about("Analyze files or directories")
                .alias("ana")

                // Path to analyze (positional, variadic)
                .arg(argu::Arg("paths")
                    .positional()
                    .help("Files or directories to analyze")
                    .value_name("PATH")
                    .takes_one_or_more()
                    .required())

                // Deep scan flag
                .arg(argu::Arg("deep")
                    .short_name('d')
                    .long_name("deep")
                    .help("Perform deep recursive scan")
                    .flag(deep_scan))

                // Report format
                .arg(argu::Arg("report-format")
                    .short_name('r')
                    .long_name("report-format")
                    .help("Report output format")
                    .value_of(report_format)
                    .choices({"text", "json", "html"})
                    .default_value("text"))

                // Output directory
                .arg(argu::Arg("output-dir")
                    .short_name('o')
                    .long_name("output-dir")
                    .help("Directory for report output")
                    .value_of(output_dir)
                    .hint_dir()
                    .default_value(".")
                    .validate(argu::validators::dir_exists()))
        )

        // --- CONFIG subcommand with nested subcommands ---
        .subcommand(
            argu::Command("config")
                .about("Manage configuration")
                .subcommand_required(true)
                .allow_subcommand_prefix(true)  // Nested prefix matching

                // GET subcommand
                .subcommand(
                    argu::Command("get")
                        .about("Get a configuration value")
                        .arg(argu::Arg("key")
                            .positional()
                            .help("Configuration key")
                            .value_of(config_key)
                            .required())
                )

                // SET subcommand
                .subcommand(
                    argu::Command("set")
                        .about("Set a configuration value")
                        .arg(argu::Arg("key")
                            .positional()
                            .help("Configuration key")
                            .value_of(config_key)
                            .required())
                        .arg(argu::Arg("value")
                            .positional()
                            .help("Value to set")
                            .value_of(config_value)
                            .required())
                )

                // LIST subcommand
                .subcommand(
                    argu::Command("list")
                        .about("List all configuration values")
                        .alias("ls")
                )

                // EDIT subcommand (hidden, for advanced users)
                .subcommand(
                    argu::Command("edit")
                        .about("Open config in editor")
                        .hidden()  // Not shown in help, but still works
                )
        )

        // --- HIDDEN subcommand (for internal/debug use) ---
        .subcommand(
            argu::Command("debug")
                .about("Internal debugging commands")
                .hidden()  // Not shown in help
                .subcommand(argu::Command("dump").about("Dump internal state"))
                .subcommand(argu::Command("test").about("Run internal tests"))
        );

    // =========================================================================
    // Parse and handle results
    // =========================================================================

    auto result = cmd.parse(argc, argv);

    // Handle help/version (exit code 0 with message) or errors
    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();

    // =========================================================================
    // Display parsed results
    // =========================================================================

    if (!quiet) {
        std::cout << "\n";

        if (color) {
            std::cout << "\033[1;36m=== FileUtil v2.0.0 ===\033[0m\n\n";
        } else {
            std::cout << "=== FileUtil v2.0.0 ===\n\n";
        }

        // Show global options
        std::cout << "Global Options:\n";
        std::cout << "  Verbose:   " << (verbose ? "yes" : "no") << "\n";
        std::cout << "  Quiet:     " << (quiet ? "yes" : "no") << "\n";
        std::cout << "  Color:     " << (color ? "yes" : "no") << "\n";
        std::cout << "  Log Level: " << log_level << "\n";
        if (!config_path.empty()) {
            std::cout << "  Config:    " << config_path << "\n";
        }
        std::cout << "\n";

        // Show subcommand chain
        auto subcmd_chain = matches.subcommand_chain();
        if (!subcmd_chain.empty()) {
            std::cout << "Subcommand: ";
            for (size_t i = 0; i < subcmd_chain.size(); ++i) {
                if (i > 0) std::cout << " -> ";
                std::cout << subcmd_chain[i];
            }
            std::cout << "\n\n";

            // Handle specific subcommands
            if (subcmd_chain[0] == "process") {
                std::cout << "Process Options:\n";
                std::cout << "  Input:   " << input_file << "\n";
                std::cout << "  Output:  " << output_file << "\n";
                std::cout << "  Format:  " << format << "\n";
                std::cout << "  Force:   " << (force ? "yes" : "no") << "\n";
                std::cout << "  Dry Run: " << (dry_run ? "yes" : "no") << "\n";
                std::cout << "  Threads: " << threads << "\n";
                if (!filters.empty()) {
                    std::cout << "  Filters: ";
                    for (size_t i = 0; i < filters.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << filters[i];
                    }
                    std::cout << "\n";
                }
            }
            else if (subcmd_chain[0] == "analyze") {
                auto* sub = matches.subcommand_matches();
                if (sub) {
                    auto paths = sub->get_many("paths");
                    std::cout << "Analyze Options:\n";
                    std::cout << "  Paths: ";
                    for (size_t i = 0; i < paths.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << paths[i];
                    }
                    std::cout << "\n";
                    std::cout << "  Deep Scan:     " << (deep_scan ? "yes" : "no") << "\n";
                    std::cout << "  Report Format: " << report_format << "\n";
                    std::cout << "  Output Dir:    " << output_dir << "\n";
                }
            }
            else if (subcmd_chain[0] == "config") {
                if (subcmd_chain.size() > 1) {
                    if (subcmd_chain[1] == "get") {
                        std::cout << "Getting config key: " << config_key << "\n";
                    }
                    else if (subcmd_chain[1] == "set") {
                        std::cout << "Setting " << config_key << " = " << config_value << "\n";
                    }
                    else if (subcmd_chain[1] == "list") {
                        std::cout << "Listing all configuration values...\n";
                    }
                    else if (subcmd_chain[1] == "edit") {
                        std::cout << "Opening config in editor (hidden command)...\n";
                    }
                }
            }
            else if (subcmd_chain[0] == "debug") {
                std::cout << "Debug mode activated (hidden command)\n";
            }
        } else {
            std::cout << "No subcommand specified. Use --help to see available commands.\n";
        }

        // Show value sources for selected args
        std::cout << "\nValue Sources:\n";
        std::cout << "  verbose:   " << static_cast<int>(matches.value_source("verbose")) << "\n";
        std::cout << "  color:     " << static_cast<int>(matches.value_source("color")) << "\n";
    }

    return 0;
}
