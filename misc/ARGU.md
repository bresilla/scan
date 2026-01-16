# Argu - Command Line Argument Parser

**Comprehensive Documentation**

A modern C++20 header-only argument parsing library inspired by Rust's [CLAP](https://github.com/clap-rs/clap) and Python's [argparse](https://docs.python.org/3/library/argparse.html). Argu provides a powerful, type-safe, and ergonomic API for building command-line interfaces with rich features including subcommands, validators, shell completions, and more.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Core Concepts](#core-concepts)
   - [Commands](#commands)
   - [Arguments](#arguments)
   - [Parsing](#parsing)
6. [Argument Types](#argument-types)
   - [Options](#options)
   - [Flags](#flags)
   - [Positional Arguments](#positional-arguments)
   - [Counting Flags](#counting-flags)
7. [Subcommands](#subcommands)
8. [Value Handling](#value-handling)
9. [Validators](#validators)
10. [Argument Relations](#argument-relations)
11. [Configuration](#configuration)
12. [Shell Completions](#shell-completions)
13. [Help & Display](#help--display)
14. [Error Handling](#error-handling)
15. [Human-Friendly Parsers](#human-friendly-parsers)
16. [Advanced Topics](#advanced-topics)
17. [API Reference](#api-reference)
18. [Examples](#examples)
19. [Comparison with Other Libraries](#comparison-with-other-libraries)

---

## Overview

Argu is designed to make command-line argument parsing in C++ as ergonomic and powerful as in languages like Rust or Python. It provides:

- **Builder Pattern API**: Fluent, chainable method calls for defining arguments
- **Type Safety**: Compile-time type checking with automatic value binding
- **Rich Validation**: 40+ built-in validators with custom validator support
- **Subcommands**: Deep nesting support (4+ levels) with prefix matching
- **Configuration**: Environment variables and config file support
- **Shell Completions**: Generate completions for Bash, Zsh, Fish, PowerShell, Elvish
- **Beautiful Help**: Colored output with customizable themes

### Design Philosophy

1. **CLAP-like API**: Familiar to Rust developers
2. **Type Safety**: Leverage C++20 for compile-time guarantees
3. **Zero Runtime Cost**: No dynamic allocations in the hot path
4. **Composability**: Mix and match features freely

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Code                         │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   Command    │  │     Arg      │  │    ArgGroup      │  │
│  │  .name()     │  │  .short()    │  │  .mutex()        │  │
│  │  .about()    │  │  .long()     │  │  .required()     │  │
│  │  .subcommand │  │  .value_of() │  │  .at_least_one() │  │
│  │  .arg()      │  │  .validate() │  │                  │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  Validators  │  │  Completions │  │   Help/Themes    │  │
│  │  40+ built-in│  │  Bash, Zsh   │  │  Colored output  │  │
│  │  Custom      │  │  Fish, PS    │  │  Custom themes   │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   Parser     │  │    Config    │  │   Duration/Size  │  │
│  │  Tokenizer   │  │  TOML/INI    │  │   Parsers        │  │
│  │  Validation  │  │  Env vars    │  │   Formatters     │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

## Features

### Core Features

| Feature | Description |
|---------|-------------|
| **Header-only** | Single include, no build required |
| **C++20** | Modern C++ with concepts and ranges |
| **Builder Pattern** | Fluent, chainable API |
| **Type-safe Binding** | Automatic value conversion |
| **Subcommands** | Deep nesting with prefix matching |
| **Validators** | 40+ built-in validators |
| **Shell Completions** | 5 shell generators |
| **Config Files** | TOML/INI support with layering |
| **Environment Variables** | With prefix support |
| **Colored Help** | Themes and customization |
| **Error Suggestions** | Typo correction |

### Feature Comparison

| Feature | argu | CLI11 | cxxopts | CLAP (Rust) |
|---------|:----:|:-----:|:-------:|:-----------:|
| Header-only | ✅ | ✅ | ✅ | N/A |
| Subcommands | ✅ | ✅ | ❌ | ✅ |
| Deep nesting (4+) | ✅ | ✅ | ❌ | ✅ |
| Prefix matching | ✅ | ❌ | ❌ | ✅ |
| Config files | ✅ | ✅ | ❌ | ✅ |
| Env variables | ✅ | ✅ | ❌ | ✅ |
| Shell completions | ✅ | ❌ | ❌ | ✅ |
| Colored help | ✅ | ❌ | ❌ | ✅ |
| Typo suggestions | ✅ | ❌ | ❌ | ✅ |
| Value hints | ✅ | ❌ | ❌ | ✅ |
| Negatable flags | ✅ | ❌ | ❌ | ✅ |
| Arg groups | ✅ | ✅ | ❌ | ✅ |
| Duration parsing | ✅ | ❌ | ❌ | ✅ |
| Size parsing | ✅ | ❌ | ❌ | ❌ |
| Error aggregation | ✅ | ❌ | ❌ | ✅ |

---

## Installation

### Using CMake FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    scan
    GIT_REPOSITORY https://github.com/bresilla/scan.git
    GIT_TAG main
)
FetchContent_MakeAvailable(scan)

target_link_libraries(your_target PRIVATE scan::scan)
```

### Manual Installation

1. Copy `include/argu` directory to your project
2. Include the header:

```cpp
#include <argu/argu.hpp>
```

### Requirements

- **C++20** compiler (GCC 11+, Clang 14+, MSVC 2022+)

---

## Quick Start

### Hello World

```cpp
#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string name = "World";

    auto cmd = argu::Command("hello")
        .about("A simple greeting program")
        .arg(argu::Arg("name")
            .short_name('n')
            .long_name("name")
            .help("Name to greet")
            .value_of(name)
            .default_value("World"));

    auto result = cmd.parse(argc, argv);
    if (!result.success()) {
        return result.exit();
    }

    std::cout << "Hello, " << name << "!\n";
    return 0;
}
```

Usage:
```bash
$ ./hello
Hello, World!

$ ./hello --name Alice
Hello, Alice!

$ ./hello -n Bob
Hello, Bob!
```

### With Subcommands

```cpp
#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string filename;
    bool verbose = false;

    auto cmd = argu::Command("myapp")
        .version("1.0.0")
        .about("A file management tool")
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Enable verbose output")
            .flag(verbose)
            .global())
        .subcommand(
            argu::Command("copy")
                .about("Copy a file")
                .arg(argu::Arg("file")
                    .positional()
                    .help("Source file")
                    .value_of(filename)
                    .required()))
        .subcommand(
            argu::Command("move")
                .about("Move a file")
                .arg(argu::Arg("file")
                    .positional()
                    .help("Source file")
                    .value_of(filename)
                    .required()));

    auto result = cmd.parse(argc, argv);
    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();
    if (auto sub = matches.subcommand()) {
        if (*sub == "copy") {
            std::cout << "Copying " << filename;
        } else if (*sub == "move") {
            std::cout << "Moving " << filename;
        }
        if (verbose) std::cout << " (verbose)";
        std::cout << "\n";
    }

    return 0;
}
```

---

## Core Concepts

### Commands

A `Command` represents a CLI application or subcommand. It contains:
- Metadata (name, version, about)
- Arguments
- Subcommands
- Configuration options

```cpp
auto cmd = argu::Command("myapp")
    .version("1.0.0")
    .about("Short description")
    .long_about("Detailed description spanning multiple lines")
    .author("Your Name <email@example.com>")
    .after_help("Examples:\n  myapp --help")
    .before_help("IMPORTANT: Read this first");
```

#### Command Options

| Method | Description |
|--------|-------------|
| `.version(str)` | Set version string |
| `.about(str)` | Short description |
| `.long_about(str)` | Detailed description |
| `.author(str)` | Author information |
| `.before_help(str)` | Text before help |
| `.after_help(str)` | Text after help |
| `.alias(str)` | Add command alias |
| `.hidden()` | Hide from help |
| `.color(ColorMode)` | Color output mode |
| `.theme(HelpTheme)` | Help theme |

### Arguments

An `Arg` represents a command-line argument. Arguments can be:
- **Options**: `-o value` or `--option value`
- **Flags**: `-f` or `--flag`
- **Positional**: `value` (no prefix)

```cpp
auto arg = argu::Arg("output")
    .short_name('o')
    .long_name("output")
    .help("Output file path")
    .value_of(output_path)
    .required();
```

#### Argument Options

| Method | Description |
|--------|-------------|
| `.short_name(char)` | Short option (-o) |
| `.long_name(str)` | Long option (--output) |
| `.help(str)` | Help text |
| `.value_of(var)` | Bind to variable |
| `.required()` | Mark as required |
| `.default_value(val)` | Default value |
| `.positional()` | Positional argument |
| `.flag(bool&)` | Boolean flag |
| `.count(int&)` | Counting flag |
| `.validate(validator)` | Add validator |
| `.choices({...})` | Restrict values |

### Parsing

```cpp
// Parse from argc/argv
auto result = cmd.parse(argc, argv);

// Parse from vector
std::vector<std::string> args = {"--verbose", "file.txt"};
auto result = cmd.parse(args);

// Check result
if (!result.success()) {
    std::cerr << result.message() << "\n";
    return result.exit_code();
}

// Or use exit() which prints message and returns code
if (!result.success()) {
    return result.exit();
}
```

#### ParseResult

| Method | Returns | Description |
|--------|---------|-------------|
| `.success()` | `bool` | Parse succeeded |
| `.message()` | `string` | Help/error message |
| `.exit_code()` | `int` | Exit code |
| `.exit()` | `int` | Print message and return code |

---

## Argument Types

### Options

Options take values and have `-` or `--` prefixes.

```cpp
std::string output;
int count;

auto cmd = argu::Command("app")
    // Short and long option
    .arg(argu::Arg("output")
        .short_name('o')
        .long_name("output")
        .value_of(output))

    // Long option only
    .arg(argu::Arg("count")
        .long_name("count")
        .value_of(count)
        .default_value("10"));
```

Usage:
```bash
$ app -o file.txt --count 5
$ app --output=file.txt --count=5
```

### Flags

Flags are boolean switches that don't take values.

```cpp
bool verbose = false;
bool quiet = false;

auto cmd = argu::Command("app")
    .arg(argu::Arg("verbose")
        .short_name('v')
        .long_name("verbose")
        .help("Enable verbose output")
        .flag(verbose))

    .arg(argu::Arg("quiet")
        .short_name('q')
        .long_name("quiet")
        .help("Suppress output")
        .flag(quiet)
        .conflicts_with("verbose"));
```

Usage:
```bash
$ app -v
$ app --verbose
$ app -vq  # Error: conflicts
```

#### Negatable Flags

```cpp
bool color = true;

auto cmd = argu::Command("app")
    .arg(argu::Arg("color")
        .long_name("color")
        .help("Enable colored output")
        .flag(color)
        .negatable());  // Allows --no-color
```

Usage:
```bash
$ app --color      # color = true
$ app --no-color   # color = false
```

### Positional Arguments

Arguments without `-` or `--` prefix.

```cpp
std::string source;
std::string dest;

auto cmd = argu::Command("cp")
    .arg(argu::Arg("source")
        .positional()
        .index(0)
        .help("Source file")
        .value_of(source)
        .required())

    .arg(argu::Arg("dest")
        .positional()
        .index(1)
        .help("Destination")
        .value_of(dest)
        .required());
```

Usage:
```bash
$ cp source.txt dest.txt
```

#### Multiple Positional Values

```cpp
std::vector<std::string> files;

auto cmd = argu::Command("cat")
    .arg(argu::Arg("files")
        .positional()
        .help("Files to concatenate")
        .value_of(files)
        .takes_one_or_more()
        .required());
```

Usage:
```bash
$ cat file1.txt file2.txt file3.txt
```

### Counting Flags

Count occurrences of a flag.

```cpp
int verbosity = 0;

auto cmd = argu::Command("app")
    .arg(argu::Arg("verbose")
        .short_name('v')
        .help("Increase verbosity (can be used multiple times)")
        .count(verbosity));
```

Usage:
```bash
$ app -v       # verbosity = 1
$ app -vv      # verbosity = 2
$ app -vvvv    # verbosity = 4
$ app -v -v -v # verbosity = 3
```

---

## Subcommands

### Basic Subcommands

```cpp
auto cmd = argu::Command("git")
    .subcommand(
        argu::Command("clone")
            .about("Clone a repository")
            .arg(argu::Arg("url").positional().required()))
    .subcommand(
        argu::Command("commit")
            .about("Record changes")
            .arg(argu::Arg("message")
                .short_name('m')
                .required()));
```

### Nested Subcommands (4+ Levels)

```cpp
auto cmd = argu::Command("kubectl")
    .subcommand(
        argu::Command("config")
            .about("Modify kubeconfig files")
            .subcommand(
                argu::Command("view")
                    .about("Display merged kubeconfig")
                    .subcommand(
                        argu::Command("users")
                            .about("Display users"))
                    .subcommand(
                        argu::Command("clusters")
                            .about("Display clusters"))));
```

Usage:
```bash
$ kubectl config view users
```

### Subcommand Prefix Matching

Enable matching subcommands by prefix:

```cpp
auto cmd = argu::Command("git")
    .allow_subcommand_prefix(true)
    .subcommand(argu::Command("status"))
    .subcommand(argu::Command("stash"));
```

Usage:
```bash
$ git stat   # Matches "status" (unambiguous)
$ git sta    # Error: ambiguous (status or stash?)
```

### Required Subcommands

```cpp
auto cmd = argu::Command("app")
    .subcommand_required(true)
    .subcommand(argu::Command("run"))
    .subcommand(argu::Command("build"));
```

### Global Arguments

Arguments that propagate to all subcommands:

```cpp
bool verbose = false;

auto cmd = argu::Command("app")
    .arg(argu::Arg("verbose")
        .short_name('v')
        .flag(verbose)
        .global())  // Available in all subcommands
    .subcommand(
        argu::Command("run")
            .about("Run the app"));
```

Usage:
```bash
$ app -v run        # Works: verbose set before subcommand
$ app run -v        # Works: verbose works in subcommand too
```

### External Subcommands

Support plugin-style external commands:

```cpp
auto cmd = argu::Command("docker")
    .allow_external_subcommands(true)
    .subcommand(argu::Command("run"))
    .subcommand(argu::Command("build"));

// ...

auto& matches = cmd.matches();
if (matches.is_external_subcommand()) {
    auto sub = *matches.subcommand();  // "compose"
    auto args = matches.external_args();  // ["up", "-d"]
    // Execute: docker-compose up -d
}
```

Usage:
```bash
$ docker compose up -d  # External: docker-compose
```

### Subcommand Chain

Get the full chain of matched subcommands:

```cpp
auto& matches = cmd.matches();
auto chain = matches.subcommand_chain();
// For "kubectl config view users": ["config", "view", "users"]
```

---

## Value Handling

### Default Values

```cpp
std::string format = "json";

auto arg = argu::Arg("format")
    .long_name("format")
    .value_of(format)
    .default_value("json");
```

### Implicit Values

Value used when option is present without a value:

```cpp
std::string log_level;

auto arg = argu::Arg("log")
    .long_name("log")
    .value_of(log_level)
    .implicit_value("info")
    .default_value("off");
```

Usage:
```bash
$ app              # log_level = "off"
$ app --log        # log_level = "info"
$ app --log=debug  # log_level = "debug"
```

### Choices (Enums)

```cpp
std::string format;

auto arg = argu::Arg("format")
    .long_name("format")
    .value_of(format)
    .choices({"json", "yaml", "xml", "csv"});
```

### Value Delimiters

Parse multiple values from a single argument:

```cpp
std::vector<std::string> tags;

auto arg = argu::Arg("tags")
    .long_name("tags")
    .value_of(tags)
    .value_delimiter(',');
```

Usage:
```bash
$ app --tags=a,b,c,d  # tags = ["a", "b", "c", "d"]
```

### Multiple Values

```cpp
std::vector<std::string> files;

auto arg = argu::Arg("files")
    .short_name('f')
    .value_of(files)
    .takes_one_or_more();  // Or: takes_multiple()
```

Usage:
```bash
$ app -f file1.txt -f file2.txt -f file3.txt
```

### Number Formats

Argu supports various number formats:

```cpp
int num;

auto arg = argu::Arg("num")
    .long_name("num")
    .value_of(num);
```

Usage:
```bash
$ app --num=42        # Decimal
$ app --num=0xFF      # Hexadecimal (255)
$ app --num=0o755     # Octal (493)
$ app --num=0b1010    # Binary (10)
$ app --num=-42       # Negative
```

### Type Binding

Argu automatically converts values to the bound variable's type:

```cpp
std::string str_val;
int int_val;
double double_val;
bool bool_val;
std::vector<std::string> vec_val;
std::vector<int> int_vec;

auto cmd = argu::Command("app")
    .arg(argu::Arg("str").value_of(str_val))
    .arg(argu::Arg("int").value_of(int_val))
    .arg(argu::Arg("double").value_of(double_val))
    .arg(argu::Arg("bool").value_of(bool_val))
    .arg(argu::Arg("vec").value_of(vec_val))
    .arg(argu::Arg("ints").value_of(int_vec));
```

Supported types:
- `std::string`
- `int`, `long`, `long long`
- `unsigned int`, `std::size_t`
- `float`, `double`
- `bool`
- `std::vector<T>` for any supported T

---

## Validators

Argu provides 40+ built-in validators in the `argu::validators` namespace.

### File System Validators

```cpp
.validate(argu::validators::file_exists())
.validate(argu::validators::dir_exists())
.validate(argu::validators::path_not_exists())
.validate(argu::validators::parent_exists())
```

### Numeric Validators

```cpp
.validate(argu::validators::range(0, 100))
.validate(argu::validators::positive())
.validate(argu::validators::non_negative())
```

### String Validators

```cpp
.validate(argu::validators::min_length(3))
.validate(argu::validators::max_length(50))
.validate(argu::validators::length(3, 50))
.validate(argu::validators::non_empty())
.validate(argu::validators::alphanumeric())
.validate(argu::validators::alphabetic())
.validate(argu::validators::numeric())
.validate(argu::validators::starts_with("prefix"))
.validate(argu::validators::ends_with("suffix"))
.validate(argu::validators::contains("substring"))
.validate(argu::validators::matches(std::regex("pattern")))
```

### Network Validators

```cpp
.validate(argu::validators::ipv4())
.validate(argu::validators::ipv6())
.validate(argu::validators::hostname())
.validate(argu::validators::host_port())
.validate(argu::validators::port())
.validate(argu::validators::non_privileged_port())
.validate(argu::validators::url())
.validate(argu::validators::email())
```

### Format Validators

```cpp
.validate(argu::validators::uuid())
.validate(argu::validators::hex_color())
.validate(argu::validators::semver())
.validate(argu::validators::json_object())
.validate(argu::validators::json_array())
```

### Human-Friendly Validators

```cpp
.validate(argu::validators::duration())  // "5s", "10m", "2h"
.validate(argu::validators::size())      // "10KB", "5MB", "1GiB"
```

### Composite Validators

```cpp
// All must pass
.validate(argu::validators::all_of({
    argu::validators::non_empty(),
    argu::validators::max_length(100)
}))

// At least one must pass
.validate(argu::validators::any_of({
    argu::validators::ipv4(),
    argu::validators::ipv6()
}))

// Custom predicate
.validate(argu::validators::predicate(
    [](const std::string& val) {
        return val.size() > 0 && val[0] == '/';
    },
    "Must start with /"
))
```

### Custom Validators

```cpp
argu::Validator my_validator = [](const std::string& value)
    -> std::optional<std::string>
{
    if (/* valid */) {
        return std::nullopt;  // No error
    }
    return "Error message";  // Validation failed
};

.validate(my_validator)
```

### Complete Validator Example

```cpp
auto cmd = argu::Command("server")
    .arg(argu::Arg("host")
        .long_name("host")
        .value_of(host)
        .default_value("localhost")
        .validate(argu::validators::any_of({
            argu::validators::ipv4(),
            argu::validators::hostname()
        })))

    .arg(argu::Arg("port")
        .short_name('p')
        .long_name("port")
        .value_of(port)
        .default_value("8080")
        .validate(argu::validators::non_privileged_port()))

    .arg(argu::Arg("timeout")
        .long_name("timeout")
        .help("Connection timeout (e.g., 30s, 5m)")
        .value_of(timeout)
        .default_value("30s")
        .validate(argu::validators::duration()))

    .arg(argu::Arg("max-size")
        .long_name("max-size")
        .help("Maximum upload size (e.g., 10MB, 1GiB)")
        .value_of(max_size)
        .default_value("10MB")
        .validate(argu::validators::size()));
```

---

## Argument Relations

### Conflicts

Arguments that cannot be used together:

```cpp
bool verbose = false;
bool quiet = false;

auto cmd = argu::Command("app")
    .arg(argu::Arg("verbose")
        .short_name('v')
        .flag(verbose)
        .conflicts_with("quiet"))
    .arg(argu::Arg("quiet")
        .short_name('q')
        .flag(quiet)
        .conflicts_with("verbose"));
```

### Requirements

Arguments that require other arguments:

```cpp
std::string username;
std::string password;

auto cmd = argu::Command("app")
    .arg(argu::Arg("password")
        .long_name("password")
        .value_of(password)
        .requires_arg("username"))  // If password is used, username is required
    .arg(argu::Arg("username")
        .long_name("username")
        .value_of(username));
```

### Conditional Requirements

#### required_unless

Argument is required unless another argument is present:

```cpp
std::string input;
bool use_stdin = false;

auto cmd = argu::Command("app")
    .arg(argu::Arg("input")
        .positional()
        .value_of(input)
        .required_unless("stdin"))
    .arg(argu::Arg("stdin")
        .long_name("stdin")
        .flag(use_stdin));
```

#### required_if_eq

Argument is required if another argument has a specific value:

```cpp
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
        .required_if_eq("format", "csv"));  // Output required when format is CSV
```

#### requires_if

Argument requires another when it has a specific value:

```cpp
std::string mode;
std::string config;

auto cmd = argu::Command("app")
    .arg(argu::Arg("mode")
        .long_name("mode")
        .value_of(mode)
        .requires_if("config", "custom"))  // When mode=custom, config is required
    .arg(argu::Arg("config")
        .long_name("config")
        .value_of(config));
```

#### default_value_if

Set conditional default based on another argument:

```cpp
std::string format;
std::string extension;

auto cmd = argu::Command("app")
    .arg(argu::Arg("format")
        .long_name("format")
        .value_of(format))
    .arg(argu::Arg("extension")
        .long_name("ext")
        .value_of(extension)
        .default_value_if("format", "json", ".json")
        .default_value_if("format", "csv", ".csv")
        .default_value(".txt"));
```

### Argument Groups

#### Mutually Exclusive Group

Only one argument from the group can be used:

```cpp
std::string input_file;
std::string input_url;
bool use_stdin = false;

auto cmd = argu::Command("app")
    .arg(argu::Arg("file").long_name("file").value_of(input_file))
    .arg(argu::Arg("url").long_name("url").value_of(input_url))
    .arg(argu::Arg("stdin").long_name("stdin").flag(use_stdin))
    .group(argu::ArgGroup("input")
        .mutually_exclusive()
        .args({"file", "url", "stdin"}));
```

#### Required Together Group

All arguments in the group must be used together:

```cpp
std::string username;
std::string password;

auto cmd = argu::Command("app")
    .arg(argu::Arg("username").long_name("username").value_of(username))
    .arg(argu::Arg("password").long_name("password").value_of(password))
    .group(argu::ArgGroup("auth")
        .required_together()
        .args({"username", "password"}));
```

#### At Least One Required Group

At least one argument from the group must be used:

```cpp
auto cmd = argu::Command("app")
    .arg(argu::Arg("input").positional())
    .arg(argu::Arg("stdin").long_name("stdin").flag(use_stdin))
    .group(argu::ArgGroup("input_source")
        .at_least_one()
        .required()
        .args({"input", "stdin"}));
```

---

## Configuration

### Environment Variables

```cpp
std::string api_key;
std::string host;

auto cmd = argu::Command("app")
    .arg(argu::Arg("api-key")
        .long_name("api-key")
        .value_of(api_key)
        .env("API_KEY"))  // Read from API_KEY env var
    .arg(argu::Arg("host")
        .long_name("host")
        .value_of(host)
        .env("SERVER_HOST"));
```

### Environment Prefix

```cpp
auto cmd = argu::Command("myapp")
    .env_prefix("MYAPP_")  // All args check MYAPP_<NAME>
    .arg(argu::Arg("host").long_name("host").value_of(host))
    .arg(argu::Arg("port").long_name("port").value_of(port));

// Reads from MYAPP_HOST and MYAPP_PORT
```

### Config Files

```cpp
auto cmd = argu::Command("app")
    .config_file("~/.myapp/config.toml")
    .arg(argu::Arg("setting").long_name("setting").value_of(value));
```

Config file example (TOML):
```toml
setting = "value"
verbose = true
count = 42
```

### Value Source Priority

Values are resolved in this order (highest to lowest priority):
1. **Command Line** - Direct argument
2. **Environment Variable** - If env() is set
3. **Config File** - If config_file() is set
4. **Default Value** - If default_value() is set

Check source:
```cpp
auto& matches = cmd.matches();
auto source = matches.get_source("option");
// ValueSource::CommandLine, Environment, ConfigFile, or Default
```

---

## Shell Completions

### Generating Completions

```cpp
auto cmd = argu::Command("myapp")
    .arg(argu::Arg("generate-completions")
        .long_name("completions")
        .help("Generate shell completions")
        .choices({"bash", "zsh", "fish", "powershell", "elvish"}));

auto result = cmd.parse(argc, argv);
if (!result.success() || !result.message().empty()) {
    return result.exit();
}

auto& matches = cmd.matches();
if (auto shell = matches.get_one("generate-completions")) {
    if (*shell == "bash") {
        std::cout << argu::completions::generate_bash(cmd);
    } else if (*shell == "zsh") {
        std::cout << argu::completions::generate_zsh(cmd);
    } else if (*shell == "fish") {
        std::cout << argu::completions::generate_fish(cmd);
    } else if (*shell == "powershell") {
        std::cout << argu::completions::generate_powershell(cmd);
    } else if (*shell == "elvish") {
        std::cout << argu::completions::generate_elvish(cmd);
    }
    return 0;
}
```

### Value Hints

Provide hints for shell completions:

```cpp
.arg(argu::Arg("file")
    .hint_file())        // Complete with files

.arg(argu::Arg("dir")
    .hint_dir())         // Complete with directories

.arg(argu::Arg("host")
    .hint_hostname())    // Complete with hostnames

.arg(argu::Arg("user")
    .hint_username())    // Complete with usernames

.arg(argu::Arg("url")
    .hint_url())         // URL format

.arg(argu::Arg("email")
    .hint_email())       // Email format

.arg(argu::Arg("cmd")
    .hint_command())     // Complete with commands
```

### Custom Completers

```cpp
auto file_completer = [](const std::string& prefix)
    -> std::vector<std::string>
{
    std::vector<std::string> completions;
    // Generate completions based on prefix
    return completions;
};

.arg(argu::Arg("file")
    .completer(file_completer))
```

### Installing Completions

**Bash:**
```bash
myapp --completions bash > ~/.local/share/bash-completion/completions/myapp
```

**Zsh:**
```bash
myapp --completions zsh > ~/.zfunc/_myapp
# Add to .zshrc: fpath+=~/.zfunc
```

**Fish:**
```bash
myapp --completions fish > ~/.config/fish/completions/myapp.fish
```

**PowerShell:**
```powershell
myapp --completions powershell | Out-File -Encoding ASCII $PROFILE.CurrentUserCurrentHost
```

---

## Help & Display

### Automatic Help

Argu automatically adds `-h, --help` flags:

```bash
$ myapp --help
myapp 1.0.0
A description of my application

Usage: myapp [OPTIONS] <COMMAND>

Commands:
  run     Run the application
  build   Build the project

Options:
  -v, --verbose   Enable verbose output
  -h, --help      Print help information
  -V, --version   Print version information
```

### Color Modes

```cpp
auto cmd = argu::Command("app")
    .color(argu::ColorMode::Auto)    // Auto-detect (default)
    .color(argu::ColorMode::Always)  // Always use colors
    .color(argu::ColorMode::Never);  // Never use colors
```

### Help Themes

```cpp
auto cmd = argu::Command("app")
    .theme(argu::HelpTheme::default_theme())  // Default colors
    .theme(argu::HelpTheme::minimal())        // Minimal (mostly white)
    .theme(argu::HelpTheme::colorful())       // Vibrant colors
    .theme(argu::HelpTheme::dark());          // Dark theme
```

### Custom Theme

```cpp
argu::HelpTheme custom;
custom.command_name = argu::Color::term_cyan();
custom.option_name = argu::Color::term_green();
custom.positional = argu::Color::term_yellow();
custom.subcommand = argu::Color::term_blue();
custom.section_header = argu::Color::term_yellow();
custom.use_bold_headers = true;

auto cmd = argu::Command("app")
    .theme(custom);
```

### Custom Help Sections

```cpp
auto cmd = argu::Command("app")
    .about("My application")
    .before_help("NOTICE: This is important")
    .after_help(R"(
EXAMPLES:
    myapp --verbose run
    myapp build --release

ENVIRONMENT VARIABLES:
    MYAPP_DEBUG    Enable debug mode
    MYAPP_CONFIG   Config file path

For more information, visit https://example.com
)");
```

### Argument Groups in Help

```cpp
auto cmd = argu::Command("app")
    .arg(argu::Arg("input").positional().group("Input"))
    .arg(argu::Arg("output").long_name("output").group("Output"))
    .arg(argu::Arg("format").long_name("format").group("Output"))
    .arg(argu::Arg("verbose").long_name("verbose").group("Logging"))
    .arg(argu::Arg("quiet").long_name("quiet").group("Logging"));
```

Output:
```
Input:
  <input>           Input file

Output:
  -o, --output      Output file
  -f, --format      Output format

Logging:
  -v, --verbose     Enable verbose output
  -q, --quiet       Suppress output
```

### Hidden Arguments

```cpp
.arg(argu::Arg("debug-internal")
    .long_name("debug-internal")
    .hidden())  // Not shown in help
```

---

## Error Handling

### Error Types

Argu provides specific error types:

| Error Type | Description |
|------------|-------------|
| `UnknownOptionError` | Unknown option provided |
| `MissingRequiredError` | Required argument missing |
| `MissingValueError` | Option missing required value |
| `InvalidValueError` | Value failed validation |
| `ConflictError` | Conflicting arguments used |
| `DependencyError` | Required dependency missing |
| `MutexGroupError` | Multiple mutex group args used |
| `ValidationError` | Validator returned error |
| `AggregatedErrors` | Multiple errors (aggregate mode) |

### Error Aggregation

Collect all errors instead of stopping at first:

```cpp
auto cmd = argu::Command("app")
    .error_mode(argu::ErrorMode::Aggregate)
    .arg(argu::Arg("input").required())
    .arg(argu::Arg("output").required());

// If both missing, shows:
// Multiple errors occurred (2):
//   1. Missing required argument: input
//   2. Missing required argument: output
```

### Typo Suggestions

Argu suggests correct options for typos:

```bash
$ myapp --verbos
error: Unknown option '--verbos'

Did you mean '--verbose'?
```

Configure suggestion threshold:

```cpp
auto cmd = argu::Command("app")
    .suggest_threshold(3);  // Max edit distance for suggestions
```

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | General error |
| 2 | Usage error (bad arguments) |

---

## Human-Friendly Parsers

### Duration Parsing

Parse human-readable durations:

```cpp
#include <argu/parsers/duration.hpp>

auto seconds = argu::parsers::parse_duration_seconds("2h30m");
// seconds = 9000 (2*3600 + 30*60)

auto ms = argu::parsers::parse_duration_ms("500ms");
// ms = 500

auto dur = argu::parsers::parse_duration<std::chrono::minutes>("1h30m");
// dur = 90 minutes
```

Supported units:
- `ns` - nanoseconds
- `us`, `µs` - microseconds
- `ms` - milliseconds
- `s` - seconds
- `m` - minutes
- `h` - hours
- `d` - days

Examples:
```
5s          → 5 seconds
10m         → 10 minutes
2h          → 2 hours
1d          → 1 day
2h30m       → 2 hours 30 minutes
1h30m45s    → 1 hour 30 minutes 45 seconds
```

Format duration back to string:
```cpp
auto str = argu::parsers::format_duration(std::chrono::seconds(9000));
// str = "2h30m"
```

### Size Parsing

Parse human-readable sizes:

```cpp
#include <argu/parsers/size.hpp>

auto bytes = argu::parsers::parse_size("10MB");
// bytes = 10000000 (SI units)

auto bytes = argu::parsers::parse_size("10MiB");
// bytes = 10485760 (binary units)
```

Supported units:

**SI Units (base 1000):**
- `B` - bytes
- `KB` - kilobytes
- `MB` - megabytes
- `GB` - gigabytes
- `TB` - terabytes
- `PB` - petabytes

**Binary Units (base 1024):**
- `KiB` - kibibytes
- `MiB` - mebibytes
- `GiB` - gibibytes
- `TiB` - tebibytes
- `PiB` - pebibytes

Examples:
```
100B        → 100 bytes
10KB        → 10000 bytes
5MB         → 5000000 bytes
1GB         → 1000000000 bytes
10KiB       → 10240 bytes
5MiB        → 5242880 bytes
1GiB        → 1073741824 bytes
1.5GiB      → 1610612736 bytes
```

Format size back to string:
```cpp
auto str = argu::parsers::format_size(1073741824, true);
// str = "1GiB" (binary)

auto str = argu::parsers::format_size(1000000000, false);
// str = "1GB" (SI)
```

### Transformers

Transform values before validation:

```cpp
.arg(argu::Arg("timeout")
    .long_name("timeout")
    .value_of(timeout_seconds)
    .transform(argu::transformers::duration_to_seconds()))

.arg(argu::Arg("max-size")
    .long_name("max-size")
    .value_of(max_bytes)
    .transform(argu::transformers::size_to_bytes()))
```

---

## Advanced Topics

### Deprecated Arguments

```cpp
.arg(argu::Arg("old-option")
    .long_name("old-option")
    .deprecated("Use --new-option instead"))
```

### Hidden Aliases

For backwards compatibility:

```cpp
.arg(argu::Arg("output")
    .long_name("output")
    .hidden_alias("out")      // Old name still works
    .hidden_alias("output-file"))
```

### Renamed Arguments

```cpp
.arg(argu::Arg("verbose")
    .long_name("verbose")
    .renamed_from("debug"))   // Warns if old name used
```

### Trailing Arguments

Capture all arguments after `--`:

```cpp
auto cmd = argu::Command("app")
    .trailing_var_arg(true);

// myapp --opt value -- arg1 arg2 arg3
// trailing_args() returns ["arg1", "arg2", "arg3"]
```

### Value Terminator

Stop consuming values at a specific string:

```cpp
.arg(argu::Arg("args")
    .takes_one_or_more()
    .value_terminator("--"))
```

### Callbacks

```cpp
.arg(argu::Arg("verbose")
    .flag(verbose)
    .on_parse([](const std::string& val) {
        std::cout << "Verbose mode enabled!\n";
    }))

.arg(argu::Arg("count")
    .value_of(count)
    .on_parse([](const std::string& val) {
        std::cout << "Count set to: " << val << "\n";
    }))
```

### Terminal Detection

```cpp
// Check terminal capabilities
bool has_color = argu::ansi::supports_color();
bool has_truecolor = argu::ansi::supports_truecolor();
bool is_tty = argu::ansi::is_tty();

// Get terminal size
auto width = argu::ansi::terminal_width();
auto height = argu::ansi::terminal_height();
```

### NO_COLOR Support

Argu respects these environment variables:
- `NO_COLOR` - Disable colors
- `FORCE_COLOR` - Force colors
- `CLICOLOR` / `CLICOLOR_FORCE` - BSD-style color control
- `TERM=dumb` - Disable colors

---

## API Reference

### Namespace Structure

```
argu::
├── Command
├── Arg
├── ArgGroup
├── GroupType
├── ParseResult
├── Matches
├── Match
├── ValueSource
├── ValueAction
├── ValueCount
├── ColorMode
├── ErrorMode
├── HelpTheme
├── Color
├── StyledText
├── Validator
├── Transformer
├── Completer
├── validators::
│   ├── file_exists()
│   ├── dir_exists()
│   ├── range(min, max)
│   ├── positive()
│   ├── email()
│   ├── url()
│   ├── ipv4()
│   ├── duration()
│   ├── size()
│   └── ... (40+ validators)
├── parsers::
│   ├── parse_duration<T>(str)
│   ├── parse_duration_seconds(str)
│   ├── parse_duration_ms(str)
│   ├── format_duration(dur)
│   ├── parse_size(str)
│   └── format_size(bytes)
├── transformers::
│   ├── duration_to_seconds()
│   ├── duration_to_ms()
│   └── size_to_bytes()
├── completions::
│   ├── generate_bash(cmd)
│   ├── generate_zsh(cmd)
│   ├── generate_fish(cmd)
│   ├── generate_powershell(cmd)
│   └── generate_elvish(cmd)
└── ansi::
    ├── supports_color()
    ├── supports_truecolor()
    ├── is_tty()
    ├── terminal_width()
    └── terminal_height()
```

---

## Examples

### Git-Style CLI

```cpp
#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::string repo_path = ".";

    auto cmd = argu::Command("git")
        .version("2.42.0")
        .about("The stupid content tracker")
        .color(argu::ColorMode::Auto)
        .allow_subcommand_prefix(true)

        .arg(argu::Arg("verbose")
            .short_name('v')
            .flag(verbose)
            .global())

        .arg(argu::Arg("git-dir")
            .long_name("git-dir")
            .value_of(repo_path)
            .hint_dir()
            .global())

        .subcommand(
            argu::Command("clone")
                .about("Clone a repository")
                .arg(argu::Arg("repository")
                    .positional()
                    .required()
                    .help("Repository URL"))
                .arg(argu::Arg("directory")
                    .positional()
                    .help("Target directory")))

        .subcommand(
            argu::Command("commit")
                .about("Record changes")
                .arg(argu::Arg("message")
                    .short_name('m')
                    .long_name("message")
                    .required()
                    .help("Commit message"))
                .arg(argu::Arg("all")
                    .short_name('a')
                    .long_name("all")
                    .help("Stage all modified files"))
                .arg(argu::Arg("amend")
                    .long_name("amend")
                    .help("Amend previous commit")))

        .subcommand(
            argu::Command("status")
                .about("Show working tree status")
                .arg(argu::Arg("short")
                    .short_name('s')
                    .long_name("short")
                    .help("Short format"))
                .arg(argu::Arg("branch")
                    .short_name('b')
                    .long_name("branch")
                    .help("Show branch info")))

        .subcommand(
            argu::Command("push")
                .about("Update remote refs")
                .arg(argu::Arg("remote")
                    .positional()
                    .default_value("origin"))
                .arg(argu::Arg("branch")
                    .positional())
                .arg(argu::Arg("force")
                    .short_name('f')
                    .long_name("force")
                    .help("Force push")));

    auto result = cmd.parse(argc, argv);
    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();
    auto chain = matches.subcommand_chain();

    if (!chain.empty()) {
        std::cout << "git";
        for (const auto& sub : chain) {
            std::cout << " " << sub;
        }
        std::cout << "\n";

        if (verbose) {
            std::cout << "Verbose mode enabled\n";
        }
    }

    return 0;
}
```

### Server Configuration

```cpp
#include <argu/argu.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string log_level = "info";
    std::string config_file;
    bool daemon = false;
    int workers = 4;
    std::string timeout = "30s";
    std::string max_body = "10MB";

    auto cmd = argu::Command("server")
        .version("1.0.0")
        .about("A high-performance HTTP server")
        .env_prefix("SERVER_")
        .error_mode(argu::ErrorMode::Aggregate)
        .theme(argu::HelpTheme::colorful())

        .arg(argu::Arg("host")
            .short_name('H')
            .long_name("host")
            .help("Bind address")
            .value_of(host)
            .default_value("0.0.0.0")
            .validate(argu::validators::any_of({
                argu::validators::ipv4(),
                argu::validators::hostname()
            })))

        .arg(argu::Arg("port")
            .short_name('p')
            .long_name("port")
            .help("Port number")
            .value_of(port)
            .default_value("8080")
            .validate(argu::validators::non_privileged_port()))

        .arg(argu::Arg("log-level")
            .short_name('l')
            .long_name("log-level")
            .help("Logging level")
            .value_of(log_level)
            .choices({"trace", "debug", "info", "warn", "error"})
            .default_value("info"))

        .arg(argu::Arg("config")
            .short_name('c')
            .long_name("config")
            .help("Configuration file")
            .value_of(config_file)
            .hint_file()
            .validate(argu::validators::file_exists()))

        .arg(argu::Arg("daemon")
            .short_name('d')
            .long_name("daemon")
            .help("Run as daemon")
            .flag(daemon))

        .arg(argu::Arg("workers")
            .short_name('w')
            .long_name("workers")
            .help("Number of worker threads")
            .value_of(workers)
            .default_value("4")
            .validate(argu::validators::range(1, 64)))

        .arg(argu::Arg("timeout")
            .long_name("timeout")
            .help("Request timeout (e.g., 30s, 1m)")
            .value_of(timeout)
            .default_value("30s")
            .validate(argu::validators::duration()))

        .arg(argu::Arg("max-body")
            .long_name("max-body")
            .help("Maximum request body size (e.g., 10MB)")
            .value_of(max_body)
            .default_value("10MB")
            .validate(argu::validators::size()));

    auto result = cmd.parse(argc, argv);
    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    std::cout << "Starting server:\n";
    std::cout << "  Host: " << host << "\n";
    std::cout << "  Port: " << port << "\n";
    std::cout << "  Log level: " << log_level << "\n";
    std::cout << "  Workers: " << workers << "\n";
    std::cout << "  Timeout: " << timeout << "\n";
    std::cout << "  Max body: " << max_body << "\n";
    if (daemon) std::cout << "  Running as daemon\n";
    if (!config_file.empty()) std::cout << "  Config: " << config_file << "\n";

    return 0;
}
```

### Build Tool

```cpp
#include <argu/argu.hpp>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    bool release = false;
    bool clean = false;
    std::string target;
    std::vector<std::string> features;
    int jobs = 0;
    bool verbose = false;

    auto cmd = argu::Command("build")
        .version("0.1.0")
        .about("Build system")
        .allow_subcommand_prefix(true)

        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .flag(verbose)
            .global())

        .subcommand(
            argu::Command("build")
                .about("Build the project")
                .arg(argu::Arg("release")
                    .short_name('r')
                    .long_name("release")
                    .help("Build in release mode")
                    .flag(release))
                .arg(argu::Arg("target")
                    .short_name('t')
                    .long_name("target")
                    .help("Build target")
                    .value_of(target))
                .arg(argu::Arg("features")
                    .short_name('F')
                    .long_name("features")
                    .help("Enable features")
                    .value_of(features)
                    .value_delimiter(','))
                .arg(argu::Arg("jobs")
                    .short_name('j')
                    .long_name("jobs")
                    .help("Number of parallel jobs")
                    .value_of(jobs)
                    .validate(argu::validators::positive())))

        .subcommand(
            argu::Command("clean")
                .about("Clean build artifacts")
                .arg(argu::Arg("all")
                    .long_name("all")
                    .help("Clean all artifacts")))

        .subcommand(
            argu::Command("test")
                .about("Run tests")
                .arg(argu::Arg("filter")
                    .positional()
                    .help("Test filter pattern"))
                .arg(argu::Arg("nocapture")
                    .long_name("nocapture")
                    .help("Don't capture stdout")))

        .subcommand(
            argu::Command("run")
                .about("Build and run")
                .arg(argu::Arg("release")
                    .short_name('r')
                    .long_name("release")
                    .flag(release))
                .arg(argu::Arg("args")
                    .positional()
                    .takes_multiple()
                    .help("Arguments to pass")));

    auto result = cmd.parse(argc, argv);
    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();
    if (auto sub = matches.subcommand()) {
        std::cout << "Running: " << *sub << "\n";
        if (verbose) std::cout << "Verbose mode\n";

        if (*sub == "build") {
            std::cout << "Mode: " << (release ? "release" : "debug") << "\n";
            if (!target.empty()) std::cout << "Target: " << target << "\n";
            if (!features.empty()) {
                std::cout << "Features: ";
                for (const auto& f : features) std::cout << f << " ";
                std::cout << "\n";
            }
            if (jobs > 0) std::cout << "Jobs: " << jobs << "\n";
        }
    }

    return 0;
}
```

---

## Comparison with Other Libraries

### vs CLI11

**Advantages of Argu:**
- Subcommand prefix matching
- Colored help output with themes
- Shell completion generation
- Typo suggestions
- Duration and size parsing
- Error aggregation
- Value hints for completions

**CLI11 Advantages:**
- More mature, widely used
- Subgroup support
- Windows-style options (/option)

### vs cxxopts

**Advantages of Argu:**
- Subcommand support
- Config file support
- Environment variables
- Validators
- Shell completions
- Colored output

**cxxopts Advantages:**
- Simpler API
- Smaller codebase

### vs Boost.Program_options

**Advantages of Argu:**
- Header-only
- No Boost dependency
- Modern C++20
- Better error messages
- Shell completions

**Boost Advantages:**
- Part of Boost ecosystem
- Long history, very stable

---

## License

MIT License - See [LICENSE](../LICENSE) for details.

---

## Acknowledgements

Inspired by:
- [CLAP](https://github.com/clap-rs/clap) - Rust's Command Line Argument Parser
- [CLI11](https://github.com/CLIUtils/CLI11) - C++ CLI parsing library
- [argparse](https://docs.python.org/3/library/argparse.html) - Python's argument parser

---

*Making C++ CLI development as enjoyable as in Rust*
