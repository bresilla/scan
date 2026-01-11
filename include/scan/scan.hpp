#pragma once

/// @file scan.hpp
/// @brief Main include file for the Scan terminal UI library
///
/// Scan is a modern C++20 header-only terminal user input library
/// inspired by Gum and Bubble Tea, built on top of Echo.
///
/// Usage:
///   #include <scan/scan.hpp>
///
///   auto name = scan::TextInput()
///       .prompt("Name: ")
///       .placeholder("Enter your name")
///       .run();

#include <echo/echo.hpp>

#include <scan/tea/cmd.hpp>
#include <scan/tea/msg.hpp>
#include <scan/tea/program.hpp>

#include <scan/terminal/alt_screen.hpp>
#include <scan/terminal/raw_mode.hpp>
#include <scan/terminal/terminal.hpp>

#include <scan/input/key.hpp>
#include <scan/input/reader.hpp>

#include <scan/render/renderer.hpp>

#include <scan/bubbles/confirm.hpp>
#include <scan/bubbles/filepicker.hpp>
#include <scan/bubbles/filter.hpp>
#include <scan/bubbles/list.hpp>
#include <scan/bubbles/pager.hpp>
#include <scan/bubbles/spinner.hpp>
#include <scan/bubbles/table.hpp>
#include <scan/bubbles/textarea.hpp>
#include <scan/bubbles/textinput.hpp>
#include <scan/bubbles/viewport.hpp>

#include <scan/style/style.hpp>

#include <scan/util/fuzzy.hpp>
#include <scan/util/utf8.hpp>

namespace scan {

    /// Library version
    inline constexpr const char *version = "0.1.0";

} // namespace scan
