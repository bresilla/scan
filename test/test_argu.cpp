/// @file test_argue.cpp
/// @brief Comprehensive tests for the Argue argument parsing library

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <argu/argu.hpp>
#include <vector>
#include <string>

// Helper to create args from string literals
std::vector<std::string> make_args(std::initializer_list<const char*> args) {
    std::vector<std::string> result;
    result.reserve(args.size());
    for (const char* arg : args) {
        result.emplace_back(arg);
    }
    return result;
}

// =============================================================================
// Basic Parsing Tests
// =============================================================================

TEST_SUITE("Argue Basic Parsing") {

    TEST_CASE("Parse positional argument") {
        std::string name;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name").positional().required().value_of(name));

        auto result = cmd.parse(make_args({"World"}));
        CHECK(result.success());
        CHECK(name == "World");
    }

    TEST_CASE("Parse short option") {
        int count = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("count").short_name('c').value_of(count));

        auto result = cmd.parse(make_args({"-c", "42"}));
        CHECK(result.success());
        CHECK(count == 42);
    }

    TEST_CASE("Parse long option") {
        int count = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("count").long_name("count").value_of(count));

        auto result = cmd.parse(make_args({"--count", "42"}));
        CHECK(result.success());
        CHECK(count == 42);
    }

    TEST_CASE("Parse long option with equals syntax") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("option").value_of(value));

        auto result = cmd.parse(make_args({"--option=hello"}));
        CHECK(result.success());
        CHECK(value == "hello");
    }

    TEST_CASE("Parse combined short options") {
        bool a = false, b = false, c = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("a").short_name('a').flag(a))
            .arg(argu::Arg("b").short_name('b').flag(b))
            .arg(argu::Arg("c").short_name('c').flag(c));

        auto result = cmd.parse(make_args({"-abc"}));
        CHECK(result.success());
        CHECK(a);
        CHECK(b);
        CHECK(c);
    }

    TEST_CASE("Parse short option with attached value") {
        int num = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("num").short_name('n').value_of(num));

        auto result = cmd.parse(make_args({"-n42"}));
        CHECK(result.success());
        CHECK(num == 42);
    }

    TEST_CASE("Parse flag") {
        bool verbose = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose").short_name('v').long_name("verbose").flag(verbose));

        SUBCASE("Flag not present") {
            auto result = cmd.parse(make_args({}));
            CHECK(result.success());
            CHECK_FALSE(verbose);
        }

        SUBCASE("Short flag present") {
            auto result = cmd.parse(make_args({"-v"}));
            CHECK(result.success());
            CHECK(verbose);
        }

        SUBCASE("Long flag present") {
            verbose = false;
            auto result = cmd.parse(make_args({"--verbose"}));
            CHECK(result.success());
            CHECK(verbose);
        }
    }

    TEST_CASE("Parse counting flag") {
        int verbosity = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose").short_name('v').count(verbosity));

        auto result = cmd.parse(make_args({"-v", "-v", "-v"}));
        CHECK(result.success());
        CHECK(verbosity == 3);
    }

    TEST_CASE("Parse multiple values into vector") {
        std::vector<std::string> files;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("files").short_name('f').value_of(files).takes_multiple());

        auto result = cmd.parse(make_args({"-f", "a.txt", "-f", "b.txt", "-f", "c.txt"}));
        CHECK(result.success());
        REQUIRE(files.size() == 3);
        CHECK(files[0] == "a.txt");
        CHECK(files[1] == "b.txt");
        CHECK(files[2] == "c.txt");
    }

    TEST_CASE("Parse with -- separator") {
        std::string name;
        std::vector<std::string> rest;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name").positional().value_of(name))
            .arg(argu::Arg("rest").positional().value_of(rest).takes_multiple());

        auto result = cmd.parse(make_args({"first", "--", "-v", "--help"}));
        CHECK(result.success());
        CHECK(name == "first");
        REQUIRE(rest.size() == 2);
        CHECK(rest[0] == "-v");
        CHECK(rest[1] == "--help");
    }
}

// =============================================================================
// Default Values Tests
// =============================================================================

TEST_SUITE("Argue Default Values") {

    TEST_CASE("Apply default string value") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("opt").value_of(value).default_value("default"));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == "default");
    }

    TEST_CASE("Apply default int value") {
        int value = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("num").long_name("num").value_of(value).default_value("42"));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == 42);
    }

    TEST_CASE("Override default with provided value") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("opt").value_of(value).default_value("default"));

        auto result = cmd.parse(make_args({"--opt", "custom"}));
        CHECK(result.success());
        CHECK(value == "custom");
    }
}

// =============================================================================
// Required Arguments Tests
// =============================================================================

TEST_SUITE("Argue Required Arguments") {

    TEST_CASE("Missing required positional argument") {
        std::string name;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name").positional().required().value_of(name));

        auto result = cmd.parse(make_args({}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("Missing required") != std::string::npos);
    }

    TEST_CASE("Provided required argument succeeds") {
        std::string name;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name").positional().required().value_of(name));

        auto result = cmd.parse(make_args({"value"}));
        CHECK(result.success());
    }
}

// =============================================================================
// Choices Tests
// =============================================================================

TEST_SUITE("Argue Choices") {

    TEST_CASE("Valid choice accepted") {
        std::string level;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("level")
                .long_name("level")
                .value_of(level)
                .choices({"debug", "info", "warn", "error"}));

        auto result = cmd.parse(make_args({"--level", "info"}));
        CHECK(result.success());
        CHECK(level == "info");
    }

    TEST_CASE("Invalid choice rejected") {
        std::string level;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("level")
                .long_name("level")
                .value_of(level)
                .choices({"debug", "info", "warn", "error"}));

        auto result = cmd.parse(make_args({"--level", "invalid"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("valid values") != std::string::npos);
    }
}

// =============================================================================
// Validators Tests
// =============================================================================

TEST_SUITE("Argue Validators") {

    TEST_CASE("Range validator accepts valid value") {
        int port = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("port")
                .long_name("port")
                .value_of(port)
                .validate(argu::validators::range(1, 65535)));

        auto result = cmd.parse(make_args({"--port", "8080"}));
        CHECK(result.success());
        CHECK(port == 8080);
    }

    TEST_CASE("Range validator rejects out of range value") {
        int port = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("port")
                .long_name("port")
                .value_of(port)
                .validate(argu::validators::range(1, 65535)));

        auto result = cmd.parse(make_args({"--port", "70000"}));
        CHECK_FALSE(result.success());
    }

    TEST_CASE("Positive validator") {
        int num = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("num")
                .long_name("num")
                .value_of(num)
                .validate(argu::validators::positive<int>()));

        SUBCASE("Accepts positive") {
            auto result = cmd.parse(make_args({"--num", "5"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects zero") {
            auto result = cmd.parse(make_args({"--num", "0"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Rejects negative") {
            auto result = cmd.parse(make_args({"--num", "-1"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("Non-empty validator") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(value)
                .validate(argu::validators::non_empty()));

        SUBCASE("Accepts non-empty") {
            auto result = cmd.parse(make_args({"--val", "hello"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects empty") {
            auto result = cmd.parse(make_args({"--val", ""}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("Regex validator") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(value)
                .validate(argu::validators::regex("^[a-z]+$", "must be lowercase letters only")));

        SUBCASE("Accepts matching pattern") {
            auto result = cmd.parse(make_args({"--val", "hello"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects non-matching pattern") {
            auto result = cmd.parse(make_args({"--val", "Hello123"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Help and Version Tests
// =============================================================================

TEST_SUITE("Argue Help and Version") {

    TEST_CASE("Help flag returns success with message") {
        auto cmd = argu::Command("test")
            .version("1.0.0")
            .about("Test app");

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.success());
        CHECK(result.exit_code() == 0);
        CHECK_FALSE(result.message().empty());
        CHECK(result.message().find("test") != std::string::npos);
    }

    TEST_CASE("Short help flag works") {
        auto cmd = argu::Command("test").about("Test app");

        auto result = cmd.parse(make_args({"-h"}));
        CHECK(result.success());
        CHECK_FALSE(result.message().empty());
    }

    TEST_CASE("Version flag returns success with message") {
        auto cmd = argu::Command("test").version("2.0.0");

        auto result = cmd.parse(make_args({"--version"}));
        CHECK(result.success());
        CHECK(result.exit_code() == 0);
        CHECK(result.message().find("2.0.0") != std::string::npos);
    }

    TEST_CASE("Help shows argument descriptions") {
        auto cmd = argu::Command("test")
            .about("My app")
            .arg(argu::Arg("input")
                .long_name("input")
                .help("The input file"));

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("input file") != std::string::npos);
    }
}

// =============================================================================
// Subcommand Tests
// =============================================================================

TEST_SUITE("Argue Subcommands") {

    TEST_CASE("Parse subcommand") {
        std::string add_name;
        auto add = argu::Command("add")
            .about("Add an item")
            .arg(argu::Arg("name").positional().required().value_of(add_name));

        auto cmd = argu::Command("test")
            .subcommand(std::move(add));

        auto result = cmd.parse(make_args({"add", "myitem"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand() == "add");
        CHECK(add_name == "myitem");
    }

    TEST_CASE("Subcommand alias works") {
        std::string name;
        auto remove = argu::Command("remove")
            .alias("rm")
            .arg(argu::Arg("name").positional().required().value_of(name));

        auto cmd = argu::Command("test")
            .subcommand(std::move(remove));

        auto result = cmd.parse(make_args({"rm", "item"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand() == "remove");
        CHECK(name == "item");
    }

    TEST_CASE("Required subcommand missing") {
        auto add = argu::Command("add").about("Add item");

        auto cmd = argu::Command("test")
            .subcommand(std::move(add))
            .subcommand_required(true);

        auto result = cmd.parse(make_args({}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("subcommand") != std::string::npos);
    }

    TEST_CASE("Global option available to subcommand") {
        bool verbose = false;
        std::string name;

        auto add = argu::Command("add")
            .arg(argu::Arg("name").positional().required().value_of(name));

        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose")
                .short_name('v')
                .flag(verbose)
                .global())
            .subcommand(std::move(add));

        auto result = cmd.parse(make_args({"-v", "add", "item"}));
        CHECK(result.success());
        CHECK(verbose);
        CHECK(name == "item");
    }
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_SUITE("Argue Error Handling") {

    TEST_CASE("Unknown option error") {
        auto cmd = argu::Command("test");

        auto result = cmd.parse(make_args({"--unknown"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("Unknown") != std::string::npos);
    }

    TEST_CASE("Missing value error") {
        std::string value;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("opt").value_of(value));

        auto result = cmd.parse(make_args({"--opt"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("requires a value") != std::string::npos);
    }

    TEST_CASE("Too many positional arguments") {
        std::string name;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name").positional().value_of(name));

        auto result = cmd.parse(make_args({"first", "second"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("Unexpected") != std::string::npos);
    }

    TEST_CASE("Conflicting arguments") {
        bool opt1 = false, opt2 = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt1").long_name("opt1").flag(opt1).conflicts_with("opt2"))
            .arg(argu::Arg("opt2").long_name("opt2").flag(opt2));

        auto result = cmd.parse(make_args({"--opt1", "--opt2"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("cannot be used together") != std::string::npos);
    }

    TEST_CASE("Required dependency missing") {
        bool debug = false;
        std::string log_file;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("debug").long_name("debug").flag(debug).requires_arg("log-file"))
            .arg(argu::Arg("log-file").long_name("log-file").value_of(log_file));

        auto result = cmd.parse(make_args({"--debug"}));
        CHECK_FALSE(result.success());
        CHECK(result.message().find("required by") != std::string::npos);
    }
}

// =============================================================================
// Matches API Tests
// =============================================================================

TEST_SUITE("Argue Matches API") {

    TEST_CASE("contains() checks argument presence") {
        bool flag = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("flag").short_name('f').flag(flag));

        cmd.parse(make_args({"-f"}));
        CHECK(cmd.matches().contains("flag"));
        CHECK_FALSE(cmd.matches().contains("nonexistent"));
    }

    TEST_CASE("occurrences() counts argument usage") {
        int verbosity = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("v").short_name('v').count(verbosity));

        cmd.parse(make_args({"-v", "-v", "-v"}));
        CHECK(cmd.matches().occurrences("v") == 3);
    }

    TEST_CASE("get_one() returns single value") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("opt").value_of(val));

        cmd.parse(make_args({"--opt", "hello"}));
        auto result = cmd.matches().get_one("opt");
        REQUIRE(result.has_value());
        CHECK(*result == "hello");
    }

    TEST_CASE("get_many() returns all values") {
        std::vector<std::string> vals;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").short_name('o').value_of(vals).takes_multiple());

        cmd.parse(make_args({"-o", "a", "-o", "b", "-o", "c"}));
        auto result = cmd.matches().get_many("opt");
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c");
    }

    TEST_CASE("get<T>() converts value") {
        int num = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("num").long_name("num").value_of(num));

        cmd.parse(make_args({"--num", "123"}));
        auto result = cmd.matches().get<int>("num");
        REQUIRE(result.has_value());
        CHECK(*result == 123);
    }

    TEST_CASE("get_or() returns default if missing") {
        auto cmd = argu::Command("test")
            .arg(argu::Arg("opt").long_name("opt"));

        cmd.parse(make_args({}));
        int result = cmd.matches().get_or<int>("opt", 42);
        CHECK(result == 42);
    }
}

// =============================================================================
// Type Conversion Tests
// =============================================================================

TEST_SUITE("Argue Type Conversion") {

    TEST_CASE("Convert to int") {
        int val = 0;
        auto cmd = argu::Command("test")
            .allow_negative_numbers(true)
            .arg(argu::Arg("n").long_name("n").value_of(val));

        cmd.parse(make_args({"--n", "-42"}));
        CHECK(val == -42);
    }

    TEST_CASE("Convert to unsigned int") {
        unsigned int val = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("n").long_name("n").value_of(val));

        cmd.parse(make_args({"--n", "42"}));
        CHECK(val == 42u);
    }

    TEST_CASE("Convert to float") {
        float val = 0.0f;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("n").long_name("n").value_of(val));

        cmd.parse(make_args({"--n", "3.14"}));
        CHECK(val == doctest::Approx(3.14f));
    }

    TEST_CASE("Convert to double") {
        double val = 0.0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("n").long_name("n").value_of(val));

        cmd.parse(make_args({"--n", "3.14159"}));
        CHECK(val == doctest::Approx(3.14159));
    }

    TEST_CASE("Convert to bool value") {
        bool val = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("b").long_name("b").value_of(val));

        SUBCASE("true") {
            cmd.parse(make_args({"--b", "true"}));
            CHECK(val == true);
        }

        SUBCASE("false") {
            val = true;
            cmd.parse(make_args({"--b", "false"}));
            CHECK(val == false);
        }

        SUBCASE("1") {
            val = false;
            cmd.parse(make_args({"--b", "1"}));
            CHECK(val == true);
        }

        SUBCASE("0") {
            val = true;
            cmd.parse(make_args({"--b", "0"}));
            CHECK(val == false);
        }
    }

    TEST_CASE("Convert to vector<int>") {
        std::vector<int> vals;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("n").short_name('n').value_of(vals).takes_multiple());

        cmd.parse(make_args({"-n", "1", "-n", "2", "-n", "3"}));
        REQUIRE(vals.size() == 3);
        CHECK(vals[0] == 1);
        CHECK(vals[1] == 2);
        CHECK(vals[2] == 3);
    }
}
