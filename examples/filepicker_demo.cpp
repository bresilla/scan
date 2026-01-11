/// @file filepicker_demo.cpp
/// @brief Demonstration of the FilePicker component

#include <scan/scan.hpp>

#include <iostream>

int main() {
    std::cout << "=== FilePicker Demo ===\n\n";

    std::cout << "Demo 1: Select any file\n";
    std::cout << "Navigation: arrows/hjkl, Enter to select, . for hidden files, Esc to cancel\n\n";

    auto file = scan::FilePicker()
                    .path(".")
                    .height(12)
                    .run();

    if (file.has_value()) {
        std::cout << "\nSelected: " << file.value().string() << "\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    std::cout << "\n";

    // Demo 2: Select only C++ files
    std::cout << "Demo 2: Select only C++ source files (.cpp, .hpp, .h)\n\n";

    auto cpp_file = scan::FilePicker()
                        .path(".")
                        .extensions({".cpp", ".hpp", ".h"})
                        .height(10)
                        .run();

    if (cpp_file.has_value()) {
        std::cout << "\nSelected: " << cpp_file.value().string() << "\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    std::cout << "\n";

    // Demo 3: Select a directory
    std::cout << "Demo 3: Select a directory\n\n";

    auto dir = scan::FilePicker()
                   .path(".")
                   .file_allowed(false)
                   .dir_allowed(true)
                   .height(10)
                   .run();

    if (dir.has_value()) {
        std::cout << "\nSelected directory: " << dir.value().string() << "\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    return 0;
}
