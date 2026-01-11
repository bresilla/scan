/// @file confirm_demo.cpp
/// @brief Demonstrates the Confirm component

#include <scan/scan.hpp>
#include <iostream>

int main() {
    std::cout << "=== Confirm Demo ===\n\n";

    // Basic confirmation
    std::cout << "1. Basic confirmation:\n";
    auto result = scan::Confirm()
        .prompt("Do you want to continue?")
        .run();

    if (result) {
        std::cout << "Answer: " << (*result ? "Yes" : "No") << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Custom options
    std::cout << "2. Custom options:\n";
    auto delete_result = scan::Confirm()
        .prompt("Delete this file?")
        .affirmative("Delete")
        .negative("Keep")
        .default_value(false)  // Default to "Keep"
        .run();

    if (delete_result) {
        if (*delete_result) {
            std::cout << "File would be deleted.\n\n";
        } else {
            std::cout << "File kept.\n\n";
        }
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Custom styling
    std::cout << "3. Custom styled confirmation:\n";
    auto styled_result = scan::Confirm()
        .prompt("Enable dark mode?")
        .affirmative("Enable")
        .negative("Disable")
        .prompt_color(255, 255, 0)                      // Yellow prompt
        .selected_color(24, 24, 27, 0, 255, 0)          // Dark on green
        .unselected_color(255, 0, 0)                    // Red unselected
        .run();

    if (styled_result) {
        std::cout << "Dark mode: " << (*styled_result ? "Enabled" : "Disabled") << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Using with value_or
    std::cout << "4. Using with value_or (default false):\n";
    bool should_save = scan::Confirm()
        .prompt("Save changes?")
        .run()
        .value_or(false);

    std::cout << "Will save: " << (should_save ? "Yes" : "No") << "\n";

    return 0;
}
