/// @file test_viewport.cpp
/// @brief Tests for the Viewport component

#include <doctest/doctest.h>
#include <scan/bubbles/viewport.hpp>

TEST_CASE("ViewportModel initialization") {
    scan::ViewportModel model;

    CHECK(model.lines.empty());
    CHECK(model.width == 80);
    CHECK(model.height == 20);
    CHECK(model.y_offset == 0);
    CHECK(model.x_offset == 0);
    CHECK(model.wrap);
}

TEST_CASE("viewport_set_content") {
    scan::ViewportModel model;
    model.width = 80;
    model.wrap = false;

    scan::viewport_set_content(model, "Line 1\nLine 2\nLine 3");

    CHECK(model.lines.size() == 3);
    CHECK(model.lines[0] == "Line 1");
    CHECK(model.lines[1] == "Line 2");
    CHECK(model.lines[2] == "Line 3");
}

TEST_CASE("viewport_set_content with word wrap") {
    scan::ViewportModel model;
    model.width = 10;
    model.wrap = true;

    std::string long_line = "This is a very long line that should be wrapped";
    scan::viewport_set_content(model, long_line);

    // Should be split into multiple lines
    CHECK(model.lines.size() > 1);
    // Each line should be at most width characters
    for (const auto& line : model.lines) {
        CHECK(line.length() <= 10);
    }
}

TEST_CASE("viewport_set_content empty content") {
    scan::ViewportModel model;
    scan::viewport_set_content(model, "");

    // Should have at least one empty line
    CHECK(model.lines.size() == 1);
    CHECK(model.lines[0].empty());
}

TEST_CASE("viewport_scroll_down") {
    scan::ViewportModel model;
    model.height = 5;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");

    scan::viewport_scroll_down(model, 2);
    CHECK(model.y_offset == 2);

    // Can't scroll past content
    scan::viewport_scroll_down(model, 100);
    CHECK(model.y_offset == 5);  // max_offset = 10 - 5 = 5
}

TEST_CASE("viewport_scroll_up") {
    scan::ViewportModel model;
    model.height = 5;
    model.y_offset = 3;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");

    scan::viewport_scroll_up(model, 2);
    CHECK(model.y_offset == 1);

    // Can't scroll past top
    scan::viewport_scroll_up(model, 100);
    CHECK(model.y_offset == 0);
}

TEST_CASE("viewport_goto_top and viewport_goto_bottom") {
    scan::ViewportModel model;
    model.height = 5;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");
    model.y_offset = 3;

    scan::viewport_goto_top(model);
    CHECK(model.y_offset == 0);

    scan::viewport_goto_bottom(model);
    CHECK(model.y_offset == 5);  // 10 - 5 = 5
}

TEST_CASE("viewport_page_up and viewport_page_down") {
    scan::ViewportModel model;
    model.height = 3;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9");
    model.y_offset = 3;

    scan::viewport_page_up(model);
    CHECK(model.y_offset == 0);

    scan::viewport_page_down(model);
    CHECK(model.y_offset == 3);
}

TEST_CASE("viewport_at_top and viewport_at_bottom") {
    scan::ViewportModel model;
    model.height = 5;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");

    model.y_offset = 0;
    CHECK(scan::viewport_at_top(model));
    CHECK_FALSE(scan::viewport_at_bottom(model));

    model.y_offset = 5;
    CHECK_FALSE(scan::viewport_at_top(model));
    CHECK(scan::viewport_at_bottom(model));
}

TEST_CASE("viewport_scroll_percent") {
    scan::ViewportModel model;
    model.height = 5;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");

    model.y_offset = 0;
    CHECK(scan::viewport_scroll_percent(model) == 0);

    model.y_offset = 5;
    CHECK(scan::viewport_scroll_percent(model) == 100);

    // Content fits in viewport
    scan::ViewportModel small_model;
    small_model.height = 10;
    scan::viewport_set_content(small_model, "1\n2\n3");
    CHECK(scan::viewport_scroll_percent(small_model) == 100);
}

TEST_CASE("viewport_update navigation keys") {
    scan::ViewportModel model;
    model.height = 5;
    scan::viewport_set_content(model, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");

    SUBCASE("down arrow scrolls down") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Down;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::viewport_update(model, msg);
        CHECK(new_model.y_offset == 1);
    }

    SUBCASE("up arrow scrolls up") {
        model.y_offset = 2;
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Up;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::viewport_update(model, msg);
        CHECK(new_model.y_offset == 1);
    }

    SUBCASE("vim j scrolls down") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Rune;
        key_msg.rune = 'j';
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::viewport_update(model, msg);
        CHECK(new_model.y_offset == 1);
    }

    SUBCASE("vim g goes to top") {
        model.y_offset = 3;
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Rune;
        key_msg.rune = 'g';
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::viewport_update(model, msg);
        CHECK(new_model.y_offset == 0);
    }

    SUBCASE("vim G goes to bottom") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Rune;
        key_msg.rune = 'G';
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::viewport_update(model, msg);
        CHECK(new_model.y_offset == 5);
    }
}

TEST_CASE("viewport_view rendering") {
    scan::ViewportModel model;
    model.width = 40;
    model.height = 3;
    scan::viewport_set_content(model, "Line 1\nLine 2\nLine 3\nLine 4\nLine 5");

    std::string view = scan::viewport_view(model);

    CHECK_FALSE(view.empty());
    // Should contain first 3 lines
    CHECK(view.find("Line 1") != std::string::npos);
    CHECK(view.find("Line 3") != std::string::npos);
}

TEST_CASE("Viewport builder") {
    auto viewport = scan::Viewport()
                        .content("Test content\nLine 2")
                        .width(60)
                        .height(10)
                        .wrap(false);

    CHECK(viewport.model().width == 60);
    CHECK(viewport.model().height == 10);
    CHECK_FALSE(viewport.model().wrap);
    CHECK(viewport.model().lines.size() == 2);
}

TEST_CASE("Viewport builder methods") {
    auto viewport = scan::Viewport()
                        .content("1\n2\n3\n4\n5\n6\n7\n8\n9\n10")
                        .height(3);

    CHECK(viewport.at_top());
    CHECK_FALSE(viewport.at_bottom());

    viewport.scroll_down(2);
    CHECK_FALSE(viewport.at_top());

    viewport.goto_bottom();
    CHECK(viewport.at_bottom());

    viewport.goto_top();
    CHECK(viewport.at_top());
    CHECK(viewport.scroll_percent() == 0);
}
