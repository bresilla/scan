/// @file style_demo.cpp
/// @brief Demonstration of the Style class (Lip Gloss-style text styling)

#include <scan/scan.hpp>

#include <iostream>

int main() {
    std::cout << "=== Style Demo ===\n\n";

    // Demo 1: Basic styling
    std::cout << "Demo 1: Basic text styling\n";
    std::cout << "--------------------------\n";

    auto bold_text = scan::Style()
                         .bold(true)
                         .foreground(255, 200, 100)
                         .render("Bold orange text");
    std::cout << bold_text << "\n";

    auto italic_text = scan::Style()
                           .italic(true)
                           .foreground(100, 200, 255)
                           .render("Italic blue text");
    std::cout << italic_text << "\n";

    auto underline_text = scan::Style()
                              .underline(true)
                              .foreground(200, 100, 255)
                              .render("Underlined purple text");
    std::cout << underline_text << "\n\n";

    // Demo 2: Box styling with padding and borders
    std::cout << "Demo 2: Box with padding and border\n";
    std::cout << "------------------------------------\n";

    auto box = scan::Style()
                   .padding(1, 2, 1, 2)
                   .border(scan::BorderStyle::Rounded)
                   .border_foreground(100, 200, 100)
                   .foreground(255, 255, 255)
                   .width(40)
                   .render("Hello from a styled box!\nThis has padding and a rounded border.");
    std::cout << box << "\n\n";

    // Demo 3: Different border styles
    std::cout << "Demo 3: Different border styles\n";
    std::cout << "--------------------------------\n\n";

    auto normal_box = scan::Style()
                          .border(scan::BorderStyle::Normal)
                          .border_foreground(200, 200, 200)
                          .padding(0, 1, 0, 1)
                          .render("Normal border");
    std::cout << normal_box << "\n\n";

    auto double_box = scan::Style()
                          .border(scan::BorderStyle::Double)
                          .border_foreground(255, 200, 100)
                          .padding(0, 1, 0, 1)
                          .render("Double border");
    std::cout << double_box << "\n\n";

    auto thick_box = scan::Style()
                         .border(scan::BorderStyle::Thick)
                         .border_foreground(100, 200, 255)
                         .padding(0, 1, 0, 1)
                         .render("Thick border");
    std::cout << thick_box << "\n\n";

    // Demo 4: Background colors
    std::cout << "Demo 4: Background colors\n";
    std::cout << "-------------------------\n";

    auto with_bg = scan::Style()
                       .foreground(255, 255, 255)
                       .background(100, 50, 150)
                       .padding(1, 2, 1, 2)
                       .bold(true)
                       .render("White text on purple background");
    std::cout << with_bg << "\n\n";

    // Demo 5: Joining boxes horizontally
    std::cout << "Demo 5: Horizontal box layout\n";
    std::cout << "-----------------------------\n";

    auto box1 = scan::Style()
                    .border(scan::BorderStyle::Rounded)
                    .border_foreground(255, 100, 100)
                    .padding(0, 1, 0, 1)
                    .width(15)
                    .render("Box 1");

    auto box2 = scan::Style()
                    .border(scan::BorderStyle::Rounded)
                    .border_foreground(100, 255, 100)
                    .padding(0, 1, 0, 1)
                    .width(15)
                    .render("Box 2");

    auto box3 = scan::Style()
                    .border(scan::BorderStyle::Rounded)
                    .border_foreground(100, 100, 255)
                    .padding(0, 1, 0, 1)
                    .width(15)
                    .render("Box 3");

    std::cout << scan::join_horizontal(scan::Position::Top, {box1, box2, box3}) << "\n\n";

    // Demo 6: Margins
    std::cout << "Demo 6: Text with margins\n";
    std::cout << "-------------------------\n";

    auto with_margin = scan::Style()
                           .margin(1, 4, 1, 4)
                           .foreground(200, 200, 100)
                           .render("This text has margins around it");
    std::cout << with_margin << "\n";

    return 0;
}
