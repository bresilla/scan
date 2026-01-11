/// @file filter_demo.cpp
/// @brief Demonstration of the Filter component with fuzzy search

#include <scan/scan.hpp>

#include <iostream>
#include <vector>

int main() {
    std::cout << "=== Filter Demo ===\n\n";

    // Create a list of programming languages to filter
    std::vector<std::string> languages = {
        "C++",          "Python",   "JavaScript", "TypeScript", "Rust",
        "Go",           "Java",     "C#",         "Ruby",       "Swift",
        "Kotlin",       "Scala",    "Haskell",    "Erlang",     "Elixir",
        "Clojure",      "F#",       "OCaml",      "Lua",        "Perl",
        "PHP",          "R",        "Julia",      "Dart",       "Zig",
        "Nim",          "Crystal",  "V",          "Odin",       "Gleam"
    };

    std::cout << "Type to filter programming languages (fuzzy search):\n\n";

    auto result = scan::Filter()
                      .items(languages)
                      .placeholder("Search languages...")
                      .height(10)
                      .run();

    if (result.has_value()) {
        std::cout << "\nYou selected: " << result.value() << "\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    return 0;
}
