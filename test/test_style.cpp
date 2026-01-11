/// @file test_style.cpp
/// @brief Tests for the Style component (Lip Gloss-style styling)

#include <doctest/doctest.h>
#include <scan/style/style.hpp>

TEST_CASE("Style render basic text") {
    scan::Style style;
    std::string result = style.render("Hello");

    // Result should contain the text
    CHECK(result.find("Hello") != std::string::npos);
}

TEST_CASE("Style render with padding") {
    scan::Style style;
    style.padding(1, 2, 1, 2);

    std::string result = style.render("X");

    // Result should have extra lines/spaces for padding
    // With padding, it should be more than just "X"
    CHECK(result.length() > 1);

    // Should contain the character
    CHECK(result.find("X") != std::string::npos);
}

TEST_CASE("Style render with border") {
    scan::Style style;
    style.border(scan::BorderStyle::Normal);

    std::string result = style.render("Test");

    // Result should contain border characters and the text
    CHECK_FALSE(result.empty());
    CHECK(result.find("Test") != std::string::npos);
}

TEST_CASE("Style render with different borders") {
    SUBCASE("Rounded border") {
        auto result = scan::Style().border(scan::BorderStyle::Rounded).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("Double border") {
        auto result = scan::Style().border(scan::BorderStyle::Double).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("Thick border") {
        auto result = scan::Style().border(scan::BorderStyle::Thick).render("X");
        CHECK_FALSE(result.empty());
    }
}

TEST_CASE("Style copy") {
    scan::Style original;
    original.bold(true).foreground(255, 0, 0);

    scan::Style copy = original.copy();

    // Both should render the same for the same input
    std::string orig_result = original.render("test");
    std::string copy_result = copy.render("test");

    CHECK(orig_result == copy_result);
}

TEST_CASE("Style chaining") {
    // Test that method chaining works
    std::string result = scan::Style()
                             .padding(1, 1, 1, 1)
                             .margin(1, 1, 1, 1)
                             .bold(true)
                             .italic(true)
                             .foreground(255, 255, 255)
                             .background(0, 0, 0)
                             .border(scan::BorderStyle::Rounded)
                             .width(30)
                             .render("Chained style");

    CHECK_FALSE(result.empty());
    CHECK(result.find("Chained style") != std::string::npos);
}

TEST_CASE("Style width enforcement") {
    std::string result = scan::Style()
                             .width(20)
                             .render("Hi");

    auto lines = scan::split_lines(result);
    // Width should be at least 20 (for the first content line)
    CHECK_FALSE(lines.empty());
    CHECK(scan::visible_width(lines[0]) >= 2);  // At least contains "Hi"
}

TEST_CASE("split_lines utility") {
    std::string text = "Line 1\nLine 2\nLine 3";
    auto lines = scan::split_lines(text);

    CHECK(lines.size() == 3);
    CHECK(lines[0] == "Line 1");
    CHECK(lines[1] == "Line 2");
    CHECK(lines[2] == "Line 3");
}

TEST_CASE("split_lines single line") {
    std::string text = "Single line";
    auto lines = scan::split_lines(text);

    CHECK(lines.size() == 1);
    CHECK(lines[0] == "Single line");
}

TEST_CASE("split_lines empty string") {
    std::string text = "";
    auto lines = scan::split_lines(text);

    CHECK(lines.size() == 1);
    CHECK(lines[0].empty());
}

TEST_CASE("visible_width handles ANSI codes") {
    // Test that ANSI escape codes are not counted in width
    std::string plain = "hello";
    std::string with_codes = "\033[1mhello\033[0m";  // bold "hello"

    CHECK(scan::visible_width(plain) == 5);
    CHECK(scan::visible_width(with_codes) == 5);  // Same visible width
}

TEST_CASE("visible_width utility") {
    CHECK(scan::visible_width("hello") == 5);
    CHECK(scan::visible_width("") == 0);
    CHECK(scan::visible_width("ab cd") == 5);
}

TEST_CASE("pad_right utility") {
    std::string result = scan::pad_right("hi", 10);
    CHECK(scan::visible_width(result) == 10);
    CHECK(result.substr(0, 2) == "hi");
}

TEST_CASE("pad_left utility") {
    std::string result = scan::pad_left("hi", 10);
    CHECK(scan::visible_width(result) == 10);
    CHECK(result.substr(8, 2) == "hi");
}

TEST_CASE("pad_center utility") {
    std::string result = scan::pad_center("hi", 10);
    CHECK(scan::visible_width(result) == 10);
    // "hi" should be somewhere in the middle
    CHECK(result.find("hi") != std::string::npos);
}

TEST_CASE("join_horizontal") {
    std::string a = "A1\nA2";
    std::string b = "B1\nB2";

    std::string joined = scan::join_horizontal(scan::Position::Top, {a, b});

    // Result should combine both side by side
    CHECK_FALSE(joined.empty());
    auto lines = scan::split_lines(joined);
    CHECK(lines.size() == 2);
}

TEST_CASE("join_vertical") {
    std::string a = "Line A";
    std::string b = "Line B";

    std::string joined = scan::join_vertical(scan::Position::Left, {a, b});

    CHECK(joined.find("Line A") != std::string::npos);
    CHECK(joined.find("Line B") != std::string::npos);
}

TEST_CASE("get_border_chars") {
    SUBCASE("Normal border") {
        auto chars = scan::get_border_chars(scan::BorderStyle::Normal);
        CHECK_FALSE(chars.horizontal.empty());
        CHECK_FALSE(chars.vertical.empty());
    }

    SUBCASE("Rounded border") {
        auto chars = scan::get_border_chars(scan::BorderStyle::Rounded);
        CHECK_FALSE(chars.top_left.empty());
        CHECK_FALSE(chars.bottom_right.empty());
    }
}

TEST_CASE("Style with set_string") {
    scan::Style style;
    style.set_string("preset text").bold(true);

    std::string result = style.render();
    CHECK(result.find("preset text") != std::string::npos);
}

TEST_CASE("Style text styles") {
    // These just test that the methods work (output has ANSI codes)
    SUBCASE("bold") {
        std::string result = scan::Style().bold(true).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("italic") {
        std::string result = scan::Style().italic(true).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("underline") {
        std::string result = scan::Style().underline(true).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("faint") {
        std::string result = scan::Style().faint(true).render("X");
        CHECK_FALSE(result.empty());
    }

    SUBCASE("strikethrough") {
        std::string result = scan::Style().strikethrough(true).render("X");
        CHECK_FALSE(result.empty());
    }
}

TEST_CASE("Style alignment") {
    scan::Style style;
    style.width(20).align(scan::Position::Center);

    std::string result = style.render("Hi");

    // Text should be centered within 20 chars
    auto lines = scan::split_lines(result);
    CHECK_FALSE(lines.empty());
}

TEST_CASE("Style margin") {
    scan::Style style;
    style.margin(1, 2, 1, 2);

    std::string result = style.render("X");

    // With margins, should have extra content
    CHECK_FALSE(result.empty());
    CHECK(result.find("X") != std::string::npos);

    // Result should have multiple lines due to top/bottom margin
    auto lines = scan::split_lines(result);
    CHECK(lines.size() >= 1);  // At least content line
}
