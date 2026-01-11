/// @file viewport_demo.cpp
/// @brief Demonstration of the Viewport component for scrollable content

#include <scan/scan.hpp>

#include <iostream>
#include <sstream>

int main() {
    std::cout << "=== Viewport Demo ===\n\n";

    // Generate some content
    std::ostringstream content;
    content << "Viewport Component Demo\n";
    content << "=======================\n\n";
    content << "This demonstrates a scrollable viewport.\n";
    content << "You can scroll through this content using:\n\n";
    content << "  j/Down - Scroll down\n";
    content << "  k/Up   - Scroll up\n";
    content << "  d      - Half page down\n";
    content << "  u      - Half page up\n";
    content << "  g      - Go to top\n";
    content << "  G      - Go to bottom\n";
    content << "  Space  - Page down\n\n";

    for (int i = 1; i <= 30; i++) {
        content << "Line " << i << ": Sample content for scrolling demonstration.\n";
    }

    content << "\n--- End of content ---\n";

    // Create a viewport
    scan::Viewport viewport;
    viewport.content(content.str())
        .width(60)
        .height(10)
        .wrap(true);

    // Display the viewport in a simple loop (non-interactive)
    std::cout << "Current viewport contents (first 10 lines):\n\n";
    std::cout << viewport.render() << "\n\n";

    std::cout << "Scroll percentage: " << viewport.scroll_percent() << "%\n";
    std::cout << "At top: " << (viewport.at_top() ? "yes" : "no") << "\n";
    std::cout << "At bottom: " << (viewport.at_bottom() ? "yes" : "no") << "\n\n";

    // Scroll down
    viewport.scroll_down(5);
    std::cout << "After scrolling down 5 lines:\n\n";
    std::cout << viewport.render() << "\n\n";
    std::cout << "Scroll percentage: " << viewport.scroll_percent() << "%\n\n";

    // Go to bottom
    viewport.goto_bottom();
    std::cout << "After going to bottom:\n\n";
    std::cout << viewport.render() << "\n\n";
    std::cout << "At bottom: " << (viewport.at_bottom() ? "yes" : "no") << "\n";

    return 0;
}
