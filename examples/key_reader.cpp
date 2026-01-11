/// @file key_reader.cpp
/// @brief Demonstrates low-level key reading for debugging

#include <scan/terminal/terminal.hpp>
#include <scan/terminal/raw_mode.hpp>
#include <scan/input/reader.hpp>

#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Key Reader Demo ===\n";
    std::cout << "Press keys to see their codes. Press Ctrl+C to exit.\n\n";

    // Enter raw mode
    scan::terminal::RawMode raw_mode;

    if (!raw_mode) {
        std::cerr << "Failed to enter raw mode. Is stdin a TTY?\n";
        return 1;
    }

    std::cout << "Raw mode enabled. Reading keys...\n\n";
    std::cout << std::left << std::setw(20) << "Key Name"
              << std::setw(10) << "Rune"
              << std::setw(10) << "Alt"
              << std::setw(10) << "Ctrl"
              << "String\n";
    std::cout << std::string(60, '-') << "\n";

    while (true) {
        auto event = scan::input::read_key(-1);  // Block forever

        if (!event) continue;

        // Check for Ctrl+C
        if (event->key == scan::input::Key::CtrlC) {
            std::cout << "\nCtrl+C pressed. Exiting.\n";
            break;
        }

        // Print key information
        std::cout << std::left << std::setw(20) << scan::input::key_name(event->key);

        // Print rune if applicable
        if (event->key == scan::input::Key::Rune || event->key == scan::input::Key::Space) {
            std::cout << std::setw(10) << static_cast<uint32_t>(event->rune);
        } else {
            std::cout << std::setw(10) << "-";
        }

        std::cout << std::setw(10) << (event->alt ? "yes" : "no");
        std::cout << std::setw(10) << (event->ctrl ? "yes" : "no");

        // Print string representation
        std::cout << scan::input::key_event_to_string(*event);

        std::cout << "\n";
    }

    return 0;
}
