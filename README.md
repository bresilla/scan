# Scan

**Modern C++20 Terminal UI Library**

A header-only terminal UI library inspired by [Charm's Bubble Tea](https://github.com/charmbracelet/bubbletea) and [Gum](https://github.com/charmbracelet/gum), built on top of [Echo](https://github.com/bresilla/echo). Scan brings the elegance of functional reactive programming to C++ terminal applications with an Elm-inspired Model-View-Update (MVU) architecture.

```
╭──────────────────────────────────────╮
│  scan - Terminal UI made simple      │
│                                      │
│  > TextInput      > Spinner          │
│  > TextArea       > Table            │
│  > List           > Viewport         │
│  > Filter         > Pager            │
│  > Confirm        > FilePicker       │
╰──────────────────────────────────────╯
```

## Features

- **Header-only** - Just include and use
- **C++20** - Modern C++ with concepts, ranges, and more
- **MVU Architecture** - Elm-inspired Model-View-Update pattern
- **Rich Components** - 10+ pre-built interactive components
- **Theming** - Built-in light/dark themes with full customization
- **Styling** - Lip Gloss-style chainable styling API
- **UTF-8** - Full Unicode support
- **Cross-platform** - Linux, macOS, and Windows (with terminal support)

## Quick Start

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    // Simple text input
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

## Installation

### Using CMake FetchContent

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

Copy the `include/scan` directory to your project and include:

```cpp
#include <scan/scan.hpp>
```

---

## Components

### TextInput - Single-Line Text Input

Interactive single-line text input with full editing capabilities.

```cpp
auto email = scan::TextInput()
    .prompt("Email: ")
    .placeholder("user@example.com")
    .char_limit(50)
    .run();
```

**Password Mode:**
```cpp
auto password = scan::TextInput()
    .prompt("Password: ")
    .password(true)
    .mask('*')
    .run();
```

**Options:**
| Method | Description |
|--------|-------------|
| `.prompt(str)` | Prefix text (default: "> ") |
| `.placeholder(str)` | Hint text when empty |
| `.value(str)` | Initial value |
| `.password(bool)` | Enable password mode |
| `.mask(char)` | Mask character (default: '*') |
| `.char_limit(int)` | Maximum characters (0 = unlimited) |
| `.width(int)` | Display width |
| `.prompt_color(r,g,b)` | Prompt text color |
| `.text_color(r,g,b)` | Input text color |
| `.placeholder_color(r,g,b)` | Placeholder color |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Enter` | Submit |
| `Escape` / `Ctrl+C` | Cancel |
| `Ctrl+A` / `Home` | Start of line |
| `Ctrl+E` / `End` | End of line |
| `Ctrl+K` | Kill to end of line |
| `Ctrl+U` | Kill from start |
| `Ctrl+W` | Kill previous word |
| `Backspace` | Delete previous character |
| `Delete` / `Ctrl+D` | Delete at cursor |

---

### TextArea - Multi-Line Text Editor

Full-featured multi-line text editor with optional line numbers.

```cpp
auto message = scan::TextArea()
    .placeholder("Type your message...")
    .width(60)
    .height(10)
    .show_line_numbers(true)
    .run();
```

**Options:**
| Method | Description |
|--------|-------------|
| `.placeholder(str)` | Hint text |
| `.value(str)` | Initial content |
| `.width(int)` | Editor width |
| `.height(int)` | Editor height (default: 10) |
| `.show_line_numbers(bool)` | Display line numbers |
| `.text_color(r,g,b)` | Text color |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Ctrl+D` / `Escape` | Submit |
| `Ctrl+C` | Cancel |
| `Enter` | New line |
| `Tab` | Insert 4 spaces |
| `Up` / `Down` | Navigate lines |
| `Ctrl+K` | Kill to end of line |
| `Ctrl+U` | Kill from start |

---

### List - Selection Lists

Selection component supporting single/multi-select with optional two-line item display (title + description).

**Single Select:**
```cpp
auto color = scan::List()
    .items({"Red", "Green", "Blue", "Yellow"})
    .cursor("> ")
    .run();  // Returns optional<string>
```

**Multi-Select:**
```cpp
auto fruits = scan::List()
    .items({"Apple", "Banana", "Cherry", "Date"})
    .no_limit()
    .selected_prefix("[x] ")
    .unselected_prefix("[ ] ")
    .run_multi();  // Returns optional<vector<string>>
```

**Rich Items with Descriptions (Two-Line Display):**
```cpp
auto editor = scan::List()
    .items({
        {"Vim", "The ubiquitous modal text editor"},
        {"Emacs", "An extensible, customizable text editor"},
        {"VS Code", "A lightweight but powerful source code editor"},
        {"Neovim", "Hyperextensible Vim-based text editor"}
    })
    .height(10)
    .cursor("→ ")
    .run();  // Returns optional<string> (title only)

// Or get full item with description:
auto item = scan::List()
    .items({...})
    .run_item();  // Returns optional<ListItem>

if (item) {
    std::cout << item->title << ": " << item->description << "\n";
}
```

**Options:**
| Method | Description |
|--------|-------------|
| `.items(vector<string>)` | Simple string items |
| `.items(vector<ListItem>)` | Rich items with descriptions |
| `.items({{"title", "desc"}, ...})` | Initializer list syntax |
| `.selected(vector<size_t>)` | Pre-select by indices |
| `.limit(n)` | Max selections |
| `.no_limit()` | Unlimited selections |
| `.height(int)` | Display height |
| `.cursor(str)` | Cursor indicator (default: ">") |
| `.selected_prefix(str)` | Selection marker (default: "[x]") |
| `.unselected_prefix(str)` | Unselected marker (default: "[ ]") |
| `.cursor_color(r,g,b)` | Cursor color |
| `.selected_color(r,g,b)` | Selection color |

**Run Methods:**
| Method | Returns |
|--------|---------|
| `.run()` | `optional<string>` - Single selection (title) |
| `.run_multi()` | `optional<vector<string>>` - Multi selection (titles) |
| `.run_item()` | `optional<ListItem>` - Single with description |
| `.run_items()` | `optional<vector<ListItem>>` - Multi with descriptions |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Enter` | Confirm selection |
| `Space` / `Tab` | Toggle selection (multi-select) |
| `Up` / `Down` / `j` / `k` | Navigate |
| `g` / `G` | Go to top/bottom |
| `a` | Toggle select all (multi-select) |
| `Home` / `End` | First/last item |
| `PageUp` / `PageDown` | Page navigation |

---

### Filter - Fuzzy Search

Real-time fuzzy filtering with match highlighting.

```cpp
auto language = scan::Filter()
    .items({"C++", "Python", "JavaScript", "Rust", "Go", "Java", "Ruby"})
    .placeholder("Search languages...")
    .height(10)
    .run();
```

**Multi-Select Filter:**
```cpp
auto selected = scan::Filter()
    .items(file_list)
    .no_limit()
    .run_multi();
```

**Options:**
| Method | Description |
|--------|-------------|
| `.items(vector<string>)` | Items to filter |
| `.placeholder(str)` | Hint text |
| `.prompt(str)` | Prefix (default: "> ") |
| `.query(str)` | Initial search query |
| `.limit(n)` | Max selections |
| `.no_limit()` | Unlimited selections |
| `.height(int)` | Display height |
| `.case_sensitive(bool)` | Case sensitivity |
| `.match_color(r,g,b)` | Highlight color |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Enter` | Select |
| `Up` / `Down` | Navigate results |
| `Tab` | Toggle selection (multi) |
| `Ctrl+U` | Clear query |
| `Backspace` | Delete character |

---

### Confirm - Yes/No Dialog

Simple confirmation dialog with customizable options.

```cpp
auto confirm = scan::Confirm()
    .prompt("Delete this file?")
    .affirmative("Delete")
    .negative("Cancel")
    .default_value(false)
    .run();

if (confirm && *confirm) {
    // User confirmed deletion
}
```

**Options:**
| Method | Description |
|--------|-------------|
| `.prompt(str)` | Question text |
| `.affirmative(str)` | Yes button (default: "Yes") |
| `.negative(str)` | No button (default: "No") |
| `.default_value(bool)` | Initial selection |
| `.prompt_color(r,g,b)` | Prompt color |
| `.selected_color(fg_r,fg_g,fg_b, bg_r,bg_g,bg_b)` | Selected button |
| `.unselected_color(r,g,b)` | Unselected button |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Enter` | Confirm |
| `Left` / `Right` / `Tab` | Toggle |
| `y` / `Y` | Quick yes |
| `n` / `N` | Quick no |
| `h` / `l` | Vim-style toggle |

---

### Spinner - Progress Indicator

Async task progress indicator with multiple animation styles.

```cpp
auto result = scan::Spinner()
    .title("Loading data...")
    .style(scan::SpinnerStyle::bouncing_bar)
    .color(100, 200, 255)
    .run([]() {
        // Your async task here
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;  // Return value passed through
    });

std::cout << "Result: " << result << "\n";
```

**With Gradient:**
```cpp
scan::Spinner()
    .title("Processing...")
    .style(scan::SpinnerStyle::aesthetic)
    .gradient({"#FF0000", "#FFFF00", "#00FF00"})
    .run([]() { /* task */ return 0; });
```

**Spinner Styles:**
- `line` - Classic rotating line
- `pipe` - Pipe animation
- `simple_dots` - Simple dots
- `dots_scrolling` - Scrolling dots
- `flip` - Flip animation
- `toggle` - Toggle animation
- `layer` - Layer animation
- `point` - Point animation
- `dqpb` - DQPB style
- `bouncing_bar` - Bouncing progress bar
- `bouncing_ball` - Bouncing ball
- `aesthetic` - Aesthetic animation
- `binary` - Binary animation
- `grow_vertical` - Vertical growth
- `grow_horizontal` - Horizontal growth

**Options:**
| Method | Description |
|--------|-------------|
| `.title(str)` | Display message |
| `.style(SpinnerStyle)` | Animation style |
| `.color(r,g,b)` | RGB color |
| `.color("#hex")` | Hex color |
| `.gradient(vector<string>)` | Color gradient |
| `.interval(ms)` | Animation speed |

---

### Table - Data Display

Structured data table with optional row selection.

**Static Display:**
```cpp
scan::Table()
    .headers({"Name", "Language", "Stars"})
    .rows({
        {"React", "JavaScript", "218k"},
        {"Vue", "JavaScript", "206k"},
        {"Angular", "TypeScript", "93k"}
    })
    .border(scan::BorderStyle::Rounded)
    .print();
```

**Interactive Selection:**
```cpp
auto row = scan::Table()
    .headers({"ID", "Task", "Status"})
    .rows({
        {"1", "Build UI", "Done"},
        {"2", "Write tests", "In Progress"},
        {"3", "Deploy", "Pending"}
    })
    .selectable(true)
    .height(5)
    .run();  // Returns optional<size_t> (row index)
```

**Border Styles:**
```
Normal:    ┌───┬───┐    Rounded:   ╭───┬───╮
           │   │   │               │   │   │
           └───┴───┘               ╰───┴───╯

Double:    ╔═══╦═══╗    Thick:     ┏━━━┳━━━┓
           ║   ║   ║               ┃   ┃   ┃
           ╚═══╩═══╝               ┗━━━┻━━━┛
```

**Options:**
| Method | Description |
|--------|-------------|
| `.headers(vector<string>)` | Column headers |
| `.rows(vector<vector<string>>)` | Table data |
| `.widths(vector<int>)` | Custom column widths |
| `.border(BorderStyle)` | Border style |
| `.height(int)` | Visible rows |
| `.selectable(bool)` | Enable selection |
| `.header_style(r,g,b)` | Header color |
| `.border_color(r,g,b)` | Border color |
| `.selected_style(fg..., bg...)` | Selection colors |

---

### Viewport - Scrollable Content

Scrollable content viewer with wrapping support.

```cpp
auto viewport = scan::Viewport()
    .content(large_text)
    .width(80)
    .height(20)
    .wrap(true);

// Programmatic scrolling
viewport.page_down();
viewport.scroll_down(5);
viewport.goto_bottom();

std::cout << viewport.render() << "\n";
std::cout << "Position: " << viewport.scroll_percent() << "%\n";
```

**Options:**
| Method | Description |
|--------|-------------|
| `.content(str)` | Set content |
| `.width(int)` | Display width |
| `.height(int)` | Display height |
| `.wrap(bool)` | Word wrapping |

**Methods:**
| Method | Description |
|--------|-------------|
| `.scroll_up(n)` | Scroll up n lines |
| `.scroll_down(n)` | Scroll down n lines |
| `.page_up()` | Page up |
| `.page_down()` | Page down |
| `.goto_top()` | Jump to top |
| `.goto_bottom()` | Jump to bottom |
| `.scroll_percent()` | Get position (0-100) |
| `.at_top()` / `.at_bottom()` | Position check |
| `.render()` | Get rendered output |

---

### Pager - Full-Screen Viewer

Full-screen document viewer (like `less`).

```cpp
scan::Pager()
    .content(file_contents)
    .title("README.md")
    .line_numbers(true)
    .status_bar(true)
    .run();
```

**Options:**
| Method | Description |
|--------|-------------|
| `.content(str)` | Document content |
| `.title(str)` | Document title |
| `.line_numbers(bool)` | Show line numbers |
| `.status_bar(bool)` | Show status bar |
| `.wrap(bool)` | Word wrapping |
| `.title_color(r,g,b)` | Title color |
| `.status_colors(fg..., bg...)` | Status bar colors |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `q` / `Escape` | Quit |
| `j` / `Down` | Scroll down |
| `k` / `Up` | Scroll up |
| `d` / `Ctrl+D` | Half-page down |
| `u` / `Ctrl+U` | Half-page up |
| `Space` | Page down |
| `g` / `G` | Top / Bottom |

---

### FilePicker - File Browser

Interactive file and directory browser.

**Select Any File:**
```cpp
auto file = scan::FilePicker()
    .path(".")
    .height(15)
    .run();  // Returns optional<filesystem::path>
```

**Filter by Extension:**
```cpp
auto cpp_file = scan::FilePicker()
    .path("/src")
    .extensions({".cpp", ".hpp", ".h"})
    .run();
```

**Directory Selection Only:**
```cpp
auto dir = scan::FilePicker()
    .path(".")
    .file_allowed(false)
    .dir_allowed(true)
    .run();
```

**Options:**
| Method | Description |
|--------|-------------|
| `.path(fs::path)` | Starting directory |
| `.show_hidden(bool)` | Show hidden files |
| `.file_allowed(bool)` | Allow file selection |
| `.dir_allowed(bool)` | Allow directory selection |
| `.extensions(vector<string>)` | Filter extensions |
| `.height(int)` | Display height |
| `.dir_color(r,g,b)` | Directory color |
| `.file_color(r,g,b)` | File color |

**Keyboard Shortcuts:**
| Key | Action |
|-----|--------|
| `Enter` | Select / Enter directory |
| `Left` / `Backspace` / `h` | Parent directory |
| `Right` / `l` | Enter directory |
| `.` | Toggle hidden files |
| `Up` / `Down` / `j` / `k` | Navigate |
| `g` / `G` | Top / Bottom |

---

## Style System

Lip Gloss-inspired chainable styling API.

```cpp
auto styled = scan::Style()
    .foreground(255, 200, 100)
    .background(50, 50, 50)
    .bold()
    .padding(1, 2)
    .border(scan::BorderStyle::Rounded)
    .border_foreground(100, 200, 100)
    .width(40)
    .render("Hello, World!");

std::cout << styled << "\n";
```

**Text Styling:**
```cpp
Style()
    .bold()              // Bold text
    .italic()            // Italic text
    .underline()         // Underlined
    .strikethrough()     // Strikethrough
    .faint()             // Dim/faint
    .reverse()           // Reverse colors
    .blink()             // Blinking
```

**Colors:**
```cpp
Style()
    .foreground(r, g, b)           // Text color
    .background(r, g, b)           // Background color
    .foreground(Color{r, g, b})    // Using Color struct
```

**Layout:**
```cpp
Style()
    .padding(all)                      // All sides
    .padding(vertical, horizontal)     // V and H
    .padding(top, right, bottom, left) // Individual
    .padding_left(n)                   // Single side

    .margin(all)                       // All sides
    .margin(vertical, horizontal)      // V and H

    .width(int)                        // Fixed width
    .height(int)                       // Fixed height
    .max_width(int)                    // Maximum width

    .align(Position::Center, Position::Top)  // Alignment

    .border(BorderStyle::Rounded)      // Add border
    .border_foreground(r, g, b)        // Border color
```

**Layout Utilities:**
```cpp
// Join horizontally
auto row = scan::join_horizontal(Position::Top, {box1, box2, box3});

// Join vertically
auto column = scan::join_vertical(Position::Center, {box1, box2});

// Place at position
auto placed = scan::place(80, 24, Position::Center, Position::Center, content);

// String utilities
scan::pad_right(str, width)
scan::pad_left(str, width)
scan::pad_center(str, width)
scan::truncate(str, max_width)
scan::repeat(str, n)
scan::visible_width(str)  // Width ignoring ANSI codes
```

---

## Theme System

Built-in theming with light and dark modes.

```cpp
// Use dark theme (default)
scan::set_theme(scan::default_theme());

// Use light theme
scan::set_theme(scan::light_theme());

// Access current theme
auto& theme = scan::current_theme();
auto primary = theme.colors.primary;
```

**Theme Colors:**
```cpp
struct ThemeColors {
    Color primary;        // Accent color
    Color secondary;      // Secondary accent
    Color success;        // Success/green
    Color error;          // Error/red
    Color warning;        // Warning/orange

    Color text;           // Main text
    Color text_muted;     // Secondary text
    Color text_subtle;    // Hints/placeholders

    Color bg;             // Background
    Color bg_subtle;      // Subtle background
    Color bg_muted;       // Muted background

    Color border;         // Border color
    Color border_focus;   // Focused border
    Color cursor;         // Cursor color
    Color selection;      // Selection highlight
    Color match_highlight;// Search matches
};
```

**Custom Theme:**
```cpp
scan::Theme custom = scan::default_theme();
custom.colors.primary = {255, 100, 100};
custom.colors.success = {100, 255, 100};
scan::set_theme(custom);
```

---

## Tea Architecture (MVU)

For advanced users, build custom components using the Elm-inspired architecture.

```cpp
#include <scan/tea/program.hpp>

// Define your model
struct CounterModel {
    int count = 0;
    bool quit = false;
};

// Initialize
auto init = []() -> std::pair<CounterModel, scan::tea::Cmd> {
    return {CounterModel{}, scan::tea::none()};
};

// Update
auto update = [](CounterModel m, scan::tea::Msg msg)
    -> std::pair<CounterModel, scan::tea::Cmd>
{
    if (auto* key = scan::tea::try_as<scan::tea::KeyMsg>(msg)) {
        if (key->key == scan::input::Key::Up) m.count++;
        if (key->key == scan::input::Key::Down) m.count--;
        if (key->key == scan::input::Key::Escape) {
            m.quit = true;
            return {m, scan::tea::quit()};
        }
    }
    return {m, scan::tea::none()};
};

// View
auto view = [](const CounterModel& m) -> std::string {
    return "Count: " + std::to_string(m.count) + "\n"
           "[Up/Down] Change  [Esc] Quit";
};

// Run
auto final = scan::tea::Program<CounterModel>(init, update, view)
    .with_alt_screen(true)
    .run();
```

**Program Options:**
```cpp
tea::Program<Model>(init, update, view)
    .with_alt_screen(bool)     // Use alternate screen
    .with_mouse(bool)          // Enable mouse
    .with_hidden_cursor(bool)  // Hide cursor
    .run()
```

---

## Complete Example

```cpp
#include <scan/scan.hpp>
#include <iostream>

int main() {
    // Step 1: Get user name
    auto name = scan::TextInput()
        .prompt("Name: ")
        .placeholder("Enter your name")
        .run();

    if (!name) return 1;

    // Step 2: Select favorite color
    auto color = scan::List()
        .items({
            {"Red", "The color of passion"},
            {"Green", "The color of nature"},
            {"Blue", "The color of calm"}
        })
        .run();

    if (!color) return 1;

    // Step 3: Select hobbies
    auto hobbies = scan::Filter()
        .items({"Reading", "Gaming", "Coding", "Music", "Sports", "Art"})
        .placeholder("Search hobbies...")
        .no_limit()
        .run_multi();

    if (!hobbies) return 1;

    // Step 4: Confirm
    auto ok = scan::Confirm()
        .prompt("Save profile?")
        .run();

    if (!ok || !*ok) return 1;

    // Step 5: Show spinner while "saving"
    scan::Spinner()
        .title("Saving profile...")
        .style(scan::SpinnerStyle::bouncing_bar)
        .run([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return 0;
        });

    // Step 6: Display result
    std::cout << scan::Style()
        .foreground(100, 255, 100)
        .bold()
        .render("Profile saved!")
        << "\n\n";

    scan::Table()
        .headers({"Field", "Value"})
        .rows({
            {"Name", *name},
            {"Color", *color},
            {"Hobbies", [&]() {
                std::string s;
                for (size_t i = 0; i < hobbies->size(); i++) {
                    if (i > 0) s += ", ";
                    s += (*hobbies)[i];
                }
                return s;
            }()}
        })
        .border(scan::BorderStyle::Rounded)
        .print();

    return 0;
}
```

---

## Building

```bash
# Configure
cmake -B build -DSCAN_BUILD_EXAMPLES=ON -DSCAN_ENABLE_TESTS=ON

# Build
cmake --build build

# Run examples
./build/list_demo
./build/filter_demo
./build/confirm_demo
```

Or using make:
```bash
make build
make test
```

---

## Dependencies

- **C++20** compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **[Echo](https://github.com/bresilla/echo)** - Terminal utilities (fetched automatically)

---

## License

MIT License - See [LICENSE](LICENSE) for details.

---

## Acknowledgements

Inspired by the excellent work of [Charm](https://charm.sh/):
- [Bubble Tea](https://github.com/charmbracelet/bubbletea) - TUI framework
- [Bubbles](https://github.com/charmbracelet/bubbles) - TUI components
- [Lip Gloss](https://github.com/charmbracelet/lipgloss) - Style definitions
- [Gum](https://github.com/charmbracelet/gum) - Shell scripting tool

---

Made with love for terminal UIs
