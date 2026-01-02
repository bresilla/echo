# Echo

**A modern C++20+ header-only logging library with visual elements and progress indicators.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-yes-green.svg)](https://github.com/robolibs/echo)

## Features

- **Header-only** - Just include and use, no linking required
- **Colored output** - Automatic coloring for log levels, HEX color support with gradients
- **Progress bars** - 6 visual styles, auto-sizing, byte/speed formatting, themes
- **Visual elements** - Boxes, banners, separators with Unicode art
- **Zero overhead** - Compile-time log levels eliminate runtime cost
- **Thread-safe** - Concurrent logging without corruption
- **Flexible** - Runtime control, structured logging, custom types
- **No dependencies** - Standard library only

## Quick Start

```cpp
#include <echo/echo.hpp>

int main() {
    echo::info("Hello, world!");
    echo::debug("Value: ", 42);
    echo::error("Something went wrong!");
    return 0;
}
```

## Installation

### Header-Only

Copy the headers to your project:

```bash
cp -r include/echo /path/to/your/project/include/
```

### CMake

```cmake
include(FetchContent)
FetchContent_Declare(
    echo
    GIT_REPOSITORY https://github.com/robolibs/echo.git
    GIT_TAG main
)
FetchContent_MakeAvailable(echo)
target_link_libraries(your_target echo::echo)
```

## Core Logging

### Log Levels

```cpp
echo::trace("Detailed trace information");
echo::debug("Debug information");
echo::info("General information");
echo::warn("Warning message");
echo::error("Error occurred");
echo::critical("Critical failure!");
```

### Multiple Arguments

```cpp
echo::info("User: ", username, " logged in at ", timestamp);
echo::debug("Position: x=", x, " y=", y, " z=", z);
```

### Compile-Time Control

Zero overhead for disabled log levels:

```cpp
#define LOGLEVEL Debug
#include <echo/echo.hpp>

echo::trace("This is compiled out!");  // No runtime cost
echo::debug("This appears");
```

Available levels: `Trace`, `Debug`, `Info`, `Warn`, `Error`, `Critical`, `Off`

### Runtime Control

```cpp
echo::set_level(echo::Level::Warn);
echo::info("Hidden");
echo::error("Visible");
```

### Timestamps

```cpp
#define ECHO_ENABLE_TIMESTAMP
#include <echo/echo.hpp>

echo::info("Message with timestamp");
// Output: [14:30:45][info] Message with timestamp
```

### Structured Logging

```cpp
echo::info("User login: ", echo::kv("user", "alice", "age", 30, "role", "admin"));
// Output: [info] User login: user=alice age=30 role=admin
```

### Custom Types

```cpp
struct Point {
    int x, y;
    std::string pretty() const {
        return "Point { x: " + std::to_string(x) + ", y: " + std::to_string(y) + " }";
    }
};

Point p{10, 20};
echo::info("Position: ", p);
```

## Visual Elements (banner.hpp)

```cpp
#include <echo/banner.hpp>
```

### Separators

Auto-sizing separators that adapt to terminal width:

```cpp
echo::separator();
echo::separator("Section 1");
echo::separator("IMPORTANT", '=');

// With colors
echo::separator("Success", '-', "#00FF00");
echo::separator("Warning", '=', {"#FF0000", "#FFFF00"});
```

### Boxes

Six box styles:

```cpp
echo::box("Hello World");                    // Single line
echo::box("Title", echo::BoxStyle::Double);  // Double line
echo::box("Info", echo::BoxStyle::Rounded);  // Rounded corners
echo::box("Alert", echo::BoxStyle::Heavy);   // Heavy/bold lines
echo::box("Note", echo::BoxStyle::Dashed);   // Dashed lines
echo::box("Plain", echo::BoxStyle::ASCII);   // ASCII compatible

// With colors
echo::box("Success", echo::BoxStyle::Double, "#00FF00");
echo::box("Fire", echo::BoxStyle::Heavy, {"#FF0000", "#FF7F00", "#FFFF00"});
```

Output:
```
┌─────────────┐
│ Hello World │
└─────────────┘

╔═══════╗
║ Title ║
╚═══════╝
```

### Headers and Banners

```cpp
echo::header("Application Started");
echo::title("My Application");
echo::banner("WELCOME");

// With colors
echo::header("Welcome", "#00FFFF");
echo::banner("READY", echo::BoxStyle::Heavy, "#00FF00");
```

## Progress Bars (wait.hpp)

```cpp
#include <echo/wait.hpp>
```

### Basic Usage

```cpp
echo::progress_bar bar(100);
bar.set_prefix("Loading");
for (int i = 0; i <= 100; ++i) {
    bar.tick();
}
bar.finish();
```

### Bar Styles

Six visual styles:

```cpp
bar.set_bar_style(echo::BarStyle::Classic);  // [===>                    ]
bar.set_bar_style(echo::BarStyle::Blocks);   // [███▓▒░░░░░░░░░░░░░░░░░░]
bar.set_bar_style(echo::BarStyle::Smooth);   // [████████▌░░░░░░░░░░░░░░]
bar.set_bar_style(echo::BarStyle::Arrows);   // [→→→→⇒                   ]
bar.set_bar_style(echo::BarStyle::Dots);     // [●●●●◉○○○○○○○○○○○○○○○○○○]
bar.set_bar_style(echo::BarStyle::ASCII);    // [###>...................]
```

### Pre-configured Themes

Eleven themes combining style and colors:

```cpp
bar.set_theme(echo::BarTheme::fire());     // Red to yellow gradient
bar.set_theme(echo::BarTheme::ocean());    // Blue gradient
bar.set_theme(echo::BarTheme::forest());   // Green gradient
bar.set_theme(echo::BarTheme::sunset());   // Orange to gold gradient
bar.set_theme(echo::BarTheme::neon());     // Magenta/cyan/yellow
```

### Auto-Sizing

Progress bars automatically adapt to terminal width:

```cpp
// No set_bar_width() call = auto-sizing
echo::progress_bar bar(100);
bar.set_prefix("Download");
bar.set_bar_style(echo::BarStyle::Smooth);
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});
```

The bar intelligently calculates available space after accounting for prefix, percentage, time, and speed displays.

### Byte Formatting

Display file sizes with automatic unit conversion:

```cpp
const size_t total_bytes = 50 * 1024 * 1024;  // 50 MB
const size_t chunk_size = 512 * 1024;         // 512 KB
const size_t total_chunks = total_bytes / chunk_size;

echo::progress_bar download(total_chunks);
download.set_prefix("Download");
download.set_show_bytes(true, chunk_size);
download.set_show_speed(true);
download.set_show_elapsed(true);
download.set_bar_style(echo::BarStyle::Smooth);
download.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});
```

Output:
```
Download [████████████████████] 25.0 MB / 50.0 MB [5s, 5.0 MB/s]
```

### Time and Speed Tracking

```cpp
bar.set_show_elapsed(true);     // Show elapsed time
bar.set_show_remaining(true);   // Show estimated remaining time
bar.set_show_speed(true);       // Show transfer speed
```

Output:
```
Processing [████████████░░░░░░░░] 60% [1m30s < 1m0s, 1.2 MB/s]
```

### Colors and Gradients

```cpp
// Single color
bar.set_color("#00FFFF");

// Gradient (smooth color transition)
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});
```

### Custom Width

Override auto-sizing with a fixed width:

```cpp
bar.set_bar_width(40);
```

## Spinners

Fifteen animated spinner styles:

```cpp
echo::spinner spin(echo::spinner_style::aesthetic);
spin.set_message("Processing...");

while (processing) {
    spin.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(spin.get_interval_ms()));
}
spin.stop("Done!");
```

Available styles: `line`, `pipe`, `simple_dots`, `dots_scrolling`, `flip`, `toggle`, `layer`, `point`, `dqpb`, `bouncing_bar`, `bouncing_ball`, `aesthetic`, `binary`, `grow_vertical`, `grow_horizontal`

With colors:
```cpp
spin.set_color("#FF00FF");
spin.set_gradient({"#FF0000", "#00FF00", "#0000FF"});
```

## Step Indicators

Track multi-step workflows:

```cpp
echo::steps workflow({"Initialize", "Load", "Process", "Save"});
workflow.next();      // Step 1/4: Initialize
workflow.complete();  // ✓ Initialize - Complete
workflow.next();
workflow.complete();

// Handle failures
workflow.next();
workflow.fail();  // ✗ Process - Failed

// With colors
workflow.set_color("#00FF00");
```

## Complete Example

```cpp
#include <echo/banner.hpp>
#include <echo/wait.hpp>

int main() {
    echo::banner("FILE PROCESSOR", echo::BoxStyle::Double, "#00FFFF");
    
    echo::steps workflow({"Initialize", "Scan Files", "Process", "Complete"});
    
    workflow.next();
    echo::info("Loading configuration...");
    workflow.complete();
    
    workflow.next();
    echo::spinner scan(echo::spinner_style::aesthetic);
    scan.set_message("Scanning directory...");
    scan.set_color("#FFFF00");
    for (int i = 0; i < 30; ++i) {
        scan.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    scan.stop("Found 1,234 files");
    workflow.complete();
    
    workflow.next();
    echo::progress_bar process(1234);
    process.set_prefix("Processing");
    process.set_theme(echo::BarTheme::ocean());
    process.set_show_elapsed(true);
    process.set_show_remaining(true);
    process.set_show_speed(true);
    
    for (int i = 0; i <= 1234; ++i) {
        process.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    process.finish();
    workflow.complete();
    
    workflow.next();
    workflow.complete();
    
    echo::separator("SUCCESS", '=', "#00FF00");
    echo::info("All files processed successfully!");
    
    return 0;
}
```

## Configuration

### Compile-Time Options

| Option | Description | Default |
|--------|-------------|---------|
| `LOGLEVEL` | Minimum log level (Trace\|Debug\|Info\|Warn\|Error\|Critical\|Off) | `Info` |
| `ECHO_ENABLE_TIMESTAMP` | Enable timestamps in HH:MM:SS format | Disabled |

### Build System Support

```bash
# Auto-detect build system
make build

# Or specify explicitly
BUILD_SYSTEM=cmake make build
BUILD_SYSTEM=xmake make build
BUILD_SYSTEM=zig make build
```

## Testing

```bash
# Run all tests
make test

# Run specific test
make test TEST=test_wait

# Build and run examples
make build
./build/linux/x86_64/release/color_demo
./build/linux/x86_64/release/advanced_progress_demo
./build/linux/x86_64/release/fullwidth_demo
```

## Requirements

- C++20 or later
- Standard library only (no external dependencies)
- Unix/Linux/macOS for terminal width detection

## Color Support

Echo supports 24-bit true color using HEX codes:

```cpp
// Single colors
echo::separator("Title", '-', "#FF5733");
echo::box("Message", echo::BoxStyle::Double, "#00FF00");

// Gradients (smooth transitions)
echo::separator("Rainbow", '=', {"#FF0000", "#FF7F00", "#FFFF00", "#00FF00", "#0000FF", "#8B00FF"});

// Progress bars
bar.set_color("#00FFFF");
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});

// Spinners
spin.set_color("#FF00FF");
spin.set_gradient({"#FF0000", "#00FF00", "#0000FF"});
```

## Contributing

Contributions are welcome. Please submit issues or pull requests.

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Acknowledgments

See [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md) for credits and acknowledgments.
