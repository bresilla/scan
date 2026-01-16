/// @file simple_input.cpp
/// @brief Simple example showing basic TextInput usage

#include <iostream>
#include <scan/scan.hpp>

int main() {
    std::cout << "Simple Input Example\n\n";

    // Basic text input
    auto name = scan::TextInput()
                    .prompt("Name: ")
                    .prompt_color(0, 255, 255) // Cyan
                    .placeholder("Enter your name")
                    .run();

    if (name) {
        std::cout << "Hello, " << *name << "!\n";
    } else {
        std::cout << "(cancelled)\n";
    }

    return 0;
}
