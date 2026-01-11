/// @file textinput_demo.cpp
/// @brief Demonstrates the TextInput component

#include <scan/scan.hpp>
#include <iostream>

int main() {
    std::cout << "=== TextInput Demo ===\n\n";

    // Basic text input
    std::cout << "1. Basic text input:\n";
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();

    if (name) {
        std::cout << "Hello, " << *name << "!\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Password input
    std::cout << "2. Password input:\n";
    auto password = scan::TextInput()
        .prompt("Password: ")
        .placeholder("Enter password")
        .password(true)
        .run();

    if (password) {
        std::cout << "Password length: " << password->length() << " characters\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Custom styling
    std::cout << "3. Custom styled input:\n";
    auto email = scan::TextInput()
        .prompt("Email: ")
        .placeholder("user@example.com")
        .prompt_color(0, 255, 255)       // Cyan
        .text_color(0, 255, 0)           // Green
        .placeholder_color(136, 136, 136) // Gray
        .run();

    if (email) {
        std::cout << "Email: " << *email << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // With character limit
    std::cout << "4. Input with character limit (10 chars max):\n";
    auto code = scan::TextInput()
        .prompt("Code: ")
        .placeholder("Enter code")
        .char_limit(10)
        .run();

    if (code) {
        std::cout << "Code: " << *code << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // With initial value
    std::cout << "5. Input with initial value:\n";
    auto edited = scan::TextInput()
        .prompt("Edit: ")
        .value("Hello, World!")
        .run();

    if (edited) {
        std::cout << "Result: " << *edited << "\n";
    } else {
        std::cout << "(cancelled)\n";
    }

    return 0;
}
