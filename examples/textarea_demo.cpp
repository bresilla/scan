/// @file textarea_demo.cpp
/// @brief Demonstration of the TextArea component for multi-line input

#include <scan/scan.hpp>

#include <iostream>

int main() {
    std::cout << "=== TextArea Demo ===\n\n";

    std::cout << "Enter a multi-line message (Ctrl+D to submit, Esc to cancel):\n\n";

    auto result = scan::TextArea()
                      .placeholder("Type your message here...")
                      .width(60)
                      .height(10)
                      .show_line_numbers(true)
                      .run();

    if (result.has_value()) {
        std::cout << "\n--- Your message ---\n";
        std::cout << result.value() << "\n";
        std::cout << "--- End ---\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    return 0;
}
