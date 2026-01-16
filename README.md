# Scan

**Modern C++20 Terminal UI & Argument Parsing Library**

A comprehensive header-only library providing two powerful modules:

- **scan::** - Terminal UI components inspired by [Charm's Bubble Tea](https://github.com/charmbracelet/bubbletea)
- **argu::** - Feature-rich argument parser comparable to Rust's CLAP

```
╭──────────────────────────────────────────────────────────────╮
│  scan::                        argu::                        │
│  ├── TextInput                 ├── Short/long options        │
│  ├── TextArea                  ├── Subcommands (4+ levels)   │
│  ├── List                      ├── Shell completions         │
│  ├── Filter                    ├── Config files (TOML/INI)   │
│  ├── Confirm                   ├── Environment variables     │
│  ├── Spinner                   ├── 40+ validators            │
│  ├── Table                     ├── Colored help              │
│  ├── Viewport                  ├── Duration/size parsing     │
│  ├── Pager                     └── Typo suggestions          │
│  └── FilePicker                                              │
╰──────────────────────────────────────────────────────────────╯
```

## Quick Start

### Terminal UI (scan::)

```cpp
#include <scan/scan.hpp>

int main() {
    // Text input
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();

    // Selection list
    auto choice = scan::List()
        .items({"Option A", "Option B", "Option C"})
        .run();

    // Confirmation
    auto confirm = scan::Confirm()
        .prompt("Continue?")
        .run();

    return 0;
}
```

### Argument Parsing (argu::)

```cpp
#include <argu/argu.hpp>

int main(int argc, char* argv[]) {
    std::string file;
    int count = 1;
    bool verbose = false;

    auto cmd = argu::Command("myapp")
        .version("1.0.0")
        .about("A sample application")
        .arg(argu::Arg("file")
            .positional()
            .required()
            .help("Input file")
            .bind_to(file))
        .arg(argu::Arg("count", 'c')
            .help("Number of iterations")
            .default_value("1")
            .bind_to(count))
        .arg(argu::Arg("verbose", 'v')
            .flag()
            .help("Enable verbose output")
            .bind_to(verbose));

    auto result = cmd.parse(argc, argv);
    if (!result) {
        std::cerr << result.error().format() << "\n";
        return 1;
    }

    std::cout << "File: " << file << ", Count: " << count
              << ", Verbose: " << verbose << "\n";
    return 0;
}
```

## Features Overview

### scan:: - Terminal UI

| Component | Description |
|-----------|-------------|
| TextInput | Single-line text input with editing |
| TextArea | Multi-line text editor |
| List | Single/multi-select lists |
| Filter | Fuzzy search with highlighting |
| Confirm | Yes/No dialogs |
| Spinner | Async progress indicators |
| Table | Data tables with selection |
| Viewport | Scrollable content |
| Pager | Full-screen document viewer |
| FilePicker | File/directory browser |

Plus: Lip Gloss-style styling, themes, MVU architecture.

### argu:: - Argument Parsing

| Feature | Description |
|---------|-------------|
| Options | Short (-v) and long (--verbose) with binding |
| Subcommands | Deep nesting (4+ levels), prefix matching |
| Validation | 40+ built-in validators (email, url, range, etc.) |
| Config | TOML/INI files with layering |
| Environment | Variable binding with prefix support |
| Help | Auto-generated, colored, with typo suggestions |
| Completions | Bash, Zsh, Fish, PowerShell, Elvish |
| Parsing | Duration (5s, 2h30m), size (10KB, 1GiB) |
| Relations | conflicts_with, requires, mutex groups |

## Installation

### CMake FetchContent

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

### Manual

Copy `include/scan` and `include/argu` to your project:

```cpp
#include <scan/scan.hpp>  // Terminal UI
#include <argu/argu.hpp>  // Argument parsing
```

## Documentation

For comprehensive documentation with all features, options, and examples:

- **[scan:: Documentation](misc/SCAN.md)** - Complete terminal UI guide (1000+ lines)
- **[argu:: Documentation](misc/ARGU.md)** - Complete argument parsing guide (1000+ lines)

## Building

```bash
cmake -B build -DSCAN_BUILD_EXAMPLES=ON -DSCAN_ENABLE_TESTS=ON
cmake --build build
```

## Examples

See the `examples/` directory:

```
examples/
├── scan_*.cpp        # Terminal UI examples
└── argu_*.cpp        # Argument parsing examples
```

## Tests

220 tests covering both libraries:

```
test/
├── test_scan.cpp        # Terminal UI tests
└── test_argu.cpp        # Argument parsing tests
```


## Requirements

- **C++20** compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **[Echo](https://github.com/bresilla/echo)** - Terminal utilities (fetched automatically)

## License

MIT License - See [LICENSE](LICENSE) for details.

---

Inspired by [Charm](https://charm.sh/) (Bubble Tea, Lip Gloss, Gum) and Rust's CLAP.
