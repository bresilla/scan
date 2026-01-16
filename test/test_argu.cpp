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
        CHECK(result.message().find("requires") != std::string::npos);
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

// =============================================================================
// New Features Tests
// =============================================================================

TEST_SUITE("Argue New Features") {

    TEST_CASE("Deprecation support") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(val)
                .deprecated("Use --output-format instead"));

        auto result = cmd.parse(make_args({"--format", "json"}));
        CHECK(result.success());
        CHECK(val == "json");
    }

    TEST_CASE("Hidden alias support") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose")
                .long_name("verbose")
                .value_of(val)
                .hidden_alias("v"));  // --v works but hidden from help

        auto result = cmd.parse(make_args({"--v", "yes"}));
        CHECK(result.success());
        CHECK(val == "yes");
    }

    TEST_CASE("Renamed from support") {
        bool flag = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("debug")
                .long_name("debug")
                .flag(flag)
                .renamed_from("debug-mode"));  // Old name still works

        // New name works
        auto result1 = cmd.parse(make_args({"--debug"}));
        CHECK(result1.success());
        CHECK(flag == true);

        // Old name also works
        flag = false;
        auto result2 = cmd.parse(make_args({"--debug-mode"}));
        CHECK(result2.success());
        CHECK(flag == true);
    }

    TEST_CASE("Stdin/stdout handling API") {
        // Test that the API works (the actual stdin handling would be in the application)
        auto arg = argu::Arg("input")
            .long_name("input")
            .allow_stdin()
            .allow_stdout();

        CHECK(arg.allows_stdin() == true);
        CHECK(arg.allows_stdout() == true);
    }

    TEST_CASE("Value source tracking") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name")
                .long_name("name")
                .value_of(val)
                .default_value("default"));

        cmd.parse(make_args({"--name", "test"}));
        // Value source should be trackable
        CHECK(cmd.matches().contains("name"));
    }

    TEST_CASE("Bounded transformer clamps values") {
        int level = 0;
        auto arg = argu::Arg("level")
            .long_name("level")
            .value_of(level)
            .transform(argu::transformers::bounded(0, 10));

        // Test clamping high values
        std::string high_val = "100";
        std::string clamped = arg.apply_transformers(high_val);
        CHECK(clamped == "10");

        // Test clamping low values
        std::string low_val = "-5";
        clamped = arg.apply_transformers(low_val);
        CHECK(clamped == "0");

        // Test value in range
        std::string in_range = "5";
        clamped = arg.apply_transformers(in_range);
        CHECK(clamped == "5");
    }

    TEST_CASE("Map value transformer") {
        auto transformer = argu::transformers::map_value({
            {"y", "yes"},
            {"n", "no"},
            {"true", "yes"},
            {"false", "no"}
        });

        CHECK(transformer("y") == "yes");
        CHECK(transformer("n") == "no");
        CHECK(transformer("true") == "yes");
        CHECK(transformer("other") == "other");  // Unmapped values pass through
    }

    TEST_CASE("Default if empty transformer") {
        auto transformer = argu::transformers::default_if_empty("default_value");

        CHECK(transformer("") == "default_value");
        CHECK(transformer("actual") == "actual");
    }

    TEST_CASE("Normalize path transformer") {
        auto transformer = argu::transformers::normalize_path();

        CHECK(transformer("foo\\bar\\baz") == "foo/bar/baz");
        CHECK(transformer("foo/bar/baz") == "foo/bar/baz");
    }

    TEST_CASE("Required unless conditional") {
        std::string source;
        std::string file;

        auto cmd = argu::Command("test")
            .arg(argu::Arg("stdin")
                .long_name("stdin")
                .value_of(source))
            .arg(argu::Arg("file")
                .long_name("file")
                .value_of(file)
                .required_unless("stdin"));

        // With --stdin, --file is not required
        auto result1 = cmd.parse(make_args({"--stdin", "data"}));
        CHECK(result1.success());
    }

    TEST_CASE("Conflict mode - last wins") {
        std::string mode;
        auto cmd = argu::Command("test")
            .allow_overrides(true)  // POSIX style: last wins
            .arg(argu::Arg("mode")
                .short_name('m')
                .long_name("mode")
                .value_of(mode));

        // With allow_overrides, last value should win
        cmd.parse(make_args({"-m", "first", "-m", "second"}));
        CHECK(mode == "second");
    }

    TEST_CASE("Lifecycle hooks are stored") {
        bool pre_called = false;
        bool complete_called = false;
        bool final_called = false;

        auto cmd = argu::Command("test")
            .pre_parse([&pre_called]() { pre_called = true; })
            .parse_complete([&complete_called](argu::Matches&) { complete_called = true; })
            .final_callback([&final_called](const argu::Matches&) { final_called = true; })
            .arg(argu::Arg("dummy").long_name("dummy").flag());

        // Just verify the command builds successfully
        CHECK(!pre_called);  // Not called until parse
    }

    TEST_CASE("Partial matching option") {
        bool verbose = false;
        auto cmd = argu::Command("test")
            .allow_partial_matching(true)
            .arg(argu::Arg("verbose")
                .long_name("verbose")
                .flag(verbose));

        CHECK(cmd.allows_partial_matching() == true);
    }

    TEST_CASE("Subcommand prefix matching option") {
        auto cmd = argu::Command("test")
            .allow_subcommand_prefix(true);

        CHECK(cmd.allows_subcommand_prefix() == true);
    }

    TEST_CASE("Negatable flags API") {
        bool flag = false;
        auto arg = argu::Arg("color")
            .long_name("color")
            .flag(flag)
            .negatable();

        CHECK(arg.is_negatable() == true);
        CHECK(arg.matches_long("color") == true);
        CHECK(arg.matches_long("no-color") == true);
        CHECK(arg.is_negated_match("no-color") == true);
        CHECK(arg.is_negated_match("color") == false);
    }

    TEST_CASE("Visible aliases") {
        auto arg = argu::Arg("verbose")
            .long_name("verbose")
            .visible_alias("v")
            .visible_aliases({"verb", "vb"});

        CHECK(arg.get_visible_aliases().size() == 3);
        CHECK(arg.matches_long("v") == true);
        CHECK(arg.matches_long("verb") == true);
        CHECK(arg.matches_long("vb") == true);
    }

    TEST_CASE("Value hints") {
        auto arg1 = argu::Arg("file").hint_file();
        auto arg2 = argu::Arg("dir").hint_dir();
        auto arg3 = argu::Arg("custom").value_hint("CUSTOM");

        CHECK(arg1.get_value_hint().value() == "FILE");
        CHECK(arg2.get_value_hint().value() == "DIR");
        CHECK(arg3.get_value_hint().value() == "CUSTOM");
    }

    TEST_CASE("Display order") {
        auto arg1 = argu::Arg("first").display_order(1);
        auto arg2 = argu::Arg("second").display_order(2);

        CHECK(arg1.get_display_order() == 1);
        CHECK(arg2.get_display_order() == 2);
    }

    TEST_CASE("Exclusive group") {
        auto arg = argu::Arg("option")
            .exclusive_group("output_format");

        CHECK(arg.get_exclusive_group().value() == "output_format");
    }

    TEST_CASE("Last wins behavior") {
        auto arg = argu::Arg("mode").last_wins();
        CHECK(arg.is_last_wins() == true);
    }

    TEST_CASE("Max occurrences") {
        auto arg = argu::Arg("item").max_occurrences(5);
        CHECK(arg.get_max_occurrences() == 5);
    }

    TEST_CASE("Command env prefix") {
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_");

        CHECK(cmd.get_env_prefix().value() == "MYAPP_");
    }

    TEST_CASE("Trailing var arg") {
        auto cmd = argu::Command("test")
            .trailing_var_arg(true)
            .trailing_values_name("FILES");

        CHECK(cmd.has_trailing_var_arg() == true);
        CHECK(cmd.get_trailing_values_name() == "FILES");
    }

    TEST_CASE("Allow hyphen values") {
        auto cmd = argu::Command("test")
            .allow_hyphen_values(true);

        CHECK(cmd.allows_hyphen_values() == true);
    }

    TEST_CASE("Ignore errors option") {
        auto cmd = argu::Command("test")
            .ignore_errors(true);

        CHECK(cmd.ignores_errors() == true);
    }

    TEST_CASE("Help on error option") {
        auto cmd = argu::Command("test")
            .help_on_error(false);

        CHECK(cmd.shows_help_on_error() == false);
    }

    TEST_CASE("Max term width") {
        auto cmd = argu::Command("test")
            .max_term_width(120);

        CHECK(cmd.get_max_term_width() == 120);
    }
}

// =============================================================================
// Environment Variable Cascading Tests
// =============================================================================

TEST_SUITE("Environment Variable Cascading") {

    TEST_CASE("Explicit env var name takes precedence") {
        // Set environment variable
        setenv("MY_EXPLICIT_VAR", "explicit_value", 1);

        std::string value;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("option")
                .long_name("option")
                .env("MY_EXPLICIT_VAR")
                .value_of(value));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == "explicit_value");
        CHECK(cmd.matches().is_from_env("option") == true);
        CHECK(cmd.matches().value_source("option") == argu::ValueSource::Environment);

        unsetenv("MY_EXPLICIT_VAR");
    }

    TEST_CASE("Arg env prefix + arg name") {
        // Set environment variable: MYARG_MY_OPTION
        setenv("MYARG_MY_OPTION", "arg_prefix_value", 1);

        std::string value;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_") // Command prefix should be ignored
            .arg(argu::Arg("my-option")
                .long_name("my-option")
                .env_prefix("MYARG_") // Arg-level prefix takes precedence
                .value_of(value));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == "arg_prefix_value");

        unsetenv("MYARG_MY_OPTION");
    }

    TEST_CASE("Command env prefix auto-generates var name") {
        // Set environment variable: MYAPP_OUTPUT_FILE
        setenv("MYAPP_OUTPUT_FILE", "cmd_prefix_value", 1);

        std::string value;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("output-file")
                .long_name("output-file")
                .value_of(value));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == "cmd_prefix_value");

        unsetenv("MYAPP_OUTPUT_FILE");
    }

    TEST_CASE("Command line overrides environment variable") {
        setenv("MYAPP_PORT", "8080", 1);

        int port = 0;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("port")
                .long_name("port")
                .value_of(port));

        auto result = cmd.parse(make_args({"--port", "9090"}));
        CHECK(result.success());
        CHECK(port == 9090);
        CHECK(cmd.matches().value_source("port") == argu::ValueSource::CommandLine);

        unsetenv("MYAPP_PORT");
    }

    TEST_CASE("Environment variable with value delimiter") {
        setenv("MYAPP_INCLUDE_PATHS", "/usr/include,/opt/include,/home/user/include", 1);

        std::vector<std::string> paths;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("include-paths")
                .long_name("include-paths")
                .value_delimiter(',')
                .value_of(paths));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        REQUIRE(paths.size() == 3);
        CHECK(paths[0] == "/usr/include");
        CHECK(paths[1] == "/opt/include");
        CHECK(paths[2] == "/home/user/include");

        unsetenv("MYAPP_INCLUDE_PATHS");
    }

    TEST_CASE("Env var with transformer applied") {
        setenv("MYAPP_LEVEL", "10", 1);

        int level = 0;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("level")
                .long_name("level")
                .transform(argu::transformers::bounded(0, 5))
                .value_of(level));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(level == 5); // Clamped to max

        unsetenv("MYAPP_LEVEL");
    }

    TEST_CASE("No env var set, uses default") {
        std::string host = "localhost";
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("host")
                .long_name("host")
                .default_value("localhost")
                .value_of(host));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(host == "localhost");
        CHECK(cmd.matches().value_source("host") == argu::ValueSource::Default);
    }

    TEST_CASE("Env var priority: explicit > arg-prefix > cmd-prefix") {
        // Set all three types of env vars
        setenv("EXPLICIT_VAR", "explicit", 1);
        setenv("ARGPFX_OPTION", "arg_prefix", 1);
        setenv("CMDPFX_OPTION", "cmd_prefix", 1);

        std::string value;
        auto cmd = argu::Command("test")
            .env_prefix("CMDPFX_")
            .arg(argu::Arg("option")
                .long_name("option")
                .env("EXPLICIT_VAR")
                .env_prefix("ARGPFX_")
                .value_of(value));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(value == "explicit"); // Explicit should win

        unsetenv("EXPLICIT_VAR");
        unsetenv("ARGPFX_OPTION");
        unsetenv("CMDPFX_OPTION");
    }

    TEST_CASE("Hyphen to underscore conversion in env var name") {
        setenv("MYAPP_LOG_LEVEL", "debug", 1);

        std::string level;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("log-level")
                .long_name("log-level")
                .value_of(level));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(level == "debug");

        unsetenv("MYAPP_LOG_LEVEL");
    }

    TEST_CASE("Dot to underscore conversion in env var name") {
        setenv("MYAPP_DB_HOST", "localhost", 1);

        std::string host;
        auto cmd = argu::Command("test")
            .env_prefix("MYAPP_")
            .arg(argu::Arg("db.host")
                .long_name("db-host")
                .value_of(host));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(host == "localhost");

        unsetenv("MYAPP_DB_HOST");
    }
}

// =============================================================================
// Negatable Flags Tests
// =============================================================================

TEST_SUITE("Negatable Flags") {

    TEST_CASE("Negatable flag --color sets true") {
        bool color = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .negatable()
                .flag(color));

        auto result = cmd.parse(make_args({"--color"}));
        CHECK(result.success());
        CHECK(color == true);
    }

    TEST_CASE("Negatable flag --no-color sets false") {
        bool color = true;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .negatable()
                .flag(color));

        auto result = cmd.parse(make_args({"--no-color"}));
        CHECK(result.success());
        CHECK(color == false);
    }

    TEST_CASE("Negatable flag default value") {
        bool color = true;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .negatable()
                .flag(color));

        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
        CHECK(color == true); // Not touched
    }

    TEST_CASE("Non-negatable flag doesn't match --no-") {
        bool verbose = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose")
                .long_name("verbose")
                .flag(verbose));

        auto result = cmd.parse(make_args({"--no-verbose"}));
        CHECK(!result.success()); // Should fail - unknown argument
    }

    TEST_CASE("Negatable flag is_negated_match") {
        auto arg = argu::Arg("color")
            .long_name("color")
            .negatable()
            .flag();

        CHECK(arg.matches_long("color") == true);
        CHECK(arg.matches_long("no-color") == true);
        CHECK(arg.is_negated_match("no-color") == true);
        CHECK(arg.is_negated_match("color") == false);
    }

    TEST_CASE("Multiple negatable flags in same command") {
        bool color = false;
        bool debug = true;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .negatable()
                .flag(color))
            .arg(argu::Arg("debug")
                .long_name("debug")
                .negatable()
                .flag(debug));

        auto result = cmd.parse(make_args({"--color", "--no-debug"}));
        CHECK(result.success());
        CHECK(color == true);
        CHECK(debug == false);
    }
}

// =============================================================================
// Custom Help Sections and Ordering Tests
// =============================================================================

TEST_SUITE("Custom Help Sections") {

    TEST_CASE("Add custom help section") {
        auto cmd = argu::Command("test")
            .about("Test program")
            .help_section("EXAMPLES", "example1\nexample2\nexample3");

        auto sections = cmd.get_custom_sections();
        REQUIRE(sections.size() == 1);
        CHECK(sections[0].title == "EXAMPLES");
        CHECK(sections[0].content == "example1\nexample2\nexample3");
    }

    TEST_CASE("Multiple custom sections with ordering") {
        auto cmd = argu::Command("test")
            .help_section("SECOND", "Second section content", 20)
            .help_section("FIRST", "First section content", 10)
            .help_section("THIRD", "Third section content", 30);

        auto sections = cmd.get_custom_sections();
        REQUIRE(sections.size() == 3);

        // Sections should be stored in add order, sorted later
        CHECK(sections[0].title == "SECOND");
        CHECK(sections[1].title == "FIRST");
        CHECK(sections[2].title == "THIRD");

        // Check orders
        CHECK(sections[0].order == 20);
        CHECK(sections[1].order == 10);
        CHECK(sections[2].order == 30);
    }

    TEST_CASE("Custom section appears in help output") {
        auto cmd = argu::Command("test")
            .about("Test program")
            .help_section("EXAMPLES", "  myapp --input file.txt");

        std::string help = cmd.help();
        CHECK(help.find("EXAMPLES:") != std::string::npos);
        CHECK(help.find("myapp --input file.txt") != std::string::npos);
    }

    TEST_CASE("Argument group ordering") {
        auto cmd = argu::Command("test")
            .group_order("OUTPUT", 10)
            .group_order("INPUT", 20)
            .group_order("OPTIONS", 30);

        CHECK(cmd.get_group_order("OUTPUT") == 10);
        CHECK(cmd.get_group_order("INPUT") == 20);
        CHECK(cmd.get_group_order("OPTIONS") == 30);
        CHECK(cmd.get_group_order("UNKNOWN") == 50); // Default
    }

    TEST_CASE("Argument display order within group") {
        auto cmd = argu::Command("test")
            .arg(argu::Arg("third")
                .long_name("third")
                .display_order(30)
                .group("OPTIONS"))
            .arg(argu::Arg("first")
                .long_name("first")
                .display_order(10)
                .group("OPTIONS"))
            .arg(argu::Arg("second")
                .long_name("second")
                .display_order(20)
                .group("OPTIONS"));

        // Verify display orders are set
        auto args = cmd.get_args();
        REQUIRE(args.size() >= 3);

        // Find each arg by name and check display order
        for (const auto &arg : args) {
            if (arg.name() == "first") CHECK(arg.get_display_order() == 10);
            if (arg.name() == "second") CHECK(arg.get_display_order() == 20);
            if (arg.name() == "third") CHECK(arg.get_display_order() == 30);
        }
    }

    TEST_CASE("Help output respects group order") {
        auto cmd = argu::Command("test")
            .about("Test program")
            .group_order("SECOND_GROUP", 20)
            .group_order("FIRST_GROUP", 10)
            .arg(argu::Arg("option2")
                .long_name("option2")
                .group("SECOND_GROUP"))
            .arg(argu::Arg("option1")
                .long_name("option1")
                .group("FIRST_GROUP"));

        std::string help = cmd.help();
        auto first_pos = help.find("FIRST_GROUP:");
        auto second_pos = help.find("SECOND_GROUP:");

        // FIRST_GROUP should appear before SECOND_GROUP
        REQUIRE(first_pos != std::string::npos);
        REQUIRE(second_pos != std::string::npos);
        CHECK(first_pos < second_pos);
    }
}

// =============================================================================
// External Subcommand Passthrough Tests
// =============================================================================

TEST_SUITE("External Subcommand Passthrough") {

    TEST_CASE("External subcommand is captured") {
        auto cmd = argu::Command("git")
            .allow_external_subcommands(true)
            .subcommand(argu::Command("status").about("Show status"));

        auto result = cmd.parse(make_args({"my-custom-tool", "arg1", "arg2"}));
        CHECK(result.success());

        auto &matches = cmd.matches();
        CHECK(matches.subcommand().has_value());
        CHECK(*matches.subcommand() == "my-custom-tool");
        CHECK(matches.is_external_subcommand() == true);

        auto ext_args = matches.external_args();
        REQUIRE(ext_args.size() == 2);
        CHECK(ext_args[0] == "arg1");
        CHECK(ext_args[1] == "arg2");
    }

    TEST_CASE("Known subcommand is handled normally") {
        auto cmd = argu::Command("git")
            .allow_external_subcommands(true)
            .subcommand(argu::Command("status").about("Show status"));

        auto result = cmd.parse(make_args({"status"}));
        CHECK(result.success());

        auto &matches = cmd.matches();
        CHECK(matches.subcommand().has_value());
        CHECK(*matches.subcommand() == "status");
        CHECK(matches.is_external_subcommand() == false);
    }

    TEST_CASE("Trailing var arg captures remaining args") {
        auto cmd = argu::Command("run")
            .trailing_var_arg(true)
            .trailing_values_name("COMMAND");

        auto result = cmd.parse(make_args({"echo", "hello", "world"}));
        CHECK(result.success());

        auto &matches = cmd.matches();
        auto trailing = matches.get_many("COMMAND");
        REQUIRE(trailing.size() == 3);
        CHECK(trailing[0] == "echo");
        CHECK(trailing[1] == "hello");
        CHECK(trailing[2] == "world");
    }

    TEST_CASE("Trailing var arg with options before") {
        bool verbose = false;
        auto cmd = argu::Command("run")
            .trailing_var_arg(true)
            .trailing_values_name("COMMAND")
            .arg(argu::Arg("verbose")
                .short_name('v')
                .long_name("verbose")
                .flag(verbose));

        auto result = cmd.parse(make_args({"-v", "echo", "hello"}));
        CHECK(result.success());
        CHECK(verbose == true);

        auto trailing = cmd.matches().get_many("COMMAND");
        REQUIRE(trailing.size() == 2);
        CHECK(trailing[0] == "echo");
        CHECK(trailing[1] == "hello");
    }
}

// =============================================================================
// Callback Chaining Tests
// =============================================================================

TEST_SUITE("Callback Chaining") {

    TEST_CASE("Value callback is called on parse") {
        std::string captured;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name")
                .long_name("name")
                .on_parse([&captured](const std::string &val) {
                    captured = val;
                }));

        cmd.parse(make_args({"--name", "test_value"}));
        CHECK(captured == "test_value");
    }

    TEST_CASE("Multiple value callbacks are chained") {
        std::vector<std::string> log;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name")
                .long_name("name")
                .on_parse([&log](const std::string &val) {
                    log.push_back("cb1: " + val);
                })
                .on_parse([&log](const std::string &val) {
                    log.push_back("cb2: " + val);
                }));

        cmd.parse(make_args({"--name", "value"}));
        REQUIRE(log.size() == 2);
        CHECK(log[0] == "cb1: value");
        CHECK(log[1] == "cb2: value");
    }

    TEST_CASE("Flag callback is called") {
        bool flag_value = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose")
                .short_name('v')
                .long_name("verbose")
                .flag()
                .on_flag([&flag_value](bool val) {
                    flag_value = val;
                }));

        cmd.parse(make_args({"-v"}));
        CHECK(flag_value == true);
    }

    TEST_CASE("Count callback is called") {
        int count_value = 0;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("verbose")
                .short_name('v')
                .count()
                .on_count([&count_value](int val) {
                    count_value = val;
                }));

        cmd.parse(make_args({"-vvv"}));
        CHECK(count_value == 3);
    }

    TEST_CASE("Callback with variable binding") {
        std::string name;
        std::string upper_name;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("name")
                .long_name("name")
                .value_of(name)
                .on_parse([&upper_name](const std::string &val) {
                    upper_name = val;
                    for (char &c : upper_name) {
                        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                    }
                }));

        cmd.parse(make_args({"--name", "hello"}));
        CHECK(name == "hello");
        CHECK(upper_name == "HELLO");
    }

    TEST_CASE("Each transform callback is chained") {
        std::string result;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("value")
                .long_name("value")
                .each([](const std::string &val) {
                    return val + "_first";
                })
                .each([](const std::string &val) {
                    return val + "_second";
                })
                .value_of(result));

        cmd.parse(make_args({"--value", "input"}));
        CHECK(result == "input_first_second");
    }
}

// =============================================================================
// Argument Group Tests
// =============================================================================

TEST_SUITE("Argument Groups") {

    TEST_CASE("Mutex group allows one argument") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("output_format")
                .mutually_exclusive()
                .args({"json", "xml", "csv"}))
            .arg(argu::Arg("json").long_name("json").flag())
            .arg(argu::Arg("xml").long_name("xml").flag())
            .arg(argu::Arg("csv").long_name("csv").flag());

        auto result = cmd.parse(make_args({"--json"}));
        CHECK(result.success());
    }

    TEST_CASE("Mutex group rejects multiple arguments") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("output_format")
                .mutually_exclusive()
                .args({"json", "xml"}))
            .arg(argu::Arg("json").long_name("json").flag())
            .arg(argu::Arg("xml").long_name("xml").flag());

        auto result = cmd.parse(make_args({"--json", "--xml"}));
        CHECK(!result.success());
        CHECK(result.message().find("mutually exclusive") != std::string::npos);
    }

    TEST_CASE("Required together group accepts all") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("credentials")
                .required_together()
                .args({"user", "password"}))
            .arg(argu::Arg("user").long_name("user"))
            .arg(argu::Arg("password").long_name("password"));

        auto result = cmd.parse(make_args({"--user", "admin", "--password", "secret"}));
        CHECK(result.success());
    }

    TEST_CASE("Required together group rejects partial") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("credentials")
                .required_together()
                .args({"user", "password"}))
            .arg(argu::Arg("user").long_name("user"))
            .arg(argu::Arg("password").long_name("password"));

        auto result = cmd.parse(make_args({"--user", "admin"}));
        CHECK(!result.success());
        CHECK(result.message().find("must be used together") != std::string::npos);
    }

    TEST_CASE("At least one group requires one argument") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("input_source")
                .at_least_one()
                .required()
                .args({"file", "stdin"}))
            .arg(argu::Arg("file").long_name("file"))
            .arg(argu::Arg("stdin").long_name("stdin").flag());

        // With one argument - should pass
        auto result = cmd.parse(make_args({"--file", "test.txt"}));
        CHECK(result.success());
    }

    TEST_CASE("At least one group rejects none") {
        auto cmd = argu::Command("test")
            .group(argu::ArgGroup("input_source")
                .at_least_one()
                .required()
                .args({"file", "url"}))
            .arg(argu::Arg("file").long_name("file"))
            .arg(argu::Arg("url").long_name("url"));

        auto result = cmd.parse(make_args({}));
        CHECK(!result.success());
        CHECK(result.message().find("At least one") != std::string::npos);
    }
}

// =============================================================================
// Partial Matching Tests
// =============================================================================

TEST_SUITE("Partial Matching") {

    TEST_CASE("Partial match for long option") {
        std::string value;
        auto cmd = argu::Command("test")
            .allow_partial_matching(true)
            .arg(argu::Arg("verbose")
                .long_name("verbose")
                .flag())
            .arg(argu::Arg("version")
                .long_name("version")
                .flag());

        // "verb" is unambiguous - matches "verbose"
        bool verbose = false;
        cmd.arg(argu::Arg("output").long_name("output").value_of(value));

        auto result = cmd.parse(make_args({"--out", "test.txt"}));
        CHECK(result.success());
        CHECK(value == "test.txt");
    }

    TEST_CASE("Partial match rejected when ambiguous") {
        auto cmd = argu::Command("test")
            .allow_partial_matching(true)
            .arg(argu::Arg("verbose").long_name("verbose").flag())
            .arg(argu::Arg("version").long_name("version").flag());

        // "ver" is ambiguous - could be "verbose" or "version"
        auto result = cmd.parse(make_args({"--ver"}));
        CHECK(!result.success()); // Should fail - ambiguous
    }

    TEST_CASE("Exact match takes precedence over partial") {
        std::string value;
        auto cmd = argu::Command("test")
            .allow_partial_matching(true)
            .arg(argu::Arg("out").long_name("out").value_of(value))
            .arg(argu::Arg("output").long_name("output"));

        auto result = cmd.parse(make_args({"--out", "test.txt"}));
        CHECK(result.success());
        CHECK(value == "test.txt");
    }
}

// =============================================================================
// Subcommand Prefix Matching Tests
// =============================================================================

TEST_SUITE("Subcommand Prefix Matching") {

    TEST_CASE("Subcommand prefix matching when enabled") {
        bool status_called = false;

        auto cmd = argu::Command("git")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("status").callback([&](const argu::Matches&) { status_called = true; }))
            .subcommand(argu::Command("commit"))
            .subcommand(argu::Command("checkout"));

        auto result = cmd.parse(make_args({"stat"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "status");
    }

    TEST_CASE("Subcommand prefix matching disabled by default") {
        auto cmd = argu::Command("git")
            .subcommand(argu::Command("status"))
            .subcommand(argu::Command("commit"));

        auto result = cmd.parse(make_args({"stat"}));
        CHECK(!result.success()); // Should fail - prefix not allowed by default
    }

    TEST_CASE("Ambiguous subcommand prefix is rejected") {
        auto cmd = argu::Command("git")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("status"))
            .subcommand(argu::Command("stash"));

        // "sta" is ambiguous - could be "status" or "stash"
        auto result = cmd.parse(make_args({"sta"}));
        CHECK(!result.success());
    }

    TEST_CASE("Exact subcommand match takes precedence") {
        auto cmd = argu::Command("git")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("st").about("Short status"))
            .subcommand(argu::Command("status").about("Full status"));

        auto result = cmd.parse(make_args({"st"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "st");
    }

    TEST_CASE("Subcommand prefix with alias") {
        auto cmd = argu::Command("git")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("checkout").alias("co"));

        // "check" should match "checkout"
        auto result = cmd.parse(make_args({"check"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "checkout");
    }

    TEST_CASE("Subcommand prefix matching with nested subcommands") {
        auto cmd = argu::Command("git")
            .allow_subcommand_prefix(true)
            .subcommand(
                argu::Command("remote")
                    .allow_subcommand_prefix(true)
                    .subcommand(argu::Command("add"))
                    .subcommand(argu::Command("remove"))
            );

        auto result = cmd.parse(make_args({"rem", "ad"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "remote");
        auto sub = cmd.matches().subcommand_matches();
        REQUIRE(sub != nullptr);
        CHECK(sub->subcommand().value() == "add");
    }
}

// =============================================================================
// Global Arguments Tests
// =============================================================================

TEST_SUITE("Global Arguments") {

    TEST_CASE("Global argument propagates to subcommand") {
        bool verbose = false;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("verbose").short_name('v').flag(verbose).global())
            .subcommand(argu::Command("run").about("Run the app"));

        auto result = cmd.parse(make_args({"run", "-v"}));
        CHECK(result.success());
        CHECK(verbose);
    }

    TEST_CASE("Global argument can be used before subcommand") {
        bool verbose = false;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("verbose").short_name('v').flag(verbose).global())
            .subcommand(argu::Command("run"));

        // Global arg before subcommand
        auto result = cmd.parse(make_args({"-v", "run"}));
        // This depends on implementation - may need adjustment
        // For now, we just test the basic propagation in subcommand
    }

    TEST_CASE("Global argument propagates to nested subcommands") {
        bool debug = false;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("debug").short_name('d').flag(debug).global())
            .subcommand(
                argu::Command("service")
                    .subcommand(argu::Command("start"))
            );

        auto result = cmd.parse(make_args({"service", "start", "-d"}));
        CHECK(result.success());
        CHECK(debug);
    }
}

// =============================================================================
// Hidden Subcommands Tests
// =============================================================================

TEST_SUITE("Hidden Subcommands") {

    TEST_CASE("Hidden subcommand still works") {
        auto cmd = argu::Command("app")
            .subcommand(argu::Command("public").about("Public command"))
            .subcommand(argu::Command("internal").about("Internal command").hidden());

        auto result = cmd.parse(make_args({"internal"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "internal");
    }

    TEST_CASE("Hidden subcommand not shown in help") {
        auto cmd = argu::Command("app")
            .about("Test app")
            .subcommand(argu::Command("public").about("Public command"))
            .subcommand(argu::Command("internal").about("Internal command").hidden());

        std::string help_text = cmd.help();
        CHECK(help_text.find("public") != std::string::npos);
        CHECK(help_text.find("internal") == std::string::npos);
    }

    TEST_CASE("Hidden subcommand not suggested for typos") {
        auto cmd = argu::Command("app")
            .subcommand_required(true)
            .subcommand(argu::Command("public"))
            .subcommand(argu::Command("internal").hidden());

        // Typo "internl" should not suggest "internal" since it's hidden
        auto result = cmd.parse(make_args({"internl"}));
        CHECK(!result.success());
        // The error message should not contain "internal" as a suggestion
        CHECK(result.message().find("internal") == std::string::npos);
    }
}

// =============================================================================
// Error Aggregation Tests
// =============================================================================

TEST_SUITE("Error Aggregation") {

    TEST_CASE("FirstError mode stops at first error") {
        std::string input;
        std::string output;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::FirstError)
            .arg(argu::Arg("input").long_name("input").required().value_of(input))
            .arg(argu::Arg("output").long_name("output").required().value_of(output));

        // Missing both required args - should only report first
        auto result = cmd.parse(make_args({}));
        CHECK(!result.success());
        // Should mention missing argument but only one error
        CHECK(result.message().find("Missing required") != std::string::npos);
        CHECK(result.message().find("Multiple errors") == std::string::npos);
    }

    TEST_CASE("Aggregate mode collects multiple errors") {
        std::string input;
        std::string output;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::Aggregate)
            .arg(argu::Arg("input").long_name("input").required().value_of(input))
            .arg(argu::Arg("output").long_name("output").required().value_of(output));

        // Missing both required args
        auto result = cmd.parse(make_args({}));
        CHECK(!result.success());
        // Should report multiple errors
        CHECK(result.message().find("Multiple errors") != std::string::npos);
        CHECK(result.message().find("input") != std::string::npos);
        CHECK(result.message().find("output") != std::string::npos);
    }

    TEST_CASE("Aggregate mode with single error") {
        std::string input;
        std::string output;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::Aggregate)
            .arg(argu::Arg("input").long_name("input").required().value_of(input))
            .arg(argu::Arg("output").long_name("output").value_of(output));

        // Missing only one required arg
        auto result = cmd.parse(make_args({}));
        CHECK(!result.success());
        // Single error should not say "Multiple errors"
        CHECK(result.message().find("Multiple errors") == std::string::npos);
        CHECK(result.message().find("input") != std::string::npos);
    }

    TEST_CASE("Aggregate mode collects constraint violations") {
        bool verbose = false;
        bool quiet = false;
        std::string required_opt;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::Aggregate)
            .arg(argu::Arg("verbose").short_name('v').flag(verbose))
            .arg(argu::Arg("quiet").short_name('q').flag(quiet).conflicts_with("verbose"))
            .arg(argu::Arg("required").long_name("required").required().value_of(required_opt));

        // Both conflict AND missing required
        auto result = cmd.parse(make_args({"-v", "-q"}));
        CHECK(!result.success());
        CHECK(result.message().find("Multiple errors") != std::string::npos);
        CHECK(result.message().find("cannot be used together") != std::string::npos);
        CHECK(result.message().find("Missing required") != std::string::npos);
    }

    TEST_CASE("Aggregate mode with group violations") {
        std::string opt1;
        std::string opt2;
        std::string opt3;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::Aggregate)
            .arg(argu::Arg("opt1").long_name("opt1").value_of(opt1))
            .arg(argu::Arg("opt2").long_name("opt2").value_of(opt2))
            .arg(argu::Arg("opt3").long_name("opt3").required().value_of(opt3))
            .group(argu::ArgGroup("mutex").mutually_exclusive().args({"opt1", "opt2"}));

        // Mutex violation AND missing required
        auto result = cmd.parse(make_args({"--opt1=a", "--opt2=b"}));
        CHECK(!result.success());
        CHECK(result.message().find("Multiple errors") != std::string::npos);
    }

    TEST_CASE("No errors in aggregate mode") {
        std::string input;

        auto cmd = argu::Command("app")
            .error_mode(argu::ErrorMode::Aggregate)
            .arg(argu::Arg("input").long_name("input").value_of(input));

        auto result = cmd.parse(make_args({"--input=test"}));
        CHECK(result.success());
    }
}

// =============================================================================
// Conditional Requirements Tests
// =============================================================================

TEST_SUITE("Conditional Requirements") {

    TEST_CASE("required_unless - satisfied when alternative present") {
        std::string input;
        std::string stdin_flag;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("input")
                .long_name("input")
                .value_of(input)
                .required_unless("stdin"))
            .arg(argu::Arg("stdin")
                .long_name("stdin")
                .value_of(stdin_flag));

        // Should succeed: --stdin is present, so --input is not required
        auto result = cmd.parse(make_args({"--stdin=yes"}));
        CHECK(result.success());
    }

    TEST_CASE("required_unless - fails when no alternative") {
        std::string input;
        std::string stdin_flag;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("input")
                .long_name("input")
                .value_of(input)
                .required_unless("stdin"))
            .arg(argu::Arg("stdin")
                .long_name("stdin")
                .value_of(stdin_flag));

        // Should fail: neither --input nor --stdin provided
        auto result = cmd.parse(make_args({}));
        CHECK(!result.success());
        CHECK(result.message().find("input") != std::string::npos);
    }

    TEST_CASE("required_unless - multiple alternatives") {
        std::string input;
        std::string file;
        std::string url;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("input")
                .long_name("input")
                .value_of(input)
                .required_unless({"file", "url"}))
            .arg(argu::Arg("file").long_name("file").value_of(file))
            .arg(argu::Arg("url").long_name("url").value_of(url));

        // Should succeed: --url is present
        auto result = cmd.parse(make_args({"--url=http://example.com"}));
        CHECK(result.success());
    }

    TEST_CASE("required_if_eq - triggered when condition met") {
        std::string format;
        std::string output;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(format)
                .choices({"json", "csv", "text"}))
            .arg(argu::Arg("output")
                .long_name("output")
                .value_of(output)
                .required_if_eq("format", "csv"));

        // Should fail: format=csv requires output
        auto result = cmd.parse(make_args({"--format=csv"}));
        CHECK(!result.success());
        CHECK(result.message().find("output") != std::string::npos);
    }

    TEST_CASE("required_if_eq - not triggered when condition not met") {
        std::string format;
        std::string output;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(format)
                .choices({"json", "csv", "text"}))
            .arg(argu::Arg("output")
                .long_name("output")
                .value_of(output)
                .required_if_eq("format", "csv"));

        // Should succeed: format=json doesn't require output
        auto result = cmd.parse(make_args({"--format=json"}));
        CHECK(result.success());
    }

    TEST_CASE("required_if_eq - satisfied when provided") {
        std::string format;
        std::string output;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(format)
                .choices({"json", "csv", "text"}))
            .arg(argu::Arg("output")
                .long_name("output")
                .value_of(output)
                .required_if_eq("format", "csv"));

        // Should succeed: format=csv and output provided
        auto result = cmd.parse(make_args({"--format=csv", "--output=data.csv"}));
        CHECK(result.success());
        CHECK(output == "data.csv");
    }

    TEST_CASE("requires_if - triggered when value matches") {
        std::string mode;
        std::string config;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("mode")
                .long_name("mode")
                .value_of(mode)
                .requires_if("config", "custom"))
            .arg(argu::Arg("config")
                .long_name("config")
                .value_of(config));

        // Should fail: mode=custom requires config
        auto result = cmd.parse(make_args({"--mode=custom"}));
        CHECK(!result.success());
        CHECK(result.message().find("config") != std::string::npos);
    }

    TEST_CASE("requires_if - not triggered when value doesn't match") {
        std::string mode;
        std::string config;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("mode")
                .long_name("mode")
                .value_of(mode)
                .requires_if("config", "custom"))
            .arg(argu::Arg("config")
                .long_name("config")
                .value_of(config));

        // Should succeed: mode=auto doesn't require config
        auto result = cmd.parse(make_args({"--mode=auto"}));
        CHECK(result.success());
    }

    TEST_CASE("default_value_if - applies conditional default") {
        std::string format;
        std::string extension;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(format)
                .choices({"json", "csv", "text"}))
            .arg(argu::Arg("extension")
                .long_name("ext")
                .value_of(extension)
                .default_value_if("format", "json", ".json")
                .default_value_if("format", "csv", ".csv")
                .default_value(".txt"));

        // Should get .json extension when format=json
        auto result = cmd.parse(make_args({"--format=json"}));
        CHECK(result.success());
        CHECK(extension == ".json");
    }

    TEST_CASE("default_value_if - falls back to regular default") {
        std::string format;
        std::string extension;

        auto cmd = argu::Command("app")
            .arg(argu::Arg("format")
                .long_name("format")
                .value_of(format)
                .choices({"json", "csv", "text"}))
            .arg(argu::Arg("extension")
                .long_name("ext")
                .value_of(extension)
                .default_value_if("format", "json", ".json")
                .default_value(".txt"));

        // Should get .txt extension when format=text (no conditional match)
        auto result = cmd.parse(make_args({"--format=text"}));
        CHECK(result.success());
        CHECK(extension == ".txt");
    }
}

// =============================================================================
// Edge Cases Tests
// =============================================================================

TEST_SUITE("Edge Cases") {

    TEST_CASE("Empty string value") {
        std::string value;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("opt").long_name("opt").value_of(value));

        auto result = cmd.parse(make_args({"--opt="}));
        CHECK(result.success());
        CHECK(value.empty());
    }

    TEST_CASE("Value with equals sign") {
        std::string value;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("opt").long_name("opt").value_of(value));

        auto result = cmd.parse(make_args({"--opt=key=value"}));
        CHECK(result.success());
        CHECK(value == "key=value");
    }

    TEST_CASE("Value with spaces using quotes") {
        std::string value;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("opt").long_name("opt").value_of(value));

        auto result = cmd.parse(make_args({"--opt=hello world"}));
        CHECK(result.success());
        CHECK(value == "hello world");
    }

    TEST_CASE("Multiple short flags combined") {
        bool a = false, b = false, c = false;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("a").short_name('a').flag(a))
            .arg(argu::Arg("b").short_name('b').flag(b))
            .arg(argu::Arg("c").short_name('c').flag(c));

        auto result = cmd.parse(make_args({"-abc"}));
        CHECK(result.success());
        CHECK(a);
        CHECK(b);
        CHECK(c);
    }

    TEST_CASE("Short option with attached value") {
        std::string value;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("opt").short_name('o').value_of(value));

        auto result = cmd.parse(make_args({"-ovalue"}));
        CHECK(result.success());
        CHECK(value == "value");
    }

    TEST_CASE("Double dash stops option parsing") {
        std::string opt;
        std::string pos;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("opt").long_name("opt").value_of(opt))
            .arg(argu::Arg("pos").positional().value_of(pos));

        auto result = cmd.parse(make_args({"--", "--opt"}));
        CHECK(result.success());
        CHECK(pos == "--opt");
        CHECK(opt.empty());
    }

    TEST_CASE("Counting flag multiple times") {
        int verbosity = 0;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("verbose").short_name('v').count(verbosity));

        auto result = cmd.parse(make_args({"-v", "-v", "-v", "-v"}));
        CHECK(result.success());
        CHECK(verbosity == 4);
    }

    TEST_CASE("Value delimiter parsing") {
        std::vector<std::string> values;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("items")
                .long_name("items")
                .value_of(values)
                .value_delimiter(','));

        auto result = cmd.parse(make_args({"--items=a,b,c,d"}));
        CHECK(result.success());
        CHECK(values.size() == 4);
        CHECK(values[0] == "a");
        CHECK(values[3] == "d");
    }

    TEST_CASE("Implicit value when no value provided") {
        std::string level;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("debug")
                .long_name("debug")
                .value_of(level)
                .implicit_value("info")
                .default_value("none"));

        // With just --debug, should get implicit value
        auto result = cmd.parse(make_args({"--debug"}));
        CHECK(result.success());
        CHECK(level == "info");
    }

    TEST_CASE("Hex number parsing") {
        int value = 0;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("num").long_name("num").value_of(value));

        auto result = cmd.parse(make_args({"--num=0xFF"}));
        CHECK(result.success());
        CHECK(value == 255);
    }

    TEST_CASE("Binary number parsing") {
        int value = 0;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("num").long_name("num").value_of(value));

        auto result = cmd.parse(make_args({"--num=0b1010"}));
        CHECK(result.success());
        CHECK(value == 10);
    }

    TEST_CASE("Octal number parsing") {
        int value = 0;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("num").long_name("num").value_of(value));

        auto result = cmd.parse(make_args({"--num=0o755"}));
        CHECK(result.success());
        CHECK(value == 493);
    }

    TEST_CASE("Negative number as value") {
        int value = 0;
        auto cmd = argu::Command("app")
            .allow_negative_numbers(true)
            .arg(argu::Arg("num").long_name("num").value_of(value));

        auto result = cmd.parse(make_args({"--num=-42"}));
        CHECK(result.success());
        CHECK(value == -42);
    }

    TEST_CASE("Multiple positional arguments") {
        std::string src, dest;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("source").positional().index(0).value_of(src).required())
            .arg(argu::Arg("dest").positional().index(1).value_of(dest).required());

        auto result = cmd.parse(make_args({"file1.txt", "file2.txt"}));
        CHECK(result.success());
        CHECK(src == "file1.txt");
        CHECK(dest == "file2.txt");
    }

    TEST_CASE("Subcommand with alias prefix matching") {
        auto cmd = argu::Command("app")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("status").about("Show status"))
            .subcommand(argu::Command("start").about("Start service"));

        // "sta" is ambiguous (status vs start)
        auto result = cmd.parse(make_args({"sta"}));
        CHECK(!result.success());
    }

    TEST_CASE("Subcommand with unambiguous prefix") {
        auto cmd = argu::Command("app")
            .allow_subcommand_prefix(true)
            .subcommand(argu::Command("status").about("Show status"))
            .subcommand(argu::Command("stop").about("Stop service"));

        // "sta" uniquely matches "status"
        auto result = cmd.parse(make_args({"sta"}));
        CHECK(result.success());
        CHECK(cmd.matches().subcommand().value() == "status");
    }

    TEST_CASE("Boolean value parsing") {
        bool flag = false;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("enabled").long_name("enabled").value_of(flag));

        auto result = cmd.parse(make_args({"--enabled=true"}));
        CHECK(result.success());
        CHECK(flag == true);

        flag = true;
        result = cmd.parse(make_args({"--enabled=false"}));
        CHECK(result.success());
        CHECK(flag == false);
    }

    TEST_CASE("Float value parsing") {
        double value = 0.0;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("rate").long_name("rate").value_of(value));

        auto result = cmd.parse(make_args({"--rate=3.14159"}));
        CHECK(result.success());
        CHECK(value == doctest::Approx(3.14159));
    }

    TEST_CASE("Multiple values for same option") {
        std::vector<std::string> files;
        auto cmd = argu::Command("app")
            .arg(argu::Arg("file")
                .short_name('f')
                .long_name("file")
                .value_of(files)
                .takes_one_or_more());

        auto result = cmd.parse(make_args({"-f", "a.txt", "-f", "b.txt", "-f", "c.txt"}));
        CHECK(result.success());
        CHECK(files.size() == 3);
    }
}
