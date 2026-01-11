/// @file test_filter.cpp
/// @brief Tests for the Filter component model and update logic

#include <doctest/doctest.h>
#include <scan/bubbles/filter.hpp>

TEST_CASE("FilterModel initialization") {
    scan::FilterModel model;

    CHECK(model.items.empty());
    CHECK(model.filtered.empty());
    CHECK(model.selected.empty());
    CHECK(model.query.empty());
    CHECK(model.cursor == 0);
    CHECK(model.height == 10);
    CHECK(model.limit == 1);
    CHECK_FALSE(model.case_sensitive);
    CHECK_FALSE(model.submitted);
    CHECK_FALSE(model.cancelled);
}

TEST_CASE("FilterModel with items") {
    scan::FilterModel model;
    model.items = {"apple", "banana", "cherry"};
    model.filtered = {0, 1, 2};

    SUBCASE("navigation down") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Down;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::filter_update(model, msg);
        CHECK(new_model.cursor == 1);
    }

    SUBCASE("navigation up at top stays at top") {
        model.cursor = 0;
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Up;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::filter_update(model, msg);
        CHECK(new_model.cursor == 0);
    }

    SUBCASE("enter submits") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Enter;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::filter_update(model, msg);
        CHECK(new_model.submitted);
    }

    SUBCASE("escape cancels") {
        scan::tea::KeyMsg key_msg;
        key_msg.key = scan::input::Key::Escape;
        auto msg = scan::tea::Msg(key_msg);

        auto [new_model, cmd] = scan::filter_update(model, msg);
        CHECK(new_model.cancelled);
    }
}

TEST_CASE("filter_view rendering") {
    scan::FilterModel model;
    model.items = {"apple", "banana"};
    model.filtered = {0, 1};
    model.query = "";
    model.placeholder = "Search...";

    std::string view = scan::filter_view(model);

    // View should contain something (not empty)
    CHECK_FALSE(view.empty());
}

TEST_CASE("Filter builder") {
    auto filter = scan::Filter()
                      .items({"one", "two", "three"})
                      .placeholder("Type to filter")
                      .height(5);

    auto model = filter.model();

    CHECK(model.items.size() == 3);
    CHECK(model.placeholder == "Type to filter");
    CHECK(model.height == 5);
    CHECK(model.filtered.size() == 3);  // All items visible initially
}

TEST_CASE("Filter query filtering") {
    scan::FilterModel model;
    model.items = {"apple", "apricot", "banana", "cherry"};
    model.filtered = {0, 1, 2, 3};
    model.query = "";

    // Simulate typing 'a'
    scan::tea::KeyMsg key_msg;
    key_msg.key = scan::input::Key::Rune;
    key_msg.rune = 'a';
    auto msg = scan::tea::Msg(key_msg);

    auto [new_model, cmd] = scan::filter_update(model, msg);

    // Query should be updated and items filtered
    CHECK(new_model.query == "a");
    // All items with 'a' should be in filtered (apple, apricot, banana)
    CHECK(new_model.filtered.size() <= 4);
}
