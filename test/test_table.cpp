/// @file test_table.cpp
/// @brief Tests for the Table component

#include <doctest/doctest.h>
#include <scan/bubbles/table.hpp>

TEST_CASE("TableModel initialization") {
    scan::TableModel model;

    CHECK(model.headers.empty());
    CHECK(model.rows.empty());
    CHECK(model.cursor == 0);
    CHECK(model.height == 10);
    CHECK_FALSE(model.selectable);
    CHECK_FALSE(model.submitted);
    CHECK_FALSE(model.cancelled);
}

TEST_CASE("calculate_widths") {
    scan::TableModel model;
    model.headers = {"Name", "Age", "City"};
    model.rows = {
        {"Alice", "25", "New York"},
        {"Bob", "30", "Los Angeles"},
        {"Charlie", "35", "Chicago"}
    };

    auto widths = scan::calculate_widths(model);

    CHECK(widths.size() == 3);
    CHECK(widths[0] >= 7);   // "Charlie" is longest
    CHECK(widths[1] >= 3);   // "Age" or "25"
    CHECK(widths[2] >= 11);  // "Los Angeles" is longest
}

TEST_CASE("calculate_widths with fixed widths") {
    scan::TableModel model;
    model.headers = {"Name", "Value"};
    model.rows = {{"test", "123"}};
    model.widths = {20, 0};  // First column fixed, second auto

    auto widths = scan::calculate_widths(model);

    CHECK(widths[0] == 20);  // Fixed width
    CHECK(widths[1] >= 5);   // Auto width
}

TEST_CASE("table_update navigation") {
    scan::TableModel model;
    model.headers = {"Col"};
    model.rows = {{"Row 1"}, {"Row 2"}, {"Row 3"}};
    model.selectable = true;
    model.cursor = 0;

    SUBCASE("move down") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Down;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.cursor == 1);
    }

    SUBCASE("move up at top stays at top") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Up;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.cursor == 0);
    }

    SUBCASE("vim j navigation") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Rune;
        key_msg.rune = 'j';
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.cursor == 1);
    }

    SUBCASE("vim k navigation") {
        model.cursor = 1;
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Rune;
        key_msg.rune = 'k';
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.cursor == 0);
    }
}

TEST_CASE("table_update submit and cancel") {
    scan::TableModel model;
    model.headers = {"Col"};
    model.rows = {{"Row 1"}};
    model.selectable = true;

    SUBCASE("enter submits") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Enter;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.submitted);
    }

    SUBCASE("escape cancels") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Escape;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::table_update(model, msg);
        CHECK(new_model.cancelled);
    }
}

TEST_CASE("table_update non-selectable ignores input") {
    scan::TableModel model;
    model.headers = {"Col"};
    model.rows = {{"Row 1"}, {"Row 2"}};
    model.selectable = false;
    model.cursor = 0;

    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Down;
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::table_update(model, msg);
    CHECK(new_model.cursor == 0);  // Cursor unchanged
}

TEST_CASE("table_view rendering") {
    scan::TableModel model;
    model.headers = {"Name", "Value"};
    model.rows = {{"foo", "123"}, {"bar", "456"}};

    std::string view = scan::table_view(model);

    CHECK_FALSE(view.empty());
    // Should contain border characters or header text
}

TEST_CASE("table_view empty table") {
    scan::TableModel model;
    model.headers = {"Col1", "Col2"};
    // No rows

    std::string view = scan::table_view(model);

    CHECK_FALSE(view.empty());
    // Should show "(no data)" or similar
}

TEST_CASE("table_view no headers") {
    scan::TableModel model;
    // No headers

    std::string view = scan::table_view(model);

    CHECK(view.find("no columns") != std::string::npos);
}

TEST_CASE("Table builder") {
    auto table = scan::Table()
                     .headers({"A", "B", "C"})
                     .rows({{"1", "2", "3"}, {"4", "5", "6"}})
                     .height(5)
                     .selectable(true)
                     .border(scan::BorderStyle::Double);

    auto model = table.model();

    CHECK(model.headers.size() == 3);
    CHECK(model.rows.size() == 2);
    CHECK(model.height == 5);
    CHECK(model.selectable);
    CHECK(model.border_style == scan::BorderStyle::Double);
}

TEST_CASE("Table render method") {
    auto table = scan::Table()
                     .headers({"X", "Y"})
                     .rows({{"1", "2"}});

    std::string rendered = table.render();
    CHECK_FALSE(rendered.empty());
}
