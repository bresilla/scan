/// @file test_list.cpp
/// @brief Tests for List component

#include <doctest/doctest.h>
#include <scan/bubbles/list.hpp>

TEST_CASE("List initial state") {
    scan::ListModel m;
    CHECK(m.items.empty());
    CHECK(m.selected.empty());
    CHECK(m.cursor == 0);
    CHECK(m.offset == 0);
    CHECK(m.limit == 1);  // Single select by default
    CHECK(m.submitted == false);
    CHECK(m.cancelled == false);
}

TEST_CASE("List cursor movement down") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E"};
    m.cursor = 0;

    scan::tea::KeyMsg down;
    down.key = scan::input::Key::Down;

    auto [m1, cmd1] = scan::list_update(m, down);
    CHECK(m1.cursor == 1);

    auto [m2, cmd2] = scan::list_update(m1, down);
    CHECK(m2.cursor == 2);
}

TEST_CASE("List cursor movement up") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E"};
    m.cursor = 3;

    scan::tea::KeyMsg up;
    up.key = scan::input::Key::Up;

    auto [m1, cmd1] = scan::list_update(m, up);
    CHECK(m1.cursor == 2);

    auto [m2, cmd2] = scan::list_update(m1, up);
    CHECK(m2.cursor == 1);
}

TEST_CASE("List cursor bounds") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};
    m.cursor = 0;

    SUBCASE("cannot go above top") {
        scan::tea::KeyMsg up;
        up.key = scan::input::Key::Up;

        auto [m1, cmd1] = scan::list_update(m, up);
        CHECK(m1.cursor == 0);
    }

    SUBCASE("cannot go below bottom") {
        m.cursor = 2;
        scan::tea::KeyMsg down;
        down.key = scan::input::Key::Down;

        auto [m1, cmd1] = scan::list_update(m, down);
        CHECK(m1.cursor == 2);
    }
}

TEST_CASE("List vim j/k keys") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D"};
    m.cursor = 0;

    SUBCASE("j moves down") {
        scan::tea::KeyMsg j;
        j.key = scan::input::Key::Rune;
        j.rune = 'j';

        auto [m1, cmd1] = scan::list_update(m, j);
        CHECK(m1.cursor == 1);
    }

    SUBCASE("k moves up") {
        m.cursor = 2;
        scan::tea::KeyMsg k;
        k.key = scan::input::Key::Rune;
        k.rune = 'k';

        auto [m1, cmd1] = scan::list_update(m, k);
        CHECK(m1.cursor == 1);
    }
}

TEST_CASE("List home/end keys") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E"};
    m.cursor = 2;

    SUBCASE("home goes to start") {
        scan::tea::KeyMsg home;
        home.key = scan::input::Key::Home;

        auto [m1, cmd1] = scan::list_update(m, home);
        CHECK(m1.cursor == 0);
    }

    SUBCASE("end goes to end") {
        scan::tea::KeyMsg end;
        end.key = scan::input::Key::End;

        auto [m1, cmd1] = scan::list_update(m, end);
        CHECK(m1.cursor == 4);
    }
}

TEST_CASE("List g/G keys") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E"};
    m.cursor = 2;

    SUBCASE("g goes to top") {
        scan::tea::KeyMsg g;
        g.key = scan::input::Key::Rune;
        g.rune = 'g';

        auto [m1, cmd1] = scan::list_update(m, g);
        CHECK(m1.cursor == 0);
    }

    SUBCASE("G goes to bottom") {
        scan::tea::KeyMsg G;
        G.key = scan::input::Key::Rune;
        G.rune = 'G';

        auto [m1, cmd1] = scan::list_update(m, G);
        CHECK(m1.cursor == 4);
    }
}

TEST_CASE("List single select on enter") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};
    m.cursor = 1;
    m.limit = 1;  // Single select

    scan::tea::KeyMsg enter;
    enter.key = scan::input::Key::Enter;

    auto [m1, cmd1] = scan::list_update(m, enter);
    CHECK(m1.submitted == true);
    CHECK(m1.selected.count(1) == 1);  // B should be selected
}

TEST_CASE("List multi select with space") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D"};
    m.cursor = 0;
    m.limit = 0;  // Unlimited

    scan::tea::KeyMsg space;
    space.key = scan::input::Key::Space;

    // Select A
    auto [m1, cmd1] = scan::list_update(m, space);
    CHECK(m1.selected.count(0) == 1);

    // Move down and select B
    m1.cursor = 1;
    auto [m2, cmd2] = scan::list_update(m1, space);
    CHECK(m2.selected.count(0) == 1);
    CHECK(m2.selected.count(1) == 1);

    // Move down and select C
    m2.cursor = 2;
    auto [m3, cmd3] = scan::list_update(m2, space);
    CHECK(m3.selected.size() == 3);
}

TEST_CASE("List deselect with space") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};
    m.cursor = 0;
    m.limit = 0;
    m.selected.insert(0);  // A is already selected

    scan::tea::KeyMsg space;
    space.key = scan::input::Key::Space;

    // Deselect A
    auto [m1, cmd1] = scan::list_update(m, space);
    CHECK(m1.selected.count(0) == 0);
}

TEST_CASE("List selection limit") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D"};
    m.cursor = 0;
    m.limit = 2;  // Max 2 selections

    scan::tea::KeyMsg space;
    space.key = scan::input::Key::Space;

    // Select A
    auto [m1, cmd1] = scan::list_update(m, space);
    CHECK(m1.selected.size() == 1);

    // Select B
    m1.cursor = 1;
    auto [m2, cmd2] = scan::list_update(m1, space);
    CHECK(m2.selected.size() == 2);

    // Try to select C - should be ignored (at limit)
    m2.cursor = 2;
    auto [m3, cmd3] = scan::list_update(m2, space);
    CHECK(m3.selected.size() == 2);
    CHECK(m3.selected.count(2) == 0);
}

TEST_CASE("List toggle all") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};
    m.cursor = 0;
    m.limit = 0;  // Unlimited

    // Press 'a' to toggle all
    scan::tea::KeyMsg a;
    a.key = scan::input::Key::Rune;
    a.rune = 'a';

    auto [m1, cmd1] = scan::list_update(m, a);
    CHECK(m1.selected.size() == 3);

    // Press 'a' again to toggle all off
    auto [m2, cmd2] = scan::list_update(m1, a);
    CHECK(m2.selected.empty());
}

TEST_CASE("List escape cancels") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};

    scan::tea::KeyMsg escape;
    escape.key = scan::input::Key::Escape;

    auto [m1, cmd1] = scan::list_update(m, escape);
    CHECK(m1.cancelled == true);
}

TEST_CASE("List scrolling down") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    m.height = 3;  // Only show 3 items
    m.cursor = 0;
    m.offset = 0;

    scan::tea::KeyMsg down;
    down.key = scan::input::Key::Down;

    // Move down - should stay in viewport
    auto [m1, cmd1] = scan::list_update(m, down);
    CHECK(m1.cursor == 1);
    CHECK(m1.offset == 0);

    auto [m2, cmd2] = scan::list_update(m1, down);
    CHECK(m2.cursor == 2);
    CHECK(m2.offset == 0);

    // Move down again - should scroll
    auto [m3, cmd3] = scan::list_update(m2, down);
    CHECK(m3.cursor == 3);
    CHECK(m3.offset == 1);
}

TEST_CASE("List scrolling up") {
    scan::ListModel m;
    m.items = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    m.height = 3;
    m.cursor = 5;
    m.offset = 3;  // Showing D, E, F

    scan::tea::KeyMsg up;
    up.key = scan::input::Key::Up;

    // Move up - should stay in viewport
    auto [m1, cmd1] = scan::list_update(m, up);
    CHECK(m1.cursor == 4);
    CHECK(m1.offset == 3);

    auto [m2, cmd2] = scan::list_update(m1, up);
    CHECK(m2.cursor == 3);
    CHECK(m2.offset == 3);

    // Move up again - should scroll
    auto [m3, cmd3] = scan::list_update(m2, up);
    CHECK(m3.cursor == 2);
    CHECK(m3.offset == 2);
}

TEST_CASE("List view empty") {
    scan::ListModel m;
    m.items = {};

    std::string view = scan::list_view(m);
    CHECK(view.find("(no items)") != std::string::npos);
}

TEST_CASE("List view with items") {
    scan::ListModel m;
    m.items = {"A", "B", "C"};
    m.cursor = 1;

    std::string view = scan::list_view(m);
    CHECK(!view.empty());
}
