/// @file test_confirm.cpp
/// @brief Tests for Confirm component

#include <doctest/doctest.h>
#include <scan/bubbles/confirm.hpp>

TEST_CASE("Confirm initial state") {
    scan::ConfirmModel m;
    CHECK(m.value == true);
    CHECK(m.affirmative == "Yes");
    CHECK(m.negative == "No");
    CHECK(m.submitted == false);
    CHECK(m.cancelled == false);
}

TEST_CASE("Confirm toggle with arrow keys") {
    scan::ConfirmModel m;
    m.value = true;

    scan::tea::KeyMsg right;
    right.key = scan::input::Key::Right;

    auto [m1, cmd1] = scan::confirm_update(m, right);
    CHECK(m1.value == false);

    scan::tea::KeyMsg left;
    left.key = scan::input::Key::Left;

    auto [m2, cmd2] = scan::confirm_update(m1, left);
    CHECK(m2.value == true);
}

TEST_CASE("Confirm toggle with tab") {
    scan::ConfirmModel m;
    m.value = true;

    scan::tea::KeyMsg tab;
    tab.key = scan::input::Key::Tab;

    auto [m1, cmd1] = scan::confirm_update(m, tab);
    CHECK(m1.value == false);

    auto [m2, cmd2] = scan::confirm_update(m1, tab);
    CHECK(m2.value == true);
}

TEST_CASE("Confirm y shortcut") {
    scan::ConfirmModel m;
    m.value = false;

    scan::tea::KeyMsg y;
    y.key = scan::input::Key::Rune;
    y.rune = 'y';

    auto [m1, cmd1] = scan::confirm_update(m, y);
    CHECK(m1.value == true);
    CHECK(m1.submitted == true);
}

TEST_CASE("Confirm n shortcut") {
    scan::ConfirmModel m;
    m.value = true;

    scan::tea::KeyMsg n;
    n.key = scan::input::Key::Rune;
    n.rune = 'n';

    auto [m1, cmd1] = scan::confirm_update(m, n);
    CHECK(m1.value == false);
    CHECK(m1.submitted == true);
}

TEST_CASE("Confirm vim h/l keys") {
    scan::ConfirmModel m;

    SUBCASE("h selects yes") {
        m.value = false;
        scan::tea::KeyMsg h;
        h.key = scan::input::Key::Rune;
        h.rune = 'h';

        auto [m1, cmd1] = scan::confirm_update(m, h);
        CHECK(m1.value == true);
        CHECK(m1.submitted == false);
    }

    SUBCASE("l selects no") {
        m.value = true;
        scan::tea::KeyMsg l;
        l.key = scan::input::Key::Rune;
        l.rune = 'l';

        auto [m1, cmd1] = scan::confirm_update(m, l);
        CHECK(m1.value == false);
        CHECK(m1.submitted == false);
    }
}

TEST_CASE("Confirm enter submits") {
    scan::ConfirmModel m;
    m.value = true;

    scan::tea::KeyMsg enter;
    enter.key = scan::input::Key::Enter;

    auto [m1, cmd1] = scan::confirm_update(m, enter);
    CHECK(m1.submitted == true);
    CHECK(m1.value == true);
}

TEST_CASE("Confirm escape cancels") {
    scan::ConfirmModel m;

    scan::tea::KeyMsg escape;
    escape.key = scan::input::Key::Escape;

    auto [m1, cmd1] = scan::confirm_update(m, escape);
    CHECK(m1.cancelled == true);
}

TEST_CASE("Confirm view renders") {
    scan::ConfirmModel m;
    m.prompt = "Continue?";
    m.affirmative = "Yes";
    m.negative = "No";
    m.value = true;

    std::string view = scan::confirm_view(m);
    CHECK(!view.empty());
}
