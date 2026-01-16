# Scan - Terminal UI Library

**Comprehensive Documentation**

A header-only C++20 terminal UI library inspired by [Charm's Bubble Tea](https://github.com/charmbracelet/bubbletea) and [Gum](https://github.com/charmbracelet/gum), built on top of [Echo](https://github.com/bresilla/echo). Scan brings the elegance of functional reactive programming to C++ terminal applications with an Elm-inspired Model-View-Update (MVU) architecture.

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Components](#components)
   - [TextInput](#textinput---single-line-text-input)
   - [TextArea](#textarea---multi-line-text-editor)
   - [List](#list---selection-lists)
   - [Filter](#filter---fuzzy-search)
   - [Confirm](#confirm---yesno-dialog)
   - [Spinner](#spinner---progress-indicator)
   - [Table](#table---data-display)
   - [Viewport](#viewport---scrollable-content)
   - [Pager](#pager---full-screen-viewer)
   - [FilePicker](#filepicker---file-browser)
6. [Style System](#style-system)
7. [Theme System](#theme-system)
8. [Tea Architecture (MVU)](#tea-architecture-mvu)
9. [Advanced Topics](#advanced-topics)
10. [API Reference](#api-reference)
11. [Examples](#examples)
12. [Building](#building)
13. [Troubleshooting](#troubleshooting)

---

## Overview

Scan is a modern C++20 terminal UI library that provides:

- **Pre-built Components**: Ready-to-use interactive components for common UI patterns
- **Composable Styling**: Lip Gloss-style chainable styling API
- **MVU Architecture**: Elm-inspired Model-View-Update pattern for building custom components
- **Cross-platform**: Works on Linux, macOS, and Windows (with ANSI-capable terminal)

### Design Philosophy

Scan follows these core principles:

1. **Simplicity First**: Simple tasks should be simple to implement
2. **Composability**: Components and styles can be combined freely
3. **Type Safety**: Leverage C++20 features for compile-time safety
4. **Zero Dependencies**: Header-only with minimal external requirements

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Application                           │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  Components  │  │    Style     │  │   Tea (MVU)      │  │
│  │  TextInput   │  │  Foreground  │  │  Model           │  │
│  │  TextArea    │  │  Background  │  │  View            │  │
│  │  List        │  │  Padding     │  │  Update          │  │
│  │  Filter      │  │  Border      │  │  Commands        │  │
│  │  Confirm     │  │  Alignment   │  │                  │  │
│  │  Spinner     │  │              │  │                  │  │
│  │  Table       │  │              │  │                  │  │
│  │  ...         │  │              │  │                  │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                      Echo (Terminal I/O)                     │
├─────────────────────────────────────────────────────────────┤
│                       Terminal / PTY                         │
└─────────────────────────────────────────────────────────────┘
```

---

## Features

### Core Features

| Feature | Description |
|---------|-------------|
| **Header-only** | Single include, no build required |
| **C++20** | Modern C++ with concepts, ranges, designated initializers |
| **MVU Architecture** | Elm-inspired functional reactive programming |
| **Rich Components** | 10+ pre-built interactive components |
| **Theming** | Built-in light/dark themes with customization |
| **Styling** | Lip Gloss-style chainable API |
| **UTF-8** | Full Unicode and emoji support |
| **Cross-platform** | Linux, macOS, Windows |

### Component Summary

| Component | Description | Return Type |
|-----------|-------------|-------------|
| `TextInput` | Single-line text input | `optional<string>` |
| `TextArea` | Multi-line text editor | `optional<string>` |
| `List` | Selection list | `optional<string>` or `optional<vector<string>>` |
| `Filter` | Fuzzy search list | `optional<string>` or `optional<vector<string>>` |
| `Confirm` | Yes/No dialog | `optional<bool>` |
| `Spinner` | Progress indicator | `T` (your return type) |
| `Table` | Data table | `optional<size_t>` (row index) |
| `Viewport` | Scrollable content | Render-only |
| `Pager` | Full-screen viewer | `void` |
| `FilePicker` | File browser | `optional<filesystem::path>` |

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

1. Clone the repository:
```bash
git clone https://github.com/bresilla/scan.git
```

2. Copy the `include/scan` directory to your project

3. Include the header:
```cpp
#include <scan/scan.hpp>
```

### Requirements

- **C++20** compiler:
  - GCC 11+
  - Clang 14+
  - MSVC 2022+ (19.29+)
- **Echo** library (fetched automatically via CMake)

### Compiler Flags

For optimal performance, use:

```bash
# GCC/Clang
-std=c++20 -O2

# MSVC
/std:c++20 /O2
```

---

## Quick Start

### Hello World

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();

    if (name) {
        std::cout << "Hello, " << *name << "!\n";
    }
    return 0;
}
```

### Simple Selection

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    auto choice = scan::List()
        .items({"Option A", "Option B", "Option C"})
        .run();

    if (choice) {
        std::cout << "You selected: " << *choice << "\n";
    }
    return 0;
}
```

### Confirmation Dialog

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    auto ok = scan::Confirm()
        .prompt("Do you want to continue?")
        .run();

    if (ok && *ok) {
        std::cout << "Continuing...\n";
    } else {
        std::cout << "Cancelled.\n";
    }
    return 0;
}
```

---

## Components

### TextInput - Single-Line Text Input

Interactive single-line text input with full editing capabilities including cursor movement, text selection concepts, and paste support.

#### Basic Usage

```cpp
auto input = scan::TextInput()
    .prompt("Enter value: ")
    .placeholder("Type here...")
    .run();
```

#### Password Mode

```cpp
auto password = scan::TextInput()
    .prompt("Password: ")
    .password(true)
    .mask('*')
    .run();
```

#### With Validation Callback

```cpp
auto email = scan::TextInput()
    .prompt("Email: ")
    .placeholder("user@example.com")
    .char_limit(100)
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.prompt(str)` | `string` | Prefix text shown before input | `"> "` |
| `.placeholder(str)` | `string` | Hint text when empty | `""` |
| `.value(str)` | `string` | Initial value | `""` |
| `.password(bool)` | `bool` | Enable password mode | `false` |
| `.mask(char)` | `char` | Mask character for password | `'*'` |
| `.char_limit(int)` | `int` | Maximum characters (0 = unlimited) | `0` |
| `.width(int)` | `int` | Display width | Terminal width |
| `.prompt_color(r,g,b)` | RGB | Prompt text color | Theme default |
| `.text_color(r,g,b)` | RGB | Input text color | Theme default |
| `.placeholder_color(r,g,b)` | RGB | Placeholder color | Theme default |
| `.cursor_color(r,g,b)` | RGB | Cursor color | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` | Submit input |
| `Escape` | Cancel (returns nullopt) |
| `Ctrl+C` | Cancel (returns nullopt) |
| `Ctrl+A` / `Home` | Move to start of line |
| `Ctrl+E` / `End` | Move to end of line |
| `Ctrl+K` | Kill (delete) to end of line |
| `Ctrl+U` | Kill from start of line |
| `Ctrl+W` | Kill previous word |
| `Ctrl+H` / `Backspace` | Delete previous character |
| `Delete` / `Ctrl+D` | Delete character at cursor |
| `Left` | Move cursor left |
| `Right` | Move cursor right |
| `Ctrl+Left` | Move to previous word |
| `Ctrl+Right` | Move to next word |

#### Return Value

- `std::optional<std::string>` - The entered text, or `std::nullopt` if cancelled

#### Example: Email Input with Styling

```cpp
auto email = scan::TextInput()
    .prompt("Email: ")
    .placeholder("user@example.com")
    .char_limit(50)
    .prompt_color(100, 200, 255)   // Light blue prompt
    .text_color(255, 255, 255)     // White text
    .placeholder_color(128, 128, 128)  // Gray placeholder
    .run();

if (email && !email->empty()) {
    // Validate email format
    if (email->find('@') != std::string::npos) {
        std::cout << "Valid email: " << *email << "\n";
    }
}
```

---

### TextArea - Multi-Line Text Editor

Full-featured multi-line text editor with optional line numbers, suitable for editing larger text content.

#### Basic Usage

```cpp
auto text = scan::TextArea()
    .placeholder("Type your message...")
    .width(60)
    .height(10)
    .run();
```

#### With Line Numbers

```cpp
auto code = scan::TextArea()
    .placeholder("Enter code...")
    .show_line_numbers(true)
    .width(80)
    .height(20)
    .run();
```

#### With Initial Content

```cpp
auto edited = scan::TextArea()
    .value("Initial content\nLine 2\nLine 3")
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.placeholder(str)` | `string` | Hint text when empty | `""` |
| `.value(str)` | `string` | Initial content | `""` |
| `.width(int)` | `int` | Editor width | `60` |
| `.height(int)` | `int` | Editor height (lines) | `10` |
| `.show_line_numbers(bool)` | `bool` | Display line numbers | `false` |
| `.text_color(r,g,b)` | RGB | Text color | Theme default |
| `.line_number_color(r,g,b)` | RGB | Line number color | Theme default |
| `.cursor_color(r,g,b)` | RGB | Cursor color | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+D` / `Escape` | Submit content |
| `Ctrl+C` | Cancel |
| `Enter` | Insert new line |
| `Tab` | Insert 4 spaces |
| `Up` / `Down` | Navigate lines |
| `Left` / `Right` | Navigate characters |
| `Ctrl+A` / `Home` | Start of line |
| `Ctrl+E` / `End` | End of line |
| `Ctrl+K` | Kill to end of line |
| `Ctrl+U` | Kill from start of line |
| `Backspace` | Delete previous character |
| `Delete` | Delete at cursor |
| `Page Up` / `Page Down` | Page navigation |

#### Return Value

- `std::optional<std::string>` - The edited text, or `std::nullopt` if cancelled

---

### List - Selection Lists

Selection component supporting single/multi-select with optional two-line item display (title + description).

#### Single Selection (Simple)

```cpp
auto color = scan::List()
    .items({"Red", "Green", "Blue", "Yellow"})
    .cursor("> ")
    .run();  // Returns optional<string>
```

#### Multi-Select

```cpp
auto fruits = scan::List()
    .items({"Apple", "Banana", "Cherry", "Date", "Elderberry"})
    .no_limit()
    .selected_prefix("[x] ")
    .unselected_prefix("[ ] ")
    .run_multi();  // Returns optional<vector<string>>
```

#### Rich Items (Title + Description)

```cpp
auto editor = scan::List()
    .items({
        {"Vim", "The ubiquitous modal text editor"},
        {"Emacs", "An extensible, customizable text editor"},
        {"VS Code", "A lightweight but powerful source code editor"},
        {"Neovim", "Hyperextensible Vim-based text editor"},
        {"Sublime Text", "A sophisticated text editor for code"},
        {"Atom", "A hackable text editor for the 21st Century"}
    })
    .height(10)
    .cursor("→ ")
    .run();
```

#### Get Full Item (with Description)

```cpp
auto item = scan::List()
    .items({
        {"Option A", "Description for option A"},
        {"Option B", "Description for option B"}
    })
    .run_item();  // Returns optional<ListItem>

if (item) {
    std::cout << "Selected: " << item->title << "\n";
    std::cout << "Description: " << item->description << "\n";
}
```

#### Pre-Selected Items

```cpp
auto selected = scan::List()
    .items({"A", "B", "C", "D", "E"})
    .selected({1, 3})  // Pre-select B and D (indices)
    .no_limit()
    .run_multi();
```

#### Limited Multi-Select

```cpp
// Allow selecting at most 3 items
auto selected = scan::List()
    .items(items)
    .limit(3)
    .run_multi();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.items(vector<string>)` | `vector` | Simple string items | `{}` |
| `.items(vector<ListItem>)` | `vector` | Rich items with descriptions | `{}` |
| `.items({{"t","d"},...})` | `init_list` | Initializer list syntax | `{}` |
| `.selected(vector<size_t>)` | `vector` | Pre-select by indices | `{}` |
| `.limit(n)` | `size_t` | Max selections | `1` |
| `.no_limit()` | - | Unlimited selections | - |
| `.height(int)` | `int` | Display height | `10` |
| `.cursor(str)` | `string` | Cursor indicator | `">"` |
| `.selected_prefix(str)` | `string` | Selection marker | `"[x]"` |
| `.unselected_prefix(str)` | `string` | Unselected marker | `"[ ]"` |
| `.cursor_color(r,g,b)` | RGB | Cursor color | Theme default |
| `.selected_color(r,g,b)` | RGB | Selection color | Theme default |
| `.title_color(r,g,b)` | RGB | Item title color | Theme default |
| `.description_color(r,g,b)` | RGB | Description color | Theme default |

#### Run Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `.run()` | `optional<string>` | Single selection (title only) |
| `.run_multi()` | `optional<vector<string>>` | Multi selection (titles) |
| `.run_item()` | `optional<ListItem>` | Single with description |
| `.run_items()` | `optional<vector<ListItem>>` | Multi with descriptions |
| `.run_index()` | `optional<size_t>` | Single selection (index) |
| `.run_indices()` | `optional<vector<size_t>>` | Multi selection (indices) |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` | Confirm selection |
| `Space` / `x` | Toggle selection (multi-select mode) |
| `Tab` | Toggle selection and move down |
| `Up` / `k` | Move cursor up |
| `Down` / `j` | Move cursor down |
| `g` | Go to first item |
| `G` | Go to last item |
| `a` | Toggle select all (multi-select) |
| `Home` | First item |
| `End` | Last item |
| `Page Up` | Page up |
| `Page Down` | Page down |
| `Escape` / `Ctrl+C` | Cancel |

---

### Filter - Fuzzy Search

Real-time fuzzy filtering with match highlighting. Combines text input with a filtered list.

#### Basic Usage

```cpp
auto language = scan::Filter()
    .items({"C++", "Python", "JavaScript", "Rust", "Go", "Java", "Ruby"})
    .placeholder("Search languages...")
    .height(10)
    .run();
```

#### Multi-Select Filter

```cpp
auto selected = scan::Filter()
    .items(file_list)
    .no_limit()
    .run_multi();
```

#### With Initial Query

```cpp
auto result = scan::Filter()
    .items(all_items)
    .query("search term")  // Start with pre-filled query
    .run();
```

#### Case-Sensitive Search

```cpp
auto result = scan::Filter()
    .items(items)
    .case_sensitive(true)
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.items(vector<string>)` | `vector` | Items to filter | `{}` |
| `.placeholder(str)` | `string` | Hint text | `""` |
| `.prompt(str)` | `string` | Search prompt prefix | `"> "` |
| `.query(str)` | `string` | Initial search query | `""` |
| `.limit(n)` | `size_t` | Max selections | `1` |
| `.no_limit()` | - | Unlimited selections | - |
| `.height(int)` | `int` | Display height | `10` |
| `.case_sensitive(bool)` | `bool` | Case sensitivity | `false` |
| `.match_color(r,g,b)` | RGB | Highlight color | Theme default |
| `.prompt_color(r,g,b)` | RGB | Prompt color | Theme default |
| `.cursor_color(r,g,b)` | RGB | Cursor color | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` | Select highlighted item |
| `Up` / `Ctrl+P` | Move to previous result |
| `Down` / `Ctrl+N` | Move to next result |
| `Tab` | Toggle selection (multi-select) |
| `Ctrl+U` | Clear query |
| `Backspace` | Delete character |
| `Escape` / `Ctrl+C` | Cancel |

#### Fuzzy Matching Algorithm

The filter uses a fuzzy matching algorithm that:
- Matches characters in order (not necessarily consecutive)
- Prioritizes exact matches and prefix matches
- Highlights matched characters in results
- Case-insensitive by default

Example: Query "js" matches:
- **J**ava**S**cript (highlighted)
- **J**ava (no match - missing 's')

---

### Confirm - Yes/No Dialog

Simple confirmation dialog with customizable button labels.

#### Basic Usage

```cpp
auto ok = scan::Confirm()
    .prompt("Save changes?")
    .run();

if (ok && *ok) {
    save_changes();
}
```

#### Custom Button Labels

```cpp
auto confirm = scan::Confirm()
    .prompt("Delete this file?")
    .affirmative("Delete")
    .negative("Keep")
    .default_value(false)  // "Keep" selected by default
    .run();
```

#### With Colors

```cpp
auto result = scan::Confirm()
    .prompt("Proceed with installation?")
    .affirmative("Install")
    .negative("Cancel")
    .prompt_color(255, 255, 255)
    .selected_color(0, 0, 0, 100, 255, 100)  // Black on green
    .unselected_color(128, 128, 128)
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.prompt(str)` | `string` | Question text | `""` |
| `.affirmative(str)` | `string` | Yes button label | `"Yes"` |
| `.negative(str)` | `string` | No button label | `"No"` |
| `.default_value(bool)` | `bool` | Initial selection | `true` |
| `.prompt_color(r,g,b)` | RGB | Prompt color | Theme default |
| `.selected_color(fg..., bg...)` | RGB x 2 | Selected button style | Theme default |
| `.unselected_color(r,g,b)` | RGB | Unselected button style | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` | Confirm current selection |
| `Left` / `h` | Select negative |
| `Right` / `l` | Select affirmative |
| `Tab` | Toggle selection |
| `y` / `Y` | Quick yes |
| `n` / `N` | Quick no |
| `Escape` / `Ctrl+C` | Cancel (returns nullopt) |

#### Return Value

- `std::optional<bool>` - `true` for affirmative, `false` for negative, `nullopt` if cancelled

---

### Spinner - Progress Indicator

Async task progress indicator with multiple animation styles. Executes a callback function while displaying an animated spinner.

#### Basic Usage

```cpp
auto result = scan::Spinner()
    .title("Loading...")
    .run([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;  // Return value is passed through
    });

std::cout << "Result: " << result << "\n";
```

#### With Style

```cpp
scan::Spinner()
    .title("Processing files...")
    .style(scan::SpinnerStyle::bouncing_bar)
    .color(100, 200, 255)
    .run([]() {
        process_files();
        return 0;
    });
```

#### With Gradient

```cpp
scan::Spinner()
    .title("Compiling...")
    .style(scan::SpinnerStyle::aesthetic)
    .gradient({"#FF0000", "#FFFF00", "#00FF00"})
    .run([]() {
        compile();
        return 0;
    });
```

#### Available Styles

| Style | Description | Animation |
|-------|-------------|-----------|
| `line` | Classic rotating line | `- \ | /` |
| `pipe` | Pipe animation | `┤ ┘ ┴ └ ├ ┌ ┬ ┐` |
| `simple_dots` | Simple dots | `. .. ...` |
| `dots_scrolling` | Scrolling dots | `.  ..  ... ....` |
| `flip` | Flip animation | `_ _ _ - ` ` ' ´ -` |
| `toggle` | Toggle animation | `⊶ ⊷` |
| `layer` | Layer animation | `- = ≡` |
| `point` | Point animation | `∙∙∙ ●∙∙ ∙●∙ ∙∙●` |
| `dqpb` | DQPB style | `d q p b` |
| `bouncing_bar` | Bouncing progress bar | `[=   ] [==  ] [=== ]` |
| `bouncing_ball` | Bouncing ball | `( ●  ) (  ● ) (   ●)` |
| `aesthetic` | Aesthetic blocks | `▰▱▱ ▰▰▱ ▰▰▰` |
| `binary` | Binary animation | `010010 101001` |
| `grow_vertical` | Vertical growth | `▁ ▃ ▄ ▅ ▆ ▇ █` |
| `grow_horizontal` | Horizontal growth | `▏ ▎ ▍ ▌ ▋ ▊ ▉ █` |

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.title(str)` | `string` | Display message | `""` |
| `.style(SpinnerStyle)` | `enum` | Animation style | `line` |
| `.color(r,g,b)` | RGB | Spinner color | Theme default |
| `.color("#hex")` | `string` | Hex color | - |
| `.gradient(vector<string>)` | `vector` | Color gradient | - |
| `.interval(ms)` | `int` | Animation speed (ms) | Style default |
| `.title_color(r,g,b)` | RGB | Title color | Theme default |

#### Return Value

The spinner returns whatever your callback function returns:

```cpp
// Returns int
int result = scan::Spinner()
    .title("Computing...")
    .run([]() -> int { return compute(); });

// Returns string
std::string data = scan::Spinner()
    .title("Fetching...")
    .run([]() -> std::string { return fetch_data(); });

// Returns void
scan::Spinner()
    .title("Saving...")
    .run([]() { save(); });
```

---

### Table - Data Display

Structured data table with optional row selection and customizable styling.

#### Static Display (Print Only)

```cpp
scan::Table()
    .headers({"Name", "Language", "Stars"})
    .rows({
        {"React", "JavaScript", "218k"},
        {"Vue", "JavaScript", "206k"},
        {"Angular", "TypeScript", "93k"},
        {"Svelte", "JavaScript", "75k"}
    })
    .border(scan::BorderStyle::Rounded)
    .print();
```

#### Interactive Selection

```cpp
auto row_index = scan::Table()
    .headers({"ID", "Task", "Status"})
    .rows({
        {"1", "Build UI", "Done"},
        {"2", "Write tests", "In Progress"},
        {"3", "Deploy", "Pending"}
    })
    .selectable(true)
    .height(5)
    .run();  // Returns optional<size_t>

if (row_index) {
    std::cout << "Selected row: " << *row_index << "\n";
}
```

#### Custom Column Widths

```cpp
scan::Table()
    .headers({"ID", "Name", "Description"})
    .widths({5, 20, 50})  // Custom widths
    .rows(data)
    .print();
```

#### Border Styles

```cpp
// Available styles:
scan::BorderStyle::Normal    // ┌───┬───┐
scan::BorderStyle::Rounded   // ╭───┬───╮
scan::BorderStyle::Double    // ╔═══╦═══╗
scan::BorderStyle::Thick     // ┏━━━┳━━━┓
scan::BorderStyle::Hidden    // No borders
```

Visual comparison:
```
Normal:           Rounded:          Double:           Thick:
┌─────┬─────┐     ╭─────┬─────╮     ╔═════╦═════╗     ┏━━━━━┳━━━━━┓
│ A   │ B   │     │ A   │ B   │     ║ A   ║ B   ║     ┃ A   ┃ B   ┃
├─────┼─────┤     ├─────┼─────┤     ╠═════╬═════╣     ┣━━━━━╋━━━━━┫
│ 1   │ 2   │     │ 1   │ 2   │     ║ 1   ║ 2   ║     ┃ 1   ┃ 2   ┃
└─────┴─────┘     ╰─────┴─────╯     ╚═════╩═════╝     ┗━━━━━┻━━━━━┛
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.headers(vector<string>)` | `vector` | Column headers | `{}` |
| `.rows(vector<vector<string>>)` | `vector` | Table data | `{}` |
| `.widths(vector<int>)` | `vector` | Column widths | Auto |
| `.border(BorderStyle)` | `enum` | Border style | `Normal` |
| `.height(int)` | `int` | Visible rows | All |
| `.selectable(bool)` | `bool` | Enable selection | `false` |
| `.header_style(r,g,b)` | RGB | Header color | Theme default |
| `.border_color(r,g,b)` | RGB | Border color | Theme default |
| `.selected_style(fg..., bg...)` | RGB x 2 | Selection style | Theme default |
| `.row_color(r,g,b)` | RGB | Row text color | Theme default |
| `.alternate_row_color(r,g,b)` | RGB | Alternate row color | - |

#### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `.print()` | `void` | Print table to stdout |
| `.render()` | `string` | Get rendered table as string |
| `.run()` | `optional<size_t>` | Interactive selection (row index) |

#### Keyboard Shortcuts (Interactive Mode)

| Key | Action |
|-----|--------|
| `Enter` | Select current row |
| `Up` / `k` | Move up |
| `Down` / `j` | Move down |
| `g` | Go to first row |
| `G` | Go to last row |
| `Home` | First row |
| `End` | Last row |
| `Page Up` / `Page Down` | Page navigation |
| `Escape` / `Ctrl+C` | Cancel |

---

### Viewport - Scrollable Content

Scrollable content viewer with word wrapping support. Useful for displaying large text content in a fixed-size area.

#### Basic Usage

```cpp
auto vp = scan::Viewport()
    .content(large_text)
    .width(80)
    .height(20);

std::cout << vp.render() << "\n";
```

#### With Word Wrapping

```cpp
auto vp = scan::Viewport()
    .content(long_paragraph)
    .width(60)
    .height(10)
    .wrap(true);
```

#### Programmatic Scrolling

```cpp
auto vp = scan::Viewport()
    .content(document)
    .width(80)
    .height(24);

// Scroll operations
vp.scroll_down(5);      // Scroll down 5 lines
vp.scroll_up(3);        // Scroll up 3 lines
vp.page_down();         // Move down one page
vp.page_up();           // Move up one page
vp.goto_top();          // Jump to top
vp.goto_bottom();       // Jump to bottom

// Get scroll position
int percent = vp.scroll_percent();  // 0-100
bool at_top = vp.at_top();
bool at_bottom = vp.at_bottom();

std::cout << vp.render() << "\n";
std::cout << "Position: " << percent << "%\n";
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.content(str)` | `string` | Set content | `""` |
| `.width(int)` | `int` | Display width | `80` |
| `.height(int)` | `int` | Display height | `20` |
| `.wrap(bool)` | `bool` | Enable word wrapping | `false` |

#### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `.scroll_up(n)` | `void` | Scroll up n lines |
| `.scroll_down(n)` | `void` | Scroll down n lines |
| `.page_up()` | `void` | Scroll up one page |
| `.page_down()` | `void` | Scroll down one page |
| `.goto_top()` | `void` | Jump to top |
| `.goto_bottom()` | `void` | Jump to bottom |
| `.scroll_percent()` | `int` | Get position (0-100) |
| `.at_top()` | `bool` | Check if at top |
| `.at_bottom()` | `bool` | Check if at bottom |
| `.render()` | `string` | Get rendered content |
| `.total_lines()` | `size_t` | Get total line count |
| `.visible_lines()` | `size_t` | Get visible line count |

---

### Pager - Full-Screen Viewer

Full-screen document viewer similar to `less`. Takes over the terminal for document viewing.

#### Basic Usage

```cpp
scan::Pager()
    .content(file_contents)
    .run();
```

#### With Title and Line Numbers

```cpp
scan::Pager()
    .content(file_contents)
    .title("README.md")
    .line_numbers(true)
    .status_bar(true)
    .run();
```

#### With Styling

```cpp
scan::Pager()
    .content(document)
    .title("Document Viewer")
    .wrap(true)
    .title_color(100, 200, 255)
    .status_colors(0, 0, 0, 100, 200, 255)  // Black on blue
    .line_number_color(128, 128, 128)
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.content(str)` | `string` | Document content | `""` |
| `.title(str)` | `string` | Document title | `""` |
| `.line_numbers(bool)` | `bool` | Show line numbers | `false` |
| `.status_bar(bool)` | `bool` | Show status bar | `true` |
| `.wrap(bool)` | `bool` | Word wrapping | `false` |
| `.title_color(r,g,b)` | RGB | Title color | Theme default |
| `.status_colors(fg..., bg...)` | RGB x 2 | Status bar colors | Theme default |
| `.line_number_color(r,g,b)` | RGB | Line number color | Theme default |
| `.text_color(r,g,b)` | RGB | Text color | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `q` / `Escape` | Quit |
| `j` / `Down` / `Enter` | Scroll down one line |
| `k` / `Up` | Scroll up one line |
| `d` / `Ctrl+D` | Half-page down |
| `u` / `Ctrl+U` | Half-page up |
| `Space` / `Page Down` | Page down |
| `b` / `Page Up` | Page up |
| `g` / `Home` | Go to top |
| `G` / `End` | Go to bottom |
| `h` | Show help (if implemented) |

---

### FilePicker - File Browser

Interactive file and directory browser with filtering support.

#### Select Any File

```cpp
auto file = scan::FilePicker()
    .path(".")
    .height(15)
    .run();  // Returns optional<filesystem::path>

if (file) {
    std::cout << "Selected: " << *file << "\n";
}
```

#### Filter by Extension

```cpp
auto source_file = scan::FilePicker()
    .path("/src")
    .extensions({".cpp", ".hpp", ".h", ".cc"})
    .run();
```

#### Directory Selection Only

```cpp
auto dir = scan::FilePicker()
    .path(".")
    .file_allowed(false)
    .dir_allowed(true)
    .run();
```

#### Show Hidden Files

```cpp
auto file = scan::FilePicker()
    .path("~")
    .show_hidden(true)
    .run();
```

#### All Options

| Method | Type | Description | Default |
|--------|------|-------------|---------|
| `.path(fs::path)` | `path` | Starting directory | Current dir |
| `.show_hidden(bool)` | `bool` | Show hidden files | `false` |
| `.file_allowed(bool)` | `bool` | Allow file selection | `true` |
| `.dir_allowed(bool)` | `bool` | Allow directory selection | `false` |
| `.extensions(vector<string>)` | `vector` | Filter by extensions | All |
| `.height(int)` | `int` | Display height | `10` |
| `.dir_color(r,g,b)` | RGB | Directory color | Theme default |
| `.file_color(r,g,b)` | RGB | File color | Theme default |
| `.symlink_color(r,g,b)` | RGB | Symlink color | Theme default |
| `.cursor_color(r,g,b)` | RGB | Cursor color | Theme default |

#### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` | Select file or enter directory |
| `Left` / `Backspace` / `h` | Go to parent directory |
| `Right` / `l` | Enter directory |
| `.` | Toggle hidden files |
| `Up` / `k` | Move cursor up |
| `Down` / `j` | Move cursor down |
| `g` | Go to first item |
| `G` | Go to last item |
| `Home` | First item |
| `End` | Last item |
| `Page Up` / `Page Down` | Page navigation |
| `Escape` / `Ctrl+C` | Cancel |

#### Return Value

- `std::optional<std::filesystem::path>` - Selected path, or `nullopt` if cancelled

---

## Style System

Scan provides a Lip Gloss-inspired chainable styling API for creating rich terminal output.

### Basic Styling

```cpp
auto styled = scan::Style()
    .foreground(255, 200, 100)  // Text color
    .background(50, 50, 50)     // Background color
    .bold()                     // Bold text
    .render("Hello, World!");

std::cout << styled << "\n";
```

### Text Attributes

```cpp
scan::Style()
    .bold()           // Bold text
    .italic()         // Italic text
    .underline()      // Underlined text
    .strikethrough()  // Strikethrough text
    .faint()          // Dim/faint text
    .reverse()        // Reverse colors
    .blink()          // Blinking text (if supported)
```

### Colors

```cpp
// RGB colors
scan::Style()
    .foreground(255, 100, 100)
    .background(50, 50, 50);

// Using Color struct
scan::Style()
    .foreground(scan::Color{255, 100, 100});

// Hex colors
scan::Style()
    .foreground(scan::Color::from_hex("#FF6464"));
```

### Padding

```cpp
// All sides
scan::Style().padding(2);

// Vertical and horizontal
scan::Style().padding(1, 2);  // 1 top/bottom, 2 left/right

// Individual sides
scan::Style().padding(1, 2, 1, 2);  // top, right, bottom, left

// Single side
scan::Style()
    .padding_top(1)
    .padding_right(2)
    .padding_bottom(1)
    .padding_left(2);
```

### Margin

```cpp
// All sides
scan::Style().margin(1);

// Vertical and horizontal
scan::Style().margin(1, 2);

// Individual sides
scan::Style().margin(1, 2, 1, 2);

// Single side
scan::Style()
    .margin_top(1)
    .margin_right(2)
    .margin_bottom(1)
    .margin_left(2);
```

### Dimensions

```cpp
scan::Style()
    .width(40)       // Fixed width
    .height(10)      // Fixed height
    .max_width(80)   // Maximum width
    .max_height(24)  // Maximum height
```

### Borders

```cpp
scan::Style()
    .border(scan::BorderStyle::Rounded)
    .border_foreground(100, 200, 100)
    .border_background(50, 50, 50);
```

### Alignment

```cpp
scan::Style()
    .width(40)
    .align(scan::Position::Center, scan::Position::Top);

// Horizontal alignment
scan::Style()
    .width(40)
    .align_horizontal(scan::Position::Center);

// Vertical alignment (for multi-line)
scan::Style()
    .height(10)
    .align_vertical(scan::Position::Center);
```

### Complete Example

```cpp
auto box = scan::Style()
    .foreground(255, 255, 255)
    .background(60, 60, 60)
    .bold()
    .padding(1, 2)
    .margin(1)
    .border(scan::BorderStyle::Rounded)
    .border_foreground(100, 200, 255)
    .width(50)
    .align(scan::Position::Center, scan::Position::Center)
    .render("Welcome to Scan!\nA modern terminal UI library.");

std::cout << box << "\n";
```

### Layout Utilities

```cpp
// Join horizontally
auto row = scan::join_horizontal(
    scan::Position::Top,
    {box1, box2, box3}
);

// Join vertically
auto column = scan::join_vertical(
    scan::Position::Center,
    {header, content, footer}
);

// Place content at position
auto placed = scan::place(
    80,                     // total width
    24,                     // total height
    scan::Position::Center, // horizontal
    scan::Position::Center, // vertical
    content
);
```

### String Utilities

```cpp
// Padding
scan::pad_right(str, width);
scan::pad_left(str, width);
scan::pad_center(str, width);

// Truncation
scan::truncate(str, max_width);
scan::truncate(str, max_width, "...");

// Repetition
scan::repeat("=", 40);  // "========"...

// Width calculation (ignoring ANSI codes)
size_t w = scan::visible_width(styled_string);
```

---

## Theme System

Scan includes a built-in theming system with light and dark modes.

### Using Themes

```cpp
// Set dark theme (default)
scan::set_theme(scan::default_theme());

// Set light theme
scan::set_theme(scan::light_theme());

// Access current theme
auto& theme = scan::current_theme();
```

### Theme Structure

```cpp
struct Theme {
    std::string name;
    ThemeColors colors;
};

struct ThemeColors {
    Color primary;         // Accent color
    Color secondary;       // Secondary accent
    Color success;         // Success/green
    Color error;           // Error/red
    Color warning;         // Warning/orange

    Color text;            // Main text
    Color text_muted;      // Secondary text
    Color text_subtle;     // Hints/placeholders

    Color bg;              // Background
    Color bg_subtle;       // Subtle background
    Color bg_muted;        // Muted background

    Color border;          // Border color
    Color border_focus;    // Focused border
    Color cursor;          // Cursor color
    Color selection;       // Selection highlight
    Color match_highlight; // Search match highlight
};
```

### Custom Theme

```cpp
// Start with default and customize
scan::Theme custom = scan::default_theme();
custom.name = "My Theme";
custom.colors.primary = {255, 100, 100};      // Red primary
custom.colors.secondary = {100, 100, 255};    // Blue secondary
custom.colors.success = {100, 255, 100};      // Green success
custom.colors.error = {255, 50, 50};          // Red error
custom.colors.text = {240, 240, 240};         // Light text
custom.colors.bg = {30, 30, 30};              // Dark background

scan::set_theme(custom);
```

### Using Theme Colors

```cpp
auto& theme = scan::current_theme();

auto styled = scan::Style()
    .foreground(theme.colors.primary)
    .background(theme.colors.bg)
    .render("Themed text");
```

---

## Tea Architecture (MVU)

For advanced users, Scan provides an Elm-inspired Model-View-Update architecture for building custom interactive components.

### Basic Structure

```cpp
#include <scan/tea/program.hpp>

// 1. Define your model (state)
struct Model {
    int count = 0;
    bool quit = false;
};

// 2. Define initialization
auto init = []() -> std::pair<Model, scan::tea::Cmd> {
    return {Model{}, scan::tea::none()};
};

// 3. Define update logic
auto update = [](Model m, scan::tea::Msg msg)
    -> std::pair<Model, scan::tea::Cmd>
{
    if (auto* key = scan::tea::try_as<scan::tea::KeyMsg>(msg)) {
        switch (key->key) {
            case scan::input::Key::Up:
                m.count++;
                break;
            case scan::input::Key::Down:
                m.count--;
                break;
            case scan::input::Key::Escape:
                m.quit = true;
                return {m, scan::tea::quit()};
        }
    }
    return {m, scan::tea::none()};
};

// 4. Define view (render)
auto view = [](const Model& m) -> std::string {
    return "Count: " + std::to_string(m.count) + "\n\n"
           "[Up] Increment  [Down] Decrement  [Esc] Quit";
};

// 5. Run program
auto final_model = scan::tea::Program<Model>(init, update, view)
    .with_alt_screen(true)
    .run();
```

### Message Types

```cpp
// Key press message
struct KeyMsg {
    scan::input::Key key;
    std::string text;  // For printable characters
    bool alt;
    bool ctrl;
    bool shift;
};

// Mouse message
struct MouseMsg {
    int x, y;
    MouseButton button;
    MouseAction action;
};

// Window resize message
struct WindowSizeMsg {
    int width;
    int height;
};

// Custom message (for async operations)
template<typename T>
struct CustomMsg {
    T data;
};
```

### Commands

```cpp
// No operation
scan::tea::none()

// Quit the program
scan::tea::quit()

// Execute async operation
scan::tea::cmd([](auto send) {
    // Async work here
    send(CustomMsg{result});
})

// Batch multiple commands
scan::tea::batch({cmd1, cmd2, cmd3})
```

### Program Options

```cpp
scan::tea::Program<Model>(init, update, view)
    .with_alt_screen(true)     // Use alternate screen buffer
    .with_mouse(true)          // Enable mouse input
    .with_hidden_cursor(true)  // Hide cursor
    .with_fps(60)              // Frame rate limit
    .run();
```

### Complete Example: Todo List

```cpp
#include <scan/tea/program.hpp>
#include <vector>
#include <string>

struct Todo {
    std::string text;
    bool done = false;
};

struct Model {
    std::vector<Todo> todos;
    size_t cursor = 0;
    std::string input;
    bool editing = false;
};

auto init = []() -> std::pair<Model, scan::tea::Cmd> {
    Model m;
    m.todos = {
        {"Learn Scan", false},
        {"Build awesome TUI", false},
        {"Ship it!", false}
    };
    return {m, scan::tea::none()};
};

auto update = [](Model m, scan::tea::Msg msg)
    -> std::pair<Model, scan::tea::Cmd>
{
    if (auto* key = scan::tea::try_as<scan::tea::KeyMsg>(msg)) {
        if (m.editing) {
            // Input mode
            if (key->key == scan::input::Key::Enter) {
                if (!m.input.empty()) {
                    m.todos.push_back({m.input, false});
                    m.input.clear();
                }
                m.editing = false;
            } else if (key->key == scan::input::Key::Escape) {
                m.input.clear();
                m.editing = false;
            } else if (key->key == scan::input::Key::Backspace) {
                if (!m.input.empty()) m.input.pop_back();
            } else if (!key->text.empty()) {
                m.input += key->text;
            }
        } else {
            // Navigation mode
            switch (key->key) {
                case scan::input::Key::Up:
                case 'k':
                    if (m.cursor > 0) m.cursor--;
                    break;
                case scan::input::Key::Down:
                case 'j':
                    if (m.cursor < m.todos.size() - 1) m.cursor++;
                    break;
                case scan::input::Key::Space:
                case scan::input::Key::Enter:
                    if (!m.todos.empty()) {
                        m.todos[m.cursor].done = !m.todos[m.cursor].done;
                    }
                    break;
                case 'a':
                    m.editing = true;
                    break;
                case 'd':
                    if (!m.todos.empty()) {
                        m.todos.erase(m.todos.begin() + m.cursor);
                        if (m.cursor >= m.todos.size() && m.cursor > 0) {
                            m.cursor--;
                        }
                    }
                    break;
                case 'q':
                case scan::input::Key::Escape:
                    return {m, scan::tea::quit()};
            }
        }
    }
    return {m, scan::tea::none()};
};

auto view = [](const Model& m) -> std::string {
    std::string s = "Todo List\n\n";

    for (size_t i = 0; i < m.todos.size(); i++) {
        const auto& todo = m.todos[i];
        std::string cursor = (i == m.cursor) ? "> " : "  ";
        std::string check = todo.done ? "[x] " : "[ ] ";
        s += cursor + check + todo.text + "\n";
    }

    s += "\n";

    if (m.editing) {
        s += "New todo: " + m.input + "_\n";
        s += "[Enter] Add  [Esc] Cancel";
    } else {
        s += "[j/k] Navigate  [Space] Toggle  [a] Add  [d] Delete  [q] Quit";
    }

    return s;
};

int main() {
    auto final = scan::tea::Program<Model>(init, update, view)
        .with_alt_screen(true)
        .run();

    std::cout << "Completed "
              << std::count_if(final.todos.begin(), final.todos.end(),
                   [](const Todo& t) { return t.done; })
              << " of " << final.todos.size() << " todos\n";

    return 0;
}
```

---

## Advanced Topics

### Custom Components

You can create reusable components by wrapping the Tea architecture:

```cpp
template<typename T>
class CustomSelector {
public:
    CustomSelector& items(std::vector<T> items) {
        m_items = std::move(items);
        return *this;
    }

    CustomSelector& render_item(std::function<std::string(const T&, bool)> fn) {
        m_render = std::move(fn);
        return *this;
    }

    std::optional<T> run() {
        // Implement using Tea architecture
        // ...
    }

private:
    std::vector<T> m_items;
    std::function<std::string(const T&, bool)> m_render;
};
```

### Handling Terminal Resize

```cpp
auto update = [](Model m, scan::tea::Msg msg) {
    if (auto* resize = scan::tea::try_as<scan::tea::WindowSizeMsg>(msg)) {
        m.width = resize->width;
        m.height = resize->height;
        // Adjust layout...
    }
    return {m, scan::tea::none()};
};
```

### Mouse Input

```cpp
auto program = scan::tea::Program<Model>(init, update, view)
    .with_mouse(true)
    .run();

auto update = [](Model m, scan::tea::Msg msg) {
    if (auto* mouse = scan::tea::try_as<scan::tea::MouseMsg>(msg)) {
        if (mouse->action == scan::tea::MouseAction::Press) {
            m.clicked_x = mouse->x;
            m.clicked_y = mouse->y;
        }
    }
    return {m, scan::tea::none()};
};
```

### Async Operations

```cpp
auto update = [](Model m, scan::tea::Msg msg) {
    if (/* user requested data */) {
        return {m, scan::tea::cmd([](auto send) {
            // This runs in a separate thread
            auto data = fetch_data_from_server();
            send(DataLoadedMsg{data});
        })};
    }

    if (auto* loaded = scan::tea::try_as<DataLoadedMsg>(msg)) {
        m.data = loaded->data;
        m.loading = false;
    }

    return {m, scan::tea::none()};
};
```

---

## API Reference

### Namespace Structure

```
scan::
├── TextInput
├── TextArea
├── List
├── ListItem
├── Filter
├── Confirm
├── Spinner
├── SpinnerStyle
├── Table
├── Viewport
├── Pager
├── FilePicker
├── Style
├── Color
├── BorderStyle
├── Position
├── Theme
├── ThemeColors
├── set_theme()
├── current_theme()
├── default_theme()
├── light_theme()
├── join_horizontal()
├── join_vertical()
├── place()
├── pad_left()
├── pad_right()
├── pad_center()
├── truncate()
├── repeat()
├── visible_width()
└── tea::
    ├── Program<Model>
    ├── Msg
    ├── Cmd
    ├── KeyMsg
    ├── MouseMsg
    ├── WindowSizeMsg
    ├── none()
    ├── quit()
    ├── cmd()
    ├── batch()
    └── try_as<T>()
```

---

## Examples

### Interactive Form

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    // Name input
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();
    if (!name) return 1;

    // Age input
    auto age_str = scan::TextInput()
        .prompt("Age: ")
        .char_limit(3)
        .run();
    if (!age_str) return 1;

    // Country selection
    auto country = scan::List()
        .items({"USA", "UK", "Germany", "France", "Japan", "Other"})
        .run();
    if (!country) return 1;

    // Interests (multi-select)
    auto interests = scan::Filter()
        .items({"Technology", "Sports", "Music", "Art", "Science", "Travel"})
        .placeholder("Search interests...")
        .no_limit()
        .run_multi();
    if (!interests) return 1;

    // Confirmation
    auto ok = scan::Confirm()
        .prompt("Submit this information?")
        .run();

    if (ok && *ok) {
        std::cout << "\nSubmitted!\n";
        scan::Table()
            .headers({"Field", "Value"})
            .rows({
                {"Name", *name},
                {"Age", *age_str},
                {"Country", *country},
                {"Interests", [&]() {
                    std::string s;
                    for (size_t i = 0; i < interests->size(); i++) {
                        if (i > 0) s += ", ";
                        s += (*interests)[i];
                    }
                    return s;
                }()}
            })
            .border(scan::BorderStyle::Rounded)
            .print();
    }

    return 0;
}
```

### File Manager

```cpp
#include <scan/scan.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    while (true) {
        auto file = scan::FilePicker()
            .path(".")
            .show_hidden(false)
            .height(20)
            .run();

        if (!file) break;

        if (std::filesystem::is_regular_file(*file)) {
            // Read file content
            std::ifstream ifs(*file);
            std::stringstream buffer;
            buffer << ifs.rdbuf();

            // Display in pager
            scan::Pager()
                .content(buffer.str())
                .title(file->filename().string())
                .line_numbers(true)
                .run();
        }
    }

    return 0;
}
```

### Dashboard

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    auto header = scan::Style()
        .foreground(100, 200, 255)
        .bold()
        .padding(0, 1)
        .render("System Dashboard");

    auto stats = scan::Style()
        .border(scan::BorderStyle::Rounded)
        .padding(1)
        .width(30)
        .render("CPU: 45%\nMemory: 62%\nDisk: 78%");

    auto processes = scan::Table()
        .headers({"PID", "Name", "CPU%"})
        .rows({
            {"1234", "chrome", "12.5"},
            {"5678", "vscode", "8.2"},
            {"9012", "node", "5.1"}
        })
        .border(scan::BorderStyle::Rounded)
        .render();

    auto layout = scan::join_vertical(scan::Position::Left, {
        header,
        "",
        scan::join_horizontal(scan::Position::Top, {stats, "  ", processes})
    });

    std::cout << layout << "\n";

    return 0;
}
```

---

## Building

### CMake Build

```bash
# Configure with examples and tests
cmake -B build \
    -DSCAN_BUILD_EXAMPLES=ON \
    -DSCAN_ENABLE_TESTS=ON

# Build
cmake --build build -j$(nproc)

# Run tests
cd build && ctest

# Run examples
./build/textinput_demo
./build/list_demo
./build/filter_demo
```

### Makefile Build

```bash
make build
make test
make examples
```

### Available CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `SCAN_BUILD_EXAMPLES` | `OFF` | Build example programs |
| `SCAN_ENABLE_TESTS` | `OFF` | Build and enable tests |

---

## Troubleshooting

### Terminal Not Supporting Colors

Ensure your terminal supports ANSI escape codes. Most modern terminals do. Check:
- `$TERM` environment variable
- Try `echo $COLORTERM`

### Unicode Characters Not Displaying

- Ensure your terminal uses a font with Unicode support
- Set locale: `export LC_ALL=en_US.UTF-8`

### Mouse Not Working

- Enable mouse in program: `.with_mouse(true)`
- Some terminals may not support mouse reporting

### Performance Issues

- Use `.with_fps(30)` to limit frame rate
- Minimize string allocations in view function
- Use `Viewport` for large content

### Windows Compatibility

- Use Windows Terminal or ConEmu for best experience
- Classic cmd.exe has limited ANSI support
- Enable virtual terminal processing if needed

---

## License

MIT License - See [LICENSE](../LICENSE) for details.

---

## Acknowledgements

Inspired by the excellent work of [Charm](https://charm.sh/):
- [Bubble Tea](https://github.com/charmbracelet/bubbletea) - TUI framework
- [Bubbles](https://github.com/charmbracelet/bubbles) - TUI components
- [Lip Gloss](https://github.com/charmbracelet/lipgloss) - Style definitions
- [Gum](https://github.com/charmbracelet/gum) - Shell scripting tool

---

*Made with love for terminal UIs*
