/// @file form_demo.cpp
/// @brief Demonstrates composing multiple components into a form

#include <scan/scan.hpp>
#include <iostream>

int main() {
    std::cout << "=== User Registration Form ===\n\n";

    // Step 1: Get username
    std::cout << "Step 1 of 4\n";
    auto username = scan::TextInput()
        .prompt("Username: ")
        .placeholder("Enter username")
        .char_limit(20)
        .run();

    if (!username) {
        std::cout << "Registration cancelled.\n";
        return 1;
    }

    // Step 2: Get email
    std::cout << "\nStep 2 of 4\n";
    auto email = scan::TextInput()
        .prompt("Email: ")
        .placeholder("user@example.com")
        .run();

    if (!email) {
        std::cout << "Registration cancelled.\n";
        return 1;
    }

    // Step 3: Get password
    std::cout << "\nStep 3 of 4\n";
    auto password = scan::TextInput()
        .prompt("Password: ")
        .placeholder("Enter password")
        .password(true)
        .run();

    if (!password) {
        std::cout << "Registration cancelled.\n";
        return 1;
    }

    // Step 4: Select role
    std::cout << "\nStep 4 of 4\n";
    std::cout << "Select your role:\n";
    auto role = scan::List()
        .items({"Developer", "Designer", "Manager", "Other"})
        .cursor("> ")
        .run();

    if (!role) {
        std::cout << "Registration cancelled.\n";
        return 1;
    }

    // Confirmation
    std::cout << "\n--- Review ---\n";
    std::cout << "Username: " << *username << "\n";
    std::cout << "Email: " << *email << "\n";
    std::cout << "Password: " << std::string(password->length(), '*') << "\n";
    std::cout << "Role: " << *role << "\n\n";

    auto confirm = scan::Confirm()
        .prompt("Create account?")
        .affirmative("Create")
        .negative("Cancel")
        .run();

    if (confirm && *confirm) {
        std::cout << "\n✓ Account created successfully!\n";
    } else {
        std::cout << "\n✗ Account creation cancelled.\n";
    }

    return 0;
}
