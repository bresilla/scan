/// @file test_utf8.cpp
/// @brief Tests for UTF-8 utilities

#include <doctest/doctest.h>
#include <scan/util/utf8.hpp>

TEST_CASE("utf8::char_length") {
    // ASCII
    CHECK(scan::utf8::char_length('A') == 1);
    CHECK(scan::utf8::char_length('z') == 1);
    CHECK(scan::utf8::char_length('0') == 1);

    // 2-byte UTF-8 (Latin Extended, etc.)
    CHECK(scan::utf8::char_length(0xC3) == 2);

    // 3-byte UTF-8 (CJK, etc.)
    CHECK(scan::utf8::char_length(0xE4) == 3);

    // 4-byte UTF-8 (Emoji, etc.)
    CHECK(scan::utf8::char_length(0xF0) == 4);
}

TEST_CASE("utf8::length") {
    CHECK(scan::utf8::length("hello") == 5);
    CHECK(scan::utf8::length("") == 0);
    CHECK(scan::utf8::length("a") == 1);
    CHECK(scan::utf8::length("héllo") == 5);
    CHECK(scan::utf8::length("日本語") == 3);
}

TEST_CASE("utf8::decode and encode") {
    SUBCASE("ASCII") {
        auto ascii = scan::utf8::decode("ABC");
        CHECK(ascii.size() == 3);
        CHECK(ascii[0] == 'A');
        CHECK(ascii[1] == 'B');
        CHECK(ascii[2] == 'C');

        std::string encoded = scan::utf8::encode(ascii);
        CHECK(encoded == "ABC");
    }

    SUBCASE("Japanese") {
        auto utf8 = scan::utf8::decode("日本");
        CHECK(utf8.size() == 2);
        CHECK(utf8[0] == 0x65E5);
        CHECK(utf8[1] == 0x672C);

        std::string back = scan::utf8::encode(utf8);
        CHECK(back == "日本");
    }
}

TEST_CASE("utf8::substring") {
    SUBCASE("ASCII") {
        CHECK(scan::utf8::substring("hello", 0, 2) == "he");
        CHECK(scan::utf8::substring("hello", 2, 3) == "llo");
        CHECK(scan::utf8::substring("hello", 3) == "lo");
    }

    SUBCASE("Japanese") {
        CHECK(scan::utf8::substring("日本語", 0, 2) == "日本");
        CHECK(scan::utf8::substring("日本語", 1, 2) == "本語");
        CHECK(scan::utf8::substring("日本語", 2) == "語");
    }
}

TEST_CASE("utf8::insert") {
    CHECK(scan::utf8::insert("hello", 0, "X") == "Xhello");
    CHECK(scan::utf8::insert("hello", 2, "X") == "heXllo");
    CHECK(scan::utf8::insert("hello", 5, "X") == "helloX");
    CHECK(scan::utf8::insert("日語", 1, "本") == "日本語");
}

TEST_CASE("utf8::erase") {
    CHECK(scan::utf8::erase("hello", 0, 1) == "ello");
    CHECK(scan::utf8::erase("hello", 2, 1) == "helo");
    CHECK(scan::utf8::erase("hello", 2, 2) == "heo");
    CHECK(scan::utf8::erase("日本語", 1, 1) == "日語");
    CHECK(scan::utf8::erase("日本語", 0, 2) == "語");
}

TEST_CASE("utf8::display_width") {
    CHECK(scan::utf8::display_width("hello") == 5);
    CHECK(scan::utf8::display_width("日本") == 4);  // CJK = double width
    CHECK(scan::utf8::display_width("A日B") == 4);  // 1 + 2 + 1
}
