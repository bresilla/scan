/// @file test_fuzzy.cpp
/// @brief Tests for the fuzzy matching algorithm

#include <doctest/doctest.h>
#include <scan/util/fuzzy.hpp>

TEST_CASE("fuzzy::match basic matching") {
    SUBCASE("exact match") {
        auto result = scan::fuzzy::match("test", "test");
        CHECK(result.matched);
        CHECK(result.score > 0);
        CHECK(result.positions.size() == 4);
    }

    SUBCASE("substring match") {
        auto result = scan::fuzzy::match("est", "test");
        CHECK(result.matched);
        CHECK(result.positions.size() == 3);
    }

    SUBCASE("fuzzy match") {
        auto result = scan::fuzzy::match("ts", "test");
        CHECK(result.matched);
        CHECK(result.positions.size() == 2);
    }

    SUBCASE("no match") {
        auto result = scan::fuzzy::match("xyz", "test");
        CHECK_FALSE(result.matched);
        CHECK(result.positions.empty());
    }

    SUBCASE("empty pattern matches everything") {
        auto result = scan::fuzzy::match("", "test");
        CHECK(result.matched);
    }

    SUBCASE("pattern longer than target") {
        auto result = scan::fuzzy::match("testing", "test");
        CHECK_FALSE(result.matched);
    }
}

TEST_CASE("fuzzy::match case sensitivity") {
    SUBCASE("case insensitive by default") {
        auto result = scan::fuzzy::match("TEST", "test", false);
        CHECK(result.matched);
    }

    SUBCASE("case sensitive when requested") {
        auto result = scan::fuzzy::match("TEST", "test", true);
        CHECK_FALSE(result.matched);
    }

    SUBCASE("case sensitive exact match") {
        auto result = scan::fuzzy::match("Test", "Test", true);
        CHECK(result.matched);
    }
}

TEST_CASE("fuzzy::filter") {
    std::vector<std::string> items = {"apple", "banana", "apricot", "orange", "grape"};

    SUBCASE("empty query returns all items") {
        auto indices = scan::fuzzy::filter(items, "");
        CHECK(indices.size() == items.size());
    }

    SUBCASE("filter by prefix") {
        auto indices = scan::fuzzy::filter(items, "ap");
        // "ap" matches apple, apricot, and grape (a...p)
        CHECK(indices.size() >= 2);

        // All should contain match
        for (auto idx : indices) {
            auto result = scan::fuzzy::match("ap", items[idx]);
            CHECK(result.matched);
        }
    }

    SUBCASE("filter with no matches") {
        auto indices = scan::fuzzy::filter(items, "xyz");
        CHECK(indices.empty());
    }

    SUBCASE("filter returns sorted by score") {
        auto indices = scan::fuzzy::filter(items, "a");
        CHECK(indices.size() >= 3);  // apple, banana, apricot, orange, grape all have 'a'
    }
}

TEST_CASE("fuzzy::match scoring") {
    SUBCASE("consecutive matches score higher") {
        auto consecutive = scan::fuzzy::match("te", "test");
        auto scattered = scan::fuzzy::match("ts", "test");
        // Consecutive should score higher or equal
        CHECK(consecutive.score >= scattered.score);
    }

    SUBCASE("start of word bonus") {
        auto start = scan::fuzzy::match("t", "test");
        CHECK(start.matched);
        CHECK(start.positions[0] == 0);
    }
}
