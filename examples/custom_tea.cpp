/// @file custom_tea.cpp
/// @brief Demonstrates building a custom component using the Tea runtime

#include <scan/tea/program.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/cmd.hpp>

#include <echo/echo.hpp>

#include <string>
#include <vector>

// A simple counter component using the Tea pattern

/// The Model - holds all state
struct CounterModel {
    int count = 0;
    int min_value = 0;
    int max_value = 100;
    bool quit = false;
};

/// Update - handles messages and returns new state
std::pair<CounterModel, scan::tea::Cmd> counter_update(CounterModel m, scan::tea::Msg msg) {
    if (auto* key = scan::tea::try_as<scan::tea::KeyMsg>(msg)) {
        switch (key->key) {
            case scan::input::Key::Up:
            case scan::input::Key::Right:
                if (m.count < m.max_value) m.count++;
                break;

            case scan::input::Key::Down:
            case scan::input::Key::Left:
                if (m.count > m.min_value) m.count--;
                break;

            case scan::input::Key::Rune:
                if (key->rune == '+' || key->rune == '=') {
                    if (m.count < m.max_value) m.count++;
                } else if (key->rune == '-' || key->rune == '_') {
                    if (m.count > m.min_value) m.count--;
                } else if (key->rune == '0') {
                    m.count = 0;
                } else if (key->rune == 'q' || key->rune == 'Q') {
                    m.quit = true;
                    return {std::move(m), scan::tea::quit()};
                }
                break;

            case scan::input::Key::Enter:
            case scan::input::Key::Escape:
            case scan::input::Key::CtrlC:
                m.quit = true;
                return {std::move(m), scan::tea::quit()};

            default:
                break;
        }
    }

    return {std::move(m), scan::tea::none()};
}

/// View - renders the model to a string
std::string counter_view(const CounterModel& m) {
    std::string view;

    // Title
    echo::format::String title("Counter Demo");
    title.bold().underline().cyan();
    view += title.str() + "\n\n";

    // Counter display
    view += "  Value: ";

    // Color based on value
    echo::format::String value_str(std::to_string(m.count));
    value_str.bold();
    if (m.count > 75) {
        value_str.red();
    } else if (m.count > 50) {
        value_str.yellow();
    } else if (m.count > 25) {
        value_str.green();
    } else {
        value_str.cyan();
    }
    view += value_str.str();
    view += " / " + std::to_string(m.max_value);

    // Progress bar
    view += "\n\n  ";
    int bar_width = 30;
    int filled = (m.count * bar_width) / m.max_value;
    view += "[";
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            view += "█";
        } else {
            view += "░";
        }
    }
    view += "]";

    // Instructions
    view += "\n\n";
    echo::format::String help("  ↑/↓ or +/- to change, 0 to reset, q to quit");
    help.fg(102, 102, 102).italic();
    view += help.str();

    return view;
}

int main() {
    // Initialize
    auto init = []() -> std::pair<CounterModel, scan::tea::Cmd> {
        CounterModel m;
        m.count = 50;  // Start at 50
        return {m, scan::tea::none()};
    };

    // Run the program
    auto final_model = scan::tea::Program<CounterModel>(init, counter_update, counter_view)
        .with_hidden_cursor(true)
        .run();

    // Print final state
    echo::info("Final count: ", final_model.count);

    return 0;
}
