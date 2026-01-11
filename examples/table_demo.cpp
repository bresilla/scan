/// @file table_demo.cpp
/// @brief Demonstration of the Table component

#include <scan/scan.hpp>

#include <iostream>

int main() {
    std::cout << "=== Table Demo ===\n\n";

    // Demo 1: Static table display
    std::cout << "Demo 1: Static table display\n\n";

    scan::Table()
        .headers({"Name", "Language", "Stars", "License"})
        .rows({
            {"React", "JavaScript", "218k", "MIT"},
            {"Vue", "JavaScript", "206k", "MIT"},
            {"Angular", "TypeScript", "93k", "MIT"},
            {"Svelte", "JavaScript", "76k", "MIT"},
            {"Solid", "TypeScript", "30k", "MIT"},
        })
        .border(scan::BorderStyle::Rounded)
        .print();

    std::cout << "\n";

    // Demo 2: Interactive table selection
    std::cout << "Demo 2: Interactive table (select a row):\n\n";

    auto selection = scan::Table()
                         .headers({"ID", "Task", "Status", "Priority"})
                         .rows({
                             {"1", "Fix login bug", "Open", "High"},
                             {"2", "Update docs", "In Progress", "Medium"},
                             {"3", "Add dark mode", "Open", "Low"},
                             {"4", "Optimize queries", "Done", "High"},
                             {"5", "Refactor auth", "Open", "Medium"},
                             {"6", "Add tests", "In Progress", "High"},
                             {"7", "Update deps", "Open", "Low"},
                             {"8", "Fix memory leak", "Open", "Critical"},
                         })
                         .height(5)
                         .selectable(true)
                         .border(scan::BorderStyle::Double)
                         .run();

    if (selection.has_value()) {
        std::cout << "\nYou selected row: " << selection.value() << "\n";
    } else {
        std::cout << "\nCancelled.\n";
    }

    // Demo 3: Different border style
    std::cout << "\nDemo 3: Simple border style\n\n";

    scan::Table()
        .headers({"Metric", "Value", "Change"})
        .rows({
            {"CPU Usage", "45%", "+5%"},
            {"Memory", "2.1 GB", "-0.2 GB"},
            {"Disk I/O", "120 MB/s", "+10 MB/s"},
            {"Network", "50 Mbps", "0"},
        })
        .border(scan::BorderStyle::Normal)
        .border_color(100, 150, 200)
        .print();

    return 0;
}
