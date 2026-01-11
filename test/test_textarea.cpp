/// @file test_textarea.cpp
/// @brief Tests for the TextArea component

#include <doctest/doctest.h>
#include <scan/bubbles/textarea.hpp>

TEST_CASE("TextAreaModel initialization") {
    scan::TextAreaModel model;

    CHECK(model.lines.size() == 1);
    CHECK(model.lines[0].empty());
    CHECK(model.cursor_row == 0);
    CHECK(model.cursor_col == 0);
    CHECK(model.width == 60);
    CHECK(model.height == 10);
    CHECK_FALSE(model.submitted);
    CHECK_FALSE(model.cancelled);
}

TEST_CASE("textarea_get_value") {
    scan::TextAreaModel model;
    model.lines = {"Hello", "World", "Test"};

    std::string value = scan::textarea_get_value(model);
    CHECK(value == "Hello\nWorld\nTest");
}

TEST_CASE("textarea_set_value") {
    scan::TextAreaModel model;
    scan::textarea_set_value(model, "Line 1\nLine 2\nLine 3");

    CHECK(model.lines.size() == 3);
    CHECK(model.lines[0] == "Line 1");
    CHECK(model.lines[1] == "Line 2");
    CHECK(model.lines[2] == "Line 3");
}

TEST_CASE("textarea_update character input") {
    scan::TextAreaModel model;
    model.lines = {""};
    model.cursor_row = 0;
    model.cursor_col = 0;

    // Type 'H'
    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Rune;
    key_msg.rune = 'H';
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.lines[0] == "H");
    CHECK(new_model.cursor_col == 1);
}

TEST_CASE("textarea_update enter creates new line") {
    scan::TextAreaModel model;
    model.lines = {"Hello"};
    model.cursor_row = 0;
    model.cursor_col = 5;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Enter;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.lines.size() == 2);
    CHECK(new_model.cursor_row == 1);
    CHECK(new_model.cursor_col == 0);
}

TEST_CASE("textarea_update navigation") {
    scan::TextAreaModel model;
    model.lines = {"Line 1", "Line 2", "Line 3"};
    model.cursor_row = 1;
    model.cursor_col = 3;

    SUBCASE("move up") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Up;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::textarea_update(model, msg);
        CHECK(new_model.cursor_row == 0);
    }

    SUBCASE("move down") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Down;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::textarea_update(model, msg);
        CHECK(new_model.cursor_row == 2);
    }

    SUBCASE("move left") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Left;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::textarea_update(model, msg);
        CHECK(new_model.cursor_col == 2);
    }

    SUBCASE("move right") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Right;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::textarea_update(model, msg);
        CHECK(new_model.cursor_col == 4);
    }
}

TEST_CASE("textarea_update backspace") {
    scan::TextAreaModel model;
    model.lines = {"Hello"};
    model.cursor_row = 0;
    model.cursor_col = 5;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Backspace;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.lines[0] == "Hell");
    CHECK(new_model.cursor_col == 4);
}

TEST_CASE("textarea_update ctrl+d submits") {
    scan::TextAreaModel model;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::CtrlD;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.submitted);
}

TEST_CASE("textarea_update escape submits") {
    scan::TextAreaModel model;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Escape;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.submitted);
}

TEST_CASE("textarea_update ctrl+c cancels") {
    scan::TextAreaModel model;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::CtrlC;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::textarea_update(model, msg);

    CHECK(new_model.cancelled);
}

TEST_CASE("textarea_view rendering") {
    scan::TextAreaModel model;
    model.lines = {"Hello", "World"};
    model.width = 40;
    model.height = 5;

    std::string view = scan::textarea_view(model);

    CHECK_FALSE(view.empty());
}

TEST_CASE("TextArea builder") {
    auto textarea = scan::TextArea()
                        .width(60)
                        .height(8)
                        .placeholder("Enter text...")
                        .show_line_numbers(true);

    auto model = textarea.model();

    CHECK(model.width == 60);
    CHECK(model.height == 8);
    CHECK(model.placeholder == "Enter text...");
    CHECK(model.show_line_numbers);
}
