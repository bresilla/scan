/// @file test_argue_validators.cpp
/// @brief Extended tests for Argue validators

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <argu/argu.hpp>
#include <fstream>
#include <filesystem>

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
// File Validators Tests
// =============================================================================

TEST_SUITE("Argue File Validators") {

    TEST_CASE("file_exists validator accepts existing file") {
        // Create temp file
        std::string temp_path = "/tmp/argue_test_file.txt";
        std::ofstream ofs(temp_path);
        ofs << "test";
        ofs.close();

        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("file")
                .long_name("file")
                .value_of(path)
                .validate(argu::validators::file_exists()));

        auto result = cmd.parse(make_args({"--file", temp_path.c_str()}));
        CHECK(result.success());
        CHECK(path == temp_path);

        std::filesystem::remove(temp_path);
    }

    TEST_CASE("file_exists validator rejects missing file") {
        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("file")
                .long_name("file")
                .value_of(path)
                .validate(argu::validators::file_exists()));

        auto result = cmd.parse(make_args({"--file", "/nonexistent/path/file.txt"}));
        CHECK_FALSE(result.success());
    }

    TEST_CASE("dir_exists validator accepts existing directory") {
        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("dir")
                .long_name("dir")
                .value_of(path)
                .validate(argu::validators::dir_exists()));

        auto result = cmd.parse(make_args({"--dir", "/tmp"}));
        CHECK(result.success());
    }

    TEST_CASE("dir_exists validator rejects non-directory") {
        std::string temp_path = "/tmp/argue_test_file2.txt";
        std::ofstream ofs(temp_path);
        ofs << "test";
        ofs.close();

        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("dir")
                .long_name("dir")
                .value_of(path)
                .validate(argu::validators::dir_exists()));

        auto result = cmd.parse(make_args({"--dir", temp_path.c_str()}));
        CHECK_FALSE(result.success());

        std::filesystem::remove(temp_path);
    }

    TEST_CASE("path_not_exists validator accepts non-existing path") {
        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("out")
                .long_name("out")
                .value_of(path)
                .validate(argu::validators::path_not_exists()));

        auto result = cmd.parse(make_args({"--out", "/nonexistent/new_file.txt"}));
        CHECK(result.success());
    }

    TEST_CASE("path_not_exists validator rejects existing path") {
        std::string path;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("out")
                .long_name("out")
                .value_of(path)
                .validate(argu::validators::path_not_exists()));

        auto result = cmd.parse(make_args({"--out", "/tmp"}));
        CHECK_FALSE(result.success());
    }
}

// =============================================================================
// String Validators Tests
// =============================================================================

TEST_SUITE("Argue String Validators") {

    TEST_CASE("min_length validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::min_length(5)));

        SUBCASE("Accepts string meeting minimum") {
            auto result = cmd.parse(make_args({"--val", "hello"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects string below minimum") {
            auto result = cmd.parse(make_args({"--val", "hi"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("max_length validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::max_length(5)));

        SUBCASE("Accepts string meeting maximum") {
            auto result = cmd.parse(make_args({"--val", "hello"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects string above maximum") {
            auto result = cmd.parse(make_args({"--val", "hello world"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("alphanumeric validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::alphanumeric()));

        SUBCASE("Accepts alphanumeric string") {
            auto result = cmd.parse(make_args({"--val", "Hello123"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects string with special chars") {
            auto result = cmd.parse(make_args({"--val", "hello-world"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("alphabetic validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::alphabetic()));

        SUBCASE("Accepts alphabetic string") {
            auto result = cmd.parse(make_args({"--val", "HelloWorld"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects string with numbers") {
            auto result = cmd.parse(make_args({"--val", "Hello123"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("numeric validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::numeric()));

        SUBCASE("Accepts numeric string") {
            auto result = cmd.parse(make_args({"--val", "12345"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects string with letters") {
            auto result = cmd.parse(make_args({"--val", "123abc"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Network Validators Tests
// =============================================================================

TEST_SUITE("Argue Network Validators") {

    TEST_CASE("email validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("email")
                .long_name("email")
                .value_of(val)
                .validate(argu::validators::email()));

        SUBCASE("Accepts valid email") {
            auto result = cmd.parse(make_args({"--email", "user@example.com"}));
            CHECK(result.success());
        }

        SUBCASE("Accepts email with subdomain") {
            auto result = cmd.parse(make_args({"--email", "user@mail.example.com"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects invalid email") {
            auto result = cmd.parse(make_args({"--email", "not-an-email"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("url validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("url")
                .long_name("url")
                .value_of(val)
                .validate(argu::validators::url()));

        SUBCASE("Accepts http URL") {
            auto result = cmd.parse(make_args({"--url", "http://example.com"}));
            CHECK(result.success());
        }

        SUBCASE("Accepts https URL") {
            auto result = cmd.parse(make_args({"--url", "https://example.com/path"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects non-URL") {
            auto result = cmd.parse(make_args({"--url", "not-a-url"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("ipv4 validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("ip")
                .long_name("ip")
                .value_of(val)
                .validate(argu::validators::ipv4()));

        SUBCASE("Accepts valid IPv4") {
            auto result = cmd.parse(make_args({"--ip", "192.168.1.1"}));
            CHECK(result.success());
        }

        SUBCASE("Accepts localhost") {
            auto result = cmd.parse(make_args({"--ip", "127.0.0.1"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects invalid IPv4") {
            auto result = cmd.parse(make_args({"--ip", "256.1.1.1"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Rejects non-IP") {
            auto result = cmd.parse(make_args({"--ip", "not.an.ip.address"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Composite Validators Tests
// =============================================================================

TEST_SUITE("Argue Composite Validators") {

    TEST_CASE("all_of validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::all_of({
                    argu::validators::min_length(3),
                    argu::validators::max_length(10),
                    argu::validators::alphanumeric()
                })));

        SUBCASE("Accepts value passing all validators") {
            auto result = cmd.parse(make_args({"--val", "hello123"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects value failing length") {
            auto result = cmd.parse(make_args({"--val", "hi"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Rejects value failing alphanumeric") {
            auto result = cmd.parse(make_args({"--val", "hello-world"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("any_of validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::any_of({
                    argu::validators::email(),
                    argu::validators::ipv4()
                })));

        SUBCASE("Accepts email") {
            auto result = cmd.parse(make_args({"--val", "user@example.com"}));
            CHECK(result.success());
        }

        SUBCASE("Accepts IPv4") {
            auto result = cmd.parse(make_args({"--val", "192.168.1.1"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects value failing all") {
            auto result = cmd.parse(make_args({"--val", "neither"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("predicate validator") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val")
                .long_name("val")
                .value_of(val)
                .validate(argu::validators::predicate(
                    [](const std::string& s) { return s.find("test") != std::string::npos; },
                    "value must contain 'test'"
                )));

        SUBCASE("Accepts matching predicate") {
            auto result = cmd.parse(make_args({"--val", "my_test_value"}));
            CHECK(result.success());
        }

        SUBCASE("Rejects non-matching predicate") {
            auto result = cmd.parse(make_args({"--val", "other_value"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Edge Cases Tests
// =============================================================================

TEST_SUITE("Argue Edge Cases") {

    TEST_CASE("Empty command name") {
        auto cmd = argu::Command("");
        auto result = cmd.parse(make_args({}));
        CHECK(result.success());
    }

    TEST_CASE("Very long argument value") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val").long_name("val").value_of(val));

        std::string long_value(10000, 'x');
        auto result = cmd.parse(std::vector<std::string>{"--val", long_value});
        CHECK(result.success());
        CHECK(val.length() == 10000);
    }

    TEST_CASE("Argument with special characters") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val").long_name("val").value_of(val));

        auto result = cmd.parse(make_args({"--val", "hello\tworld\nwith\rspecial"}));
        CHECK(result.success());
    }

    TEST_CASE("Unicode in argument value") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val").long_name("val").value_of(val));

        auto result = cmd.parse(make_args({"--val", "Hello 世界 🌍"}));
        CHECK(result.success());
        CHECK(val == "Hello 世界 🌍");
    }

    TEST_CASE("Multiple equals signs in value") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val").long_name("val").value_of(val));

        auto result = cmd.parse(make_args({"--val=key=value=extra"}));
        CHECK(result.success());
        CHECK(val == "key=value=extra");
    }

    TEST_CASE("Empty value with equals") {
        std::string val = "default";
        auto cmd = argu::Command("test")
            .arg(argu::Arg("val").long_name("val").value_of(val));

        auto result = cmd.parse(make_args({"--val="}));
        CHECK(result.success());
        CHECK(val == "");
    }

    TEST_CASE("Numeric edge cases") {
        SUBCASE("Max int") {
            int val = 0;
            auto cmd = argu::Command("test")
                .arg(argu::Arg("n").long_name("n").value_of(val));

            cmd.parse(make_args({"--n", "2147483647"}));
            CHECK(val == 2147483647);
        }

        SUBCASE("Min int") {
            int val = 0;
            auto cmd = argu::Command("test")
                .allow_negative_numbers(true)
                .arg(argu::Arg("n").long_name("n").value_of(val));

            cmd.parse(make_args({"--n", "-2147483648"}));
            CHECK(val == -2147483648);
        }

        SUBCASE("Zero") {
            int val = 99;
            auto cmd = argu::Command("test")
                .arg(argu::Arg("n").long_name("n").value_of(val));

            cmd.parse(make_args({"--n", "0"}));
            CHECK(val == 0);
        }
    }

    TEST_CASE("Boolean parsing edge cases") {
        bool val = false;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("b").long_name("b").value_of(val));

        SUBCASE("yes") {
            cmd.parse(make_args({"--b", "yes"}));
            CHECK(val == true);
        }

        SUBCASE("no") {
            val = true;
            cmd.parse(make_args({"--b", "no"}));
            CHECK(val == false);
        }

        SUBCASE("on") {
            val = false;
            cmd.parse(make_args({"--b", "on"}));
            CHECK(val == true);
        }

        SUBCASE("off") {
            val = true;
            cmd.parse(make_args({"--b", "off"}));
            CHECK(val == false);
        }
    }
}

// =============================================================================
// Help Text Tests
// =============================================================================

TEST_SUITE("Argue Help Text") {

    TEST_CASE("Help includes version") {
        auto cmd = argu::Command("myapp").version("2.5.0");
        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("2.5.0") != std::string::npos);
    }

    TEST_CASE("Help includes description") {
        auto cmd = argu::Command("myapp").about("My awesome application");
        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("My awesome application") != std::string::npos);
    }

    TEST_CASE("Help includes argument help text") {
        auto cmd = argu::Command("myapp")
            .arg(argu::Arg("input")
                .long_name("input")
                .help("The input file to process"));

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("input file to process") != std::string::npos);
    }

    TEST_CASE("Help includes choices") {
        auto cmd = argu::Command("myapp")
            .arg(argu::Arg("format")
                .long_name("format")
                .choices({"json", "xml", "csv"}));

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("json") != std::string::npos);
        CHECK(result.message().find("xml") != std::string::npos);
        CHECK(result.message().find("csv") != std::string::npos);
    }

    TEST_CASE("Help includes default value") {
        auto cmd = argu::Command("myapp")
            .arg(argu::Arg("port")
                .long_name("port")
                .default_value("8080"));

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("8080") != std::string::npos);
    }

    TEST_CASE("Hidden arguments not in help") {
        auto cmd = argu::Command("myapp")
            .arg(argu::Arg("visible").long_name("visible").help("Visible option"))
            .arg(argu::Arg("hidden").long_name("hidden").help("Hidden option").hidden());

        auto result = cmd.parse(make_args({"--help"}));
        CHECK(result.message().find("visible") != std::string::npos);
        CHECK(result.message().find("Hidden option") == std::string::npos);
    }
}

// =============================================================================
// UUID Validator Tests
// =============================================================================

TEST_SUITE("UUID Validator") {

    TEST_CASE("Accepts valid UUID v4") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("uuid")
                .long_name("uuid")
                .value_of(val)
                .validate(argu::validators::uuid()));

        auto result = cmd.parse(make_args({"--uuid", "550e8400-e29b-41d4-a716-446655440000"}));
        CHECK(result.success());
    }

    TEST_CASE("Accepts lowercase UUID") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("uuid")
                .long_name("uuid")
                .value_of(val)
                .validate(argu::validators::uuid()));

        auto result = cmd.parse(make_args({"--uuid", "a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11"}));
        CHECK(result.success());
    }

    TEST_CASE("Accepts uppercase UUID") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("uuid")
                .long_name("uuid")
                .value_of(val)
                .validate(argu::validators::uuid()));

        auto result = cmd.parse(make_args({"--uuid", "A0EEBC99-9C0B-4EF8-BB6D-6BB9BD380A11"}));
        CHECK(result.success());
    }

    TEST_CASE("Rejects invalid UUID format") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("uuid")
                .long_name("uuid")
                .value_of(val)
                .validate(argu::validators::uuid()));

        SUBCASE("Missing dashes") {
            auto result = cmd.parse(make_args({"--uuid", "550e8400e29b41d4a716446655440000"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Too short") {
            auto result = cmd.parse(make_args({"--uuid", "550e8400-e29b-41d4-a716"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Invalid characters") {
            auto result = cmd.parse(make_args({"--uuid", "550e8400-e29b-41d4-a716-44665544000g"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Wrong dash positions") {
            auto result = cmd.parse(make_args({"--uuid", "550e840-0e29b-41d4-a716-446655440000"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Hex Color Validator Tests
// =============================================================================

TEST_SUITE("Hex Color Validator") {

    TEST_CASE("Accepts 6-digit hex color") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .value_of(val)
                .validate(argu::validators::hex_color()));

        SUBCASE("Black") {
            auto result = cmd.parse(make_args({"--color", "#000000"}));
            CHECK(result.success());
        }

        SUBCASE("White") {
            auto result = cmd.parse(make_args({"--color", "#FFFFFF"}));
            CHECK(result.success());
        }

        SUBCASE("Red") {
            auto result = cmd.parse(make_args({"--color", "#FF0000"}));
            CHECK(result.success());
        }

        SUBCASE("Lowercase") {
            auto result = cmd.parse(make_args({"--color", "#abcdef"}));
            CHECK(result.success());
        }
    }

    TEST_CASE("Accepts 3-digit hex color") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .value_of(val)
                .validate(argu::validators::hex_color()));

        SUBCASE("Short black") {
            auto result = cmd.parse(make_args({"--color", "#000"}));
            CHECK(result.success());
        }

        SUBCASE("Short white") {
            auto result = cmd.parse(make_args({"--color", "#FFF"}));
            CHECK(result.success());
        }

        SUBCASE("Short lowercase") {
            auto result = cmd.parse(make_args({"--color", "#abc"}));
            CHECK(result.success());
        }
    }

    TEST_CASE("Rejects invalid hex color") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("color")
                .long_name("color")
                .value_of(val)
                .validate(argu::validators::hex_color()));

        SUBCASE("Missing hash") {
            auto result = cmd.parse(make_args({"--color", "FF0000"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Wrong length") {
            auto result = cmd.parse(make_args({"--color", "#FFFF"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Invalid characters") {
            auto result = cmd.parse(make_args({"--color", "#GGGGGG"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Color name instead of hex") {
            auto result = cmd.parse(make_args({"--color", "red"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// JSON Validator Tests
// =============================================================================

TEST_SUITE("JSON Validators") {

    TEST_CASE("json_object accepts valid object") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("json")
                .long_name("json")
                .value_of(val)
                .validate(argu::validators::json_object()));

        SUBCASE("Simple object") {
            auto result = cmd.parse(make_args({"--json", "{}"}));
            CHECK(result.success());
        }

        SUBCASE("Object with content") {
            auto result = cmd.parse(make_args({"--json", R"({"key": "value"})"}));
            CHECK(result.success());
        }

        SUBCASE("Nested object") {
            auto result = cmd.parse(make_args({"--json", R"({"a": {"b": 1}})"}));
            CHECK(result.success());
        }
    }

    TEST_CASE("json_object rejects non-objects") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("json")
                .long_name("json")
                .value_of(val)
                .validate(argu::validators::json_object()));

        SUBCASE("Array") {
            auto result = cmd.parse(make_args({"--json", "[]"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("String") {
            auto result = cmd.parse(make_args({"--json", R"("hello")"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Plain text") {
            auto result = cmd.parse(make_args({"--json", "not json"}));
            CHECK_FALSE(result.success());
        }
    }

    TEST_CASE("json_array accepts valid array") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("json")
                .long_name("json")
                .value_of(val)
                .validate(argu::validators::json_array()));

        SUBCASE("Empty array") {
            auto result = cmd.parse(make_args({"--json", "[]"}));
            CHECK(result.success());
        }

        SUBCASE("Array with elements") {
            auto result = cmd.parse(make_args({"--json", R"([1, 2, 3])"}));
            CHECK(result.success());
        }

        SUBCASE("Array with mixed types") {
            auto result = cmd.parse(make_args({"--json", R"([1, "two", true])"}));
            CHECK(result.success());
        }
    }

    TEST_CASE("json_array rejects non-arrays") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("json")
                .long_name("json")
                .value_of(val)
                .validate(argu::validators::json_array()));

        SUBCASE("Object") {
            auto result = cmd.parse(make_args({"--json", "{}"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("String") {
            auto result = cmd.parse(make_args({"--json", R"("hello")"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Plain text") {
            auto result = cmd.parse(make_args({"--json", "not json"}));
            CHECK_FALSE(result.success());
        }
    }
}

// =============================================================================
// Semantic Version Validator Tests
// =============================================================================

TEST_SUITE("Semantic Version Validator") {

    TEST_CASE("semver accepts valid versions") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("version")
                .long_name("version")
                .value_of(val)
                .validate(argu::validators::semver()));

        SUBCASE("Simple version") {
            auto result = cmd.parse(make_args({"--version", "1.0.0"}));
            CHECK(result.success());
        }

        SUBCASE("Two-digit version") {
            auto result = cmd.parse(make_args({"--version", "12.34.56"}));
            CHECK(result.success());
        }

        SUBCASE("Zero version") {
            auto result = cmd.parse(make_args({"--version", "0.0.0"}));
            CHECK(result.success());
        }

        SUBCASE("Pre-release version") {
            auto result = cmd.parse(make_args({"--version", "1.0.0-alpha"}));
            CHECK(result.success());
        }

        SUBCASE("Build metadata") {
            auto result = cmd.parse(make_args({"--version", "1.0.0+build.123"}));
            CHECK(result.success());
        }
    }

    TEST_CASE("semver rejects invalid versions") {
        std::string val;
        auto cmd = argu::Command("test")
            .arg(argu::Arg("version")
                .long_name("version")
                .value_of(val)
                .validate(argu::validators::semver()));

        SUBCASE("Missing patch") {
            auto result = cmd.parse(make_args({"--version", "1.0"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Leading v") {
            auto result = cmd.parse(make_args({"--version", "v1.0.0"}));
            CHECK_FALSE(result.success());
        }

        SUBCASE("Non-numeric") {
            auto result = cmd.parse(make_args({"--version", "1.a.0"}));
            CHECK_FALSE(result.success());
        }
    }
}
