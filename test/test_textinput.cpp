/// @file test_textinput.cpp
/// @brief Tests for TextInput component

#include <doctest/doctest.h>
#include <scan/bubbles/textinput.hpp>

TEST_CASE("TextInput initial state") {
    scan::TextInputModel m;
    CHECK(m.value.empty());
    CHECK(m.cursor == 0);
    CHECK(m.focused == true);
    CHECK(m.password == false);
    CHECK(m.submitted == false);
    CHECK(m.cancelled == false);
}

TEST_CASE("TextInput type characters") {
    scan::TextInputModel m;
    m.focused = true;

    scan::tea::KeyMsg key_a;
    key_a.key = scan::input::Key::Rune;
    key_a.rune = 'a';

    auto [m1, cmd1] = scan::textinput_update(m, key_a);
    CHECK(m1.value == "a");
    CHECK(m1.cursor == 1);

    scan::tea::KeyMsg key_b;
    key_b.key = scan::input::Key::Rune;
    key_b.rune = 'b';

    auto [m2, cmd2] = scan::textinput_update(m1, key_b);
    CHECK(m2.value == "ab");
    CHECK(m2.cursor == 2);
}

TEST_CASE("TextInput backspace") {
    scan::TextInputModel m;
    m.value = "abc";
    m.cursor = 3;

    scan::tea::KeyMsg backspace;
    backspace.key = scan::input::Key::Backspace;

    auto [m1, cmd1] = scan::textinput_update(m, backspace);
    CHECK(m1.value == "ab");
    CHECK(m1.cursor == 2);

    auto [m2, cmd2] = scan::textinput_update(m1, backspace);
    CHECK(m2.value == "a");
    CHECK(m2.cursor == 1);
}

TEST_CASE("TextInput cursor movement") {
    scan::TextInputModel m;
    m.value = "hello";
    m.cursor = 5;

    SUBCASE("Left arrow") {
        scan::tea::KeyMsg left;
        left.key = scan::input::Key::Left;

        auto [m1, cmd1] = scan::textinput_update(m, left);
        CHECK(m1.cursor == 4);
    }

    SUBCASE("Right arrow") {
        m.cursor = 2;
        scan::tea::KeyMsg right;
        right.key = scan::input::Key::Right;

        auto [m1, cmd1] = scan::textinput_update(m, right);
        CHECK(m1.cursor == 3);
    }

    SUBCASE("Home") {
        scan::tea::KeyMsg home;
        home.key = scan::input::Key::Home;

        auto [m1, cmd1] = scan::textinput_update(m, home);
        CHECK(m1.cursor == 0);
    }

    SUBCASE("End") {
        m.cursor = 0;
        scan::tea::KeyMsg end;
        end.key = scan::input::Key::End;

        auto [m1, cmd1] = scan::textinput_update(m, end);
        CHECK(m1.cursor == 5);
    }
}

TEST_CASE("TextInput delete") {
    scan::TextInputModel m;
    m.value = "hello";
    m.cursor = 2;

    scan::tea::KeyMsg del;
    del.key = scan::input::Key::Delete;

    auto [m1, cmd1] = scan::textinput_update(m, del);
    CHECK(m1.value == "helo");
    CHECK(m1.cursor == 2);
}

TEST_CASE("TextInput Ctrl+K kill to end") {
    scan::TextInputModel m;
    m.value = "hello world";
    m.cursor = 5;

    scan::tea::KeyMsg ctrl_k;
    ctrl_k.key = scan::input::Key::CtrlK;

    auto [m1, cmd1] = scan::textinput_update(m, ctrl_k);
    CHECK(m1.value == "hello");
    CHECK(m1.cursor == 5);
}

TEST_CASE("TextInput Ctrl+U kill to start") {
    scan::TextInputModel m;
    m.value = "hello world";
    m.cursor = 6;

    scan::tea::KeyMsg ctrl_u;
    ctrl_u.key = scan::input::Key::CtrlU;

    auto [m1, cmd1] = scan::textinput_update(m, ctrl_u);
    CHECK(m1.value == "world");
    CHECK(m1.cursor == 0);
}

TEST_CASE("TextInput char limit") {
    scan::TextInputModel m;
    m.char_limit = 5;

    scan::tea::KeyMsg key;
    key.key = scan::input::Key::Rune;
    key.rune = 'a';

    auto model = m;
    for (int i = 0; i < 5; i++) {
        auto [new_model, cmd] = scan::textinput_update(model, key);
        model = new_model;
    }
    CHECK(model.value == "aaaaa");

    auto [m6, cmd6] = scan::textinput_update(model, key);
    CHECK(m6.value == "aaaaa");  // No change - at limit
}

TEST_CASE("TextInput submit") {
    scan::TextInputModel m;
    m.value = "hello";

    scan::tea::KeyMsg enter;
    enter.key = scan::input::Key::Enter;

    auto [m1, cmd1] = scan::textinput_update(m, enter);
    CHECK(m1.submitted == true);
}

TEST_CASE("TextInput cancel") {
    scan::TextInputModel m;
    m.value = "hello";

    scan::tea::KeyMsg escape;
    escape.key = scan::input::Key::Escape;

    auto [m1, cmd1] = scan::textinput_update(m, escape);
    CHECK(m1.cancelled == true);
}

TEST_CASE("TextInput unfocused ignores input") {
    scan::TextInputModel m;
    m.value = "hello";
    m.cursor = 5;
    m.focused = false;

    scan::tea::KeyMsg key;
    key.key = scan::input::Key::Rune;
    key.rune = 'a';

    auto [m1, cmd1] = scan::textinput_update(m, key);
    CHECK(m1.value == "hello");
    CHECK(m1.cursor == 5);
}

TEST_CASE("TextInput view renders") {
    scan::TextInputModel m;
    m.prompt = "> ";
    m.value = "hello";
    m.cursor = 5;

    std::string view = scan::textinput_view(m);
    CHECK(!view.empty());
}
