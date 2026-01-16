/// @file argu_features.cpp
/// @brief Showcase of new argu features: deprecation, lifecycle hooks, value delimiters, etc.

#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
    // Variables for demonstration
    std::string input_file;
    std::string output_file = "output.txt";
    std::vector<std::string> include_paths;
    int optimization_level = 0;
    int verbosity = 0;
    bool debug = false;
    std::string format = "json";
    int port = 8080;
    std::string log_file;

    // Build command with new features
    auto cmd = argu::Command("argu_features")
        .version("1.0.0")
        .about("Demonstrates new argu features")
        .color(argu::ColorMode::Auto)
        .theme(argu::HelpTheme::colorful())

        // Enable POSIX-style "last wins" for conflicts
        .allow_overrides(true)

        // Enable partial matching for options (--verb matches --verbose)
        .allow_partial_matching(true)

        // Enable prefix matching for subcommands
        .allow_subcommand_prefix(true)

        // Lifecycle hooks
        .pre_parse([]() {
            std::cerr << "[hook] Pre-parse: Starting argument parsing\n";
        })
        .parse_complete([](argu::Matches &matches) {
            std::cerr << "[hook] Parse complete: All arguments parsed\n";
            (void)matches;
        })
        .final_callback([](const argu::Matches &matches) {
            std::cerr << "[hook] Final: All validation complete\n";
            (void)matches;
        })

        // Input file with stdin support (using "-" for stdin)
        .arg(argu::Arg("input")
            .short_name('i')
            .long_name("input")
            .help("Input file (use '-' for stdin)")
            .value_of(input_file)
            .value_name("FILE")
            .allow_stdin()
            .required())

        // Output file with stdout support
        .arg(argu::Arg("output")
            .short_name('o')
            .long_name("output")
            .help("Output file (use '-' for stdout)")
            .value_of(output_file)
            .value_name("FILE")
            .allow_stdout()
            .default_value("output.txt"))

        // Include paths with value delimiter (--include=path1,path2,path3)
        .arg(argu::Arg("include")
            .short_name('I')
            .long_name("include")
            .help("Include paths (comma-separated)")
            .value_of(include_paths)
            .value_name("PATHS")
            .value_delimiter(','))

        // Optimization level with bounded transformer (clamps to 0-3)
        .arg(argu::Arg("optimize")
            .short_name('O')
            .long_name("optimize")
            .help("Optimization level (0-3)")
            .value_of(optimization_level)
            .value_name("LEVEL")
            .default_value("0")
            .transform(argu::transformers::bounded(0, 3)))  // Clamp to 0-3

        // Verbosity counter (-v, -vv, -vvv)
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Increase verbosity (can be repeated)")
            .count(verbosity))

        // Debug flag with deprecated old name
        .arg(argu::Arg("debug")
            .short_name('d')
            .long_name("debug")
            .help("Enable debug mode")
            .flag(debug)
            .renamed_from("debug-mode")  // Old name still works but warns
            .hidden_alias("dbg"))        // Hidden alias for compatibility

        // Format with choices
        .arg(argu::Arg("format")
            .short_name('f')
            .long_name("format")
            .help("Output format")
            .value_of(format)
            .choices({"json", "xml", "csv", "yaml"})
            .default_value("json")
            .deprecated("Use --output-format instead"))  // Mark as deprecated

        // Port with bounded transformer (clamps instead of rejecting)
        .arg(argu::Arg("port")
            .short_name('p')
            .long_name("port")
            .help("Server port (1024-65535)")
            .value_of(port)
            .default_value("8080")
            .transform(argu::transformers::bounded(1024, 65535)))

        // Conditional requirement: log-file is required if debug is set
        .arg(argu::Arg("log-file")
            .long_name("log-file")
            .help("Log file path (required with --debug)")
            .value_of(log_file)
            .value_name("FILE")
            .required_if_eq("debug", "true"))

        // After help with examples
        .after_help(R"(
EXAMPLES:
    Process stdin to stdout:
        argu_features -i - -o -

    Use comma-separated include paths:
        argu_features -i file.txt --include=./src,./lib,./include

    Enable optimization level 3:
        argu_features -i file.txt -O 3

    Use verbose mode:
        argu_features -i file.txt -vvv

    Debug mode with log file:
        argu_features -i file.txt --debug --log-file=debug.log

VALUE SOURCES:
    Arguments can come from (in order of precedence):
    1. Command line arguments (highest priority)
    2. Environment variables
    3. Configuration files
    4. Default values (lowest priority)
)");

    // Parse arguments
    auto result = cmd.parse(argc, argv);

    // Handle help/version/errors
    if (!result) {
        return result.exit();
    }

    auto &matches = cmd.matches();

    // Show results
    std::cout << "\n=== Parsed Arguments ===\n\n";

    std::cout << "Input file:    " << input_file;
    if (input_file == "-") std::cout << " (stdin)";
    std::cout << "\n";

    std::cout << "Output file:   " << output_file;
    if (output_file == "-") std::cout << " (stdout)";
    std::cout << "\n";

    std::cout << "Include paths: ";
    if (include_paths.empty()) {
        std::cout << "(none)";
    } else {
        for (size_t i = 0; i < include_paths.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << include_paths[i];
        }
    }
    std::cout << "\n";

    std::cout << "Optimization:  " << optimization_level << "\n";
    std::cout << "Verbosity:     " << verbosity << "\n";
    std::cout << "Debug mode:    " << (debug ? "yes" : "no") << "\n";
    std::cout << "Format:        " << format << "\n";
    std::cout << "Port:          " << port << "\n";

    if (!log_file.empty()) {
        std::cout << "Log file:      " << log_file << "\n";
    }

    // Show value sources
    std::cout << "\n=== Value Sources ===\n\n";

    auto source_name = [](argu::ValueSource src) -> const char* {
        switch (src) {
            case argu::ValueSource::Default:     return "default";
            case argu::ValueSource::ConfigFile:  return "config file";
            case argu::ValueSource::Environment: return "environment";
            case argu::ValueSource::CommandLine: return "command line";
        }
        return "unknown";
    };

    std::cout << "input:    " << source_name(matches.value_source("input")) << "\n";
    std::cout << "output:   " << source_name(matches.value_source("output")) << "\n";
    std::cout << "optimize: " << source_name(matches.value_source("optimize")) << "\n";
    std::cout << "format:   " << source_name(matches.value_source("format")) << "\n";
    std::cout << "port:     " << source_name(matches.value_source("port")) << "\n";

    return 0;
}
