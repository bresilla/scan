/// @file list_demo.cpp
/// @brief Demonstrates the List component

#include <iostream>
#include <scan/scan.hpp>

int main() {
    // Single select
    std::cout << "1. Single select (choose a color):\n";
    auto color = scan::List().items({"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"}).cursor("> ").run();

    if (color) {
        std::cout << "Selected: " << *color << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Multi-select with unlimited selection
    std::cout << "2. Multi-select (choose your favorite fruits):\n";
    std::cout << "   Use Space/Tab to select, Enter to confirm\n";
    auto fruits = scan::List()
                      .items({"Apple", "Banana", "Cherry", "Date", "Elderberry", "Fig", "Grape"})
                      .no_limit()
                      .cursor("→ ")
                      .selected_prefix("[✓] ")
                      .unselected_prefix("[ ] ")
                      .run_multi();

    if (fruits) {
        std::cout << "Selected fruits: ";
        for (size_t i = 0; i < fruits->size(); i++) {
            if (i > 0)
                std::cout << ", ";
            std::cout << (*fruits)[i];
        }
        std::cout << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Multi-select with limit
    std::cout << "3. Multi-select with limit (choose up to 3 languages):\n";
    auto languages = scan::List()
                         .items({"C++", "Python", "JavaScript", "Rust", "Go", "Java", "Ruby", "Swift"})
                         .limit(3)
                         .cursor_color(0, 255, 255) // Cyan
                         .selected_color(0, 255, 0) // Green
                         .run_multi();

    if (languages) {
        std::cout << "Selected languages: ";
        for (size_t i = 0; i < languages->size(); i++) {
            if (i > 0)
                std::cout << ", ";
            std::cout << (*languages)[i];
        }
        std::cout << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Long list with scrolling
    std::cout << "4. Long list with scrolling (height=5):\n";
    std::vector<std::string> numbers;
    for (int i = 1; i <= 20; i++) {
        numbers.push_back("Item " + std::to_string(i));
    }

    auto item = scan::List().items(numbers).height(5).run();

    if (item) {
        std::cout << "Selected: " << *item << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Pre-selected items
    std::cout << "5. Pre-selected items:\n";
    auto preselected = scan::List()
                           .items({"Option A", "Option B", "Option C", "Option D"})
                           .selected({1, 2}) // Pre-select B and C
                           .no_limit()
                           .run_multi();

    if (preselected) {
        std::cout << "Final selection: ";
        for (size_t i = 0; i < preselected->size(); i++) {
            if (i > 0)
                std::cout << ", ";
            std::cout << (*preselected)[i];
        }
        std::cout << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Rich items with title + description (two-line display)
    std::cout << "6. Rich items with descriptions (two-line display):\n";
    auto editor = scan::List()
                      .items({{"Vim", "The ubiquitous modal text editor"},
                              {"Emacs", "An extensible, customizable, free text editor"},
                              {"VS Code", "A lightweight but powerful source code editor"},
                              {"Neovim", "Hyperextensible Vim-based text editor"},
                              {"Sublime Text", "A sophisticated text editor for code and markup"}})
                      .height(8)
                      .cursor("→ ")
                      .run();

    if (editor) {
        std::cout << "Selected editor: " << *editor << "\n\n";
    } else {
        std::cout << "(cancelled)\n\n";
    }

    // Rich items multi-select
    std::cout << "7. Rich items multi-select (choose your tools):\n";
    auto tools = scan::List()
                     .items({{"Git", "Distributed version control system"},
                             {"Docker", "Platform for developing and running containers"},
                             {"Kubernetes", "Container orchestration platform"},
                             {"Terraform", "Infrastructure as code tool"},
                             {"Ansible", "Automation and configuration management"}})
                     .no_limit()
                     .height(10)
                     .cursor("▸ ")
                     .selected_prefix("[●] ")
                     .unselected_prefix("[○] ")
                     .run_items(); // Returns full ListItem with description

    if (tools) {
        std::cout << "Selected tools:\n";
        for (const auto &tool : *tools) {
            std::cout << "  - " << tool.title;
            if (!tool.description.empty()) {
                std::cout << ": " << tool.description;
            }
            std::cout << "\n";
        }
    } else {
        std::cout << "(cancelled)\n";
    }

    return 0;
}
