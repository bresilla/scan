/// @file argue_basic.cpp
/// @brief Basic argument parsing demonstration with Argue library

#include <argue/argue.hpp>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    // Variables to bind arguments to
    std::string name;
    int count = 1;
    bool verbose = false;
    std::vector<std::string> files;

    // Build the command
    auto cmd = argue::Command("argue_basic")
                   .version("1.0.0")
                   .about("A basic example demonstrating the Argue argument parsing library")
                   .arg(argue::Arg("name")
                            .positional()
                            .help("Your name")
                            .required()
                            .value_of(name)
                            .value_name("NAME"))
                   .arg(argue::Arg("count")
                            .short_name('c')
                            .long_name("count")
                            .help("Number of times to greet")
                            .value_of(count)
                            .default_value("1")
                            .value_name("NUM"))
                   .arg(argue::Arg("verbose")
                            .short_name('v')
                            .long_name("verbose")
                            .help("Enable verbose output")
                            .flag(verbose))
                   .arg(argue::Arg("files")
                            .short_name('f')
                            .long_name("file")
                            .help("Input files to process")
                            .value_of(files)
                            .takes_multiple()
                            .value_name("FILE"));

    // Parse the arguments
    auto result = cmd.parse(argc, argv);

    // Handle help/version (exit code 0 with message)
    if (result.exit_code() == 0 && !result.message().empty()) {
        std::cout << result.message() << std::endl;
        return 0;
    }

    // Handle errors
    if (!result) {
        return result.exit();
    }

    // Use the parsed values
    if (verbose) {
        std::cout << "Verbose mode enabled\n";
        std::cout << "Arguments received:\n";
        std::cout << "  Name: " << name << "\n";
        std::cout << "  Count: " << count << "\n";
        std::cout << "  Files: " << files.size() << " file(s)\n";
        for (const auto &f : files) {
            std::cout << "    - " << f << "\n";
        }
        std::cout << "\n";
    }

    // Do the greeting
    for (int i = 0; i < count; ++i) {
        std::cout << "Hello, " << name << "!" << std::endl;
    }

    if (!files.empty()) {
        std::cout << "\nWould process files: ";
        for (size_t i = 0; i < files.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << files[i];
        }
        std::cout << std::endl;
    }

    return 0;
}
