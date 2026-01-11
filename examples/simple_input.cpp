/// @file simple_input.cpp
/// @brief Simple example showing basic TextInput usage

#include <scan/scan.hpp>
#include <iostream>

int main() {
    std::cout << "Simple Input Example\n\n";

    // Basic text input
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();

    if (name) {
        std::cout << "Hello, " << *name << "!\n";
    } else {
        std::cout << "(cancelled)\n";
    }

    return 0;
}
