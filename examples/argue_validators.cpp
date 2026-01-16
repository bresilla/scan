/// @file argue_validators.cpp
/// @brief Demonstrates validators and advanced argument features in Argue

#include <argue/argue.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
    // Variables to bind to
    std::string input_file;
    std::string output_file;
    int port = 8080;
    std::string email;
    int verbosity = 0;
    std::string log_level = "info";

    // Build command with validators
    auto cmd =
        argue::Command("argue_validators")
            .version("1.0.0")
            .about("Demonstrates argument validation features")

            // File that must exist
            .arg(argue::Arg("input")
                     .short_name('i')
                     .long_name("input")
                     .help("Input file (must exist)")
                     .value_of(input_file)
                     .value_name("FILE")
                     .validate(argue::validators::file_exists()))

            // Output file (parent directory must exist)
            .arg(argue::Arg("output")
                     .short_name('o')
                     .long_name("output")
                     .help("Output file")
                     .value_of(output_file)
                     .value_name("FILE"))

            // Port with range validation
            .arg(argue::Arg("port")
                     .short_name('p')
                     .long_name("port")
                     .help("Server port (1024-65535)")
                     .value_of(port)
                     .default_value("8080")
                     .validate(argue::validators::range(1024, 65535)))

            // Email validation
            .arg(argue::Arg("email")
                     .short_name('e')
                     .long_name("email")
                     .help("Contact email address")
                     .value_of(email)
                     .validate(argue::validators::email()))

            // Counting flag (-v, -vv, -vvv)
            .arg(argue::Arg("verbosity")
                     .short_name('v')
                     .long_name("verbose")
                     .help("Increase verbosity (can be repeated)")
                     .count(verbosity))

            // Choices constraint
            .arg(argue::Arg("log-level")
                     .short_name('l')
                     .long_name("log-level")
                     .help("Log level")
                     .value_of(log_level)
                     .choices({"debug", "info", "warn", "error"})
                     .default_value("info"))

            // Environment variable fallback
            .arg(argue::Arg("config")
                     .long_name("config")
                     .help("Config file path (or set ARGUE_CONFIG env var)")
                     .env("ARGUE_CONFIG")
                     .value_name("PATH"));

    // Parse
    auto result = cmd.parse(argc, argv);

    if (result.exit_code() == 0 && !result.message().empty()) {
        std::cout << result.message() << std::endl;
        return 0;
    }

    if (!result) {
        return result.exit();
    }

    // Show parsed values
    std::cout << "Parsed configuration:\n";
    std::cout << "  Input file: " << (input_file.empty() ? "(not set)" : input_file) << "\n";
    std::cout << "  Output file: " << (output_file.empty() ? "(not set)" : output_file) << "\n";
    std::cout << "  Port: " << port << "\n";
    std::cout << "  Email: " << (email.empty() ? "(not set)" : email) << "\n";
    std::cout << "  Verbosity level: " << verbosity << "\n";
    std::cout << "  Log level: " << log_level << "\n";

    auto config_val = cmd.matches().get_one("config");
    std::cout << "  Config: " << (config_val ? *config_val : "(not set)") << "\n";

    return 0;
}
