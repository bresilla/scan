/// @file pager_demo.cpp
/// @brief Demonstration of the Pager component for viewing large content

#include <scan/scan.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

std::string generate_content() {
    std::ostringstream oss;

    oss << "Welcome to the Pager Demo!\n";
    oss << "==========================\n\n";

    oss << "This is a demonstration of the full-screen pager component.\n";
    oss << "You can use it to display large amounts of text with scrolling.\n\n";

    oss << "Navigation:\n";
    oss << "-----------\n";
    oss << "  j/Down   - Scroll down one line\n";
    oss << "  k/Up     - Scroll up one line\n";
    oss << "  d/Ctrl-D - Half page down\n";
    oss << "  u/Ctrl-U - Half page up\n";
    oss << "  Space    - Page down\n";
    oss << "  g        - Go to top\n";
    oss << "  G        - Go to bottom\n";
    oss << "  q/Esc    - Quit\n\n";

    oss << "Features:\n";
    oss << "---------\n";
    oss << "  - Full-screen alternate screen mode\n";
    oss << "  - Status bar with position info\n";
    oss << "  - Optional line numbers\n";
    oss << "  - Word wrap support\n";
    oss << "  - Vim-style navigation\n\n";

    // Generate some lorem ipsum content
    for (int i = 1; i <= 50; i++) {
        oss << "Line " << i << ": ";
        oss << "Lorem ipsum dolor sit amet, consectetur adipiscing elit. ";
        oss << "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n";
    }

    oss << "\n--- End of content ---\n";

    return oss.str();
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Read file if provided
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: Could not open file: " << argv[1] << "\n";
            return 1;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        scan::Pager()
            .content(buffer.str())
            .title(argv[1])
            .line_numbers(true)
            .run();
    } else {
        // Use generated content
        scan::Pager()
            .content(generate_content())
            .title("Pager Demo")
            .line_numbers(true)
            .run();
    }

    std::cout << "Pager closed.\n";
    return 0;
}
