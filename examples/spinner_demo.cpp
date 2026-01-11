/// @file spinner_demo.cpp
/// @brief Demonstration of the Spinner component with async tasks

#include <scan/scan.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    std::cout << "=== Spinner Demo ===\n\n";

    // Demo 1: Line spinner (classic)
    std::cout << "Demo 1: Loading with line spinner...\n";

    auto result1 = scan::Spinner()
                       .title("Loading data...")
                       .style(scan::SpinnerStyle::line)
                       .color(100, 200, 255)
                       .run([]() {
                           std::this_thread::sleep_for(std::chrono::seconds(2));
                           return 42;
                       });

    std::cout << "Task completed with result: " << result1 << "\n\n";

    // Demo 2: Bouncing bar
    std::cout << "Demo 2: Processing with bouncing bar...\n";

    auto result2 = scan::Spinner()
                       .title("Processing files...")
                       .style(scan::SpinnerStyle::bouncing_bar)
                       .color(100, 255, 100)
                       .run([]() {
                           std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                           return std::string("success");
                       });

    std::cout << "Processing result: " << result2 << "\n\n";

    // Demo 3: Aesthetic spinner with gradient
    std::cout << "Demo 3: Thinking with aesthetic spinner...\n";

    scan::Spinner()
        .title("Thinking...")
        .style(scan::SpinnerStyle::aesthetic)
        .gradient({"#FF0000", "#FFFF00", "#00FF00"})
        .run([]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return 0;
        });

    std::cout << "Done thinking!\n\n";

    // Demo 4: Point spinner
    std::cout << "Demo 4: Connecting with point spinner...\n";

    scan::Spinner()
        .title("Connecting...")
        .style(scan::SpinnerStyle::point)
        .color("#00FFFF")
        .run([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return 0;
        });

    std::cout << "Connected!\n\n";

    // Demo 5: Bouncing ball
    std::cout << "Demo 5: Computing with bouncing ball...\n";

    scan::Spinner()
        .title("Computing...")
        .style(scan::SpinnerStyle::bouncing_ball)
        .color("#FF00FF")
        .run([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return 0;
        });

    std::cout << "Computation complete!\n";

    return 0;
}
