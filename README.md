# Echo

**A modern C++20+ header-only logging library with visual elements and progress indicators.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-yes-green.svg)](https://github.com/robolibs/echo)

## Features

- **Header-only** - Just include and use, no linking required
- **Fluent interface** - Chain colors and modifiers: `.red().bold().italic()`
- **Simple echo()** - Print without log levels: `echo("text").cyan()`
- **Print once** - `.once()` for loop-safe logging
- **Colored output** - Named colors, HEX (#FF5733), RGB (255,87,51), gradients
- **Progress bars** - 6 visual styles, auto-sizing, byte/speed formatting, themes
- **Visual elements** - Boxes, banners, separators with Unicode art
- **Zero overhead** - Compile-time log levels eliminate runtime cost (literally 0ns!)
- **Thread-safe** - Concurrent logging without corruption
- **Flexible log levels** - LOGLEVEL/ECHOLEVEL via compile-time, environment, or runtime
- **No dependencies** - Standard library only

## Quick Start

```cpp
#include <echo/echo.hpp>

int main() {
    // Simple printing (no log levels)
    echo("Hello, world!");
    echo("Colored text").red().bold();
    
    // Logging with levels
    echo::info("Hello, world!");
    echo::debug("Value: ", 42);
    echo::error("Something went wrong!").red();
    
    // Fluent interface with colors
    echo::warn("Warning!").yellow().bold();
    echo::info("Custom color").hex("#FF1493");
    echo::error("RGB color").rgb(255, 87, 51);
    
    // Print only once (useful in loops)
    for (int i = 0; i < 1000; i++) {
        echo::info("This prints only once!").once();
    }
    
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

### Simple Echo (No Log Levels)

Print without log level filtering - always shows, no `[level]` prefix:

```cpp
echo("Simple message");
echo("Colored text").red();
echo("Custom color").hex("#FF5733");
echo("RGB color").rgb(255, 165, 0);
echo("Bold and italic").bold().italic();
```

### Log Levels

Logging with levels - shows `[level]` prefix (bold by default), respects log level filtering:

```cpp
echo::trace("Detailed trace information");    // Level 0 - Most verbose
echo::debug("Debug information");              // Level 1
echo::info("General information");             // Level 2 - Default
echo::warn("Warning message");                 // Level 3
echo::error("Error occurred");                 // Level 4
echo::critical("Critical failure!");           // Level 5 - Least verbose
```

**All level labels are bold by default for better visibility!**

### Multiple Arguments

```cpp
echo::info("User: ", username, " logged in at ", timestamp);
echo::debug("Position: x=", x, " y=", y, " z=", z);
echo("Simple: ", value1, " and ", value2);
```

### Fluent Interface with Colors

Chain colors and text modifiers on any log statement:

```cpp
// Named colors
echo::info("Red text").red();
echo::warn("Green warning").green();
echo::error("Blue error").blue();

// Available colors: .red(), .green(), .blue(), .yellow(), .cyan(), 
//                   .magenta(), .white(), .gray()

// Custom HEX colors
echo::info("Custom color").hex("#FF1493");
echo::warn("Another color").hex("00FFFF");  // # is optional

// Custom RGB colors
echo::info("RGB color").rgb(255, 87, 51);
echo::error("Another RGB").rgb(0, 255, 128);

// Text modifiers
echo::info("Bold text").bold();
echo::info("Italic text").italic();
echo::info("Underlined text").underline();

// Chain everything together!
echo::info("All combined").red().bold().italic().underline();
echo::warn("Custom styled").hex("#FF00FF").bold();
echo("Simple echo styled").cyan().italic();
```

### Print Once (Loop-Safe Logging)

Print a message only once, even in loops - perfect for avoiding log spam:

```cpp
// Prints only on first iteration
for (int i = 0; i < 1000; i++) {
    echo::info("Loop started").once();
    echo::warn("Warning in loop").red().once();
}

// Each unique location prints once
for (int i = 0; i < 100; i++) {
    echo::info("First message").once();   // Prints once
    echo::warn("Second message").once();  // Also prints once
}

// Works with all features
for (int i = 0; i < 1000; i++) {
    echo::error("Styled once").hex("#FF0000").bold().once();
    echo("Simple once").cyan().once();
}
```

**How it works:** `.once()` uses `__FILE__` and `__LINE__` to track unique code locations. Each call site prints only once, ever.

### Log Level Control

**Priority order (highest to lowest):**
1. Runtime API: `echo::set_level(echo::Level::Debug)`
2. Environment variable: `LOGLEVEL=Debug` or `ECHOLEVEL=Trace`
3. In-file define: `#define LOGLEVEL Debug`
4. Build system: `-DLOGLEVEL=Error`

#### Compile-Time Control (Zero Overhead!)

When you set a compile-time level, filtered logs are **completely removed** from the binary:

```cpp
#define LOGLEVEL Error  // or ECHOLEVEL Error
#include <echo/echo.hpp>

echo::trace("Removed from binary!");  // 0 ns overhead - doesn't exist!
echo::debug("Removed from binary!");  // 0 ns overhead - doesn't exist!
echo::info("Removed from binary!");   // 0 ns overhead - doesn't exist!
echo::warn("Removed from binary!");   // 0 ns overhead - doesn't exist!
echo::error("This exists");           // Only this code exists in binary
```

**Build system:**
```bash
# Makefile
make build LOGLEVEL=Error
make build ECHOLEVEL=Debug

# CMake
cmake -DLOGLEVEL=Error ..
cmake -DECHOLEVEL=Debug ..

# Direct compilation
g++ -DLOGLEVEL=Error myapp.cpp
```

**Both LOGLEVEL and ECHOLEVEL are supported. LOGLEVEL takes precedence if both are set.**

Available levels: `Trace`, `Debug`, `Info`, `Warn`, `Error`, `Critical`, `Off`

#### Environment Variable Control

When **no** compile-time level is set, you can use environment variables:

```bash
# Set log level via environment
LOGLEVEL=Debug ./myapp
ECHOLEVEL=Trace ./myapp

# LOGLEVEL takes precedence
LOGLEVEL=Error ECHOLEVEL=Debug ./myapp  # Uses Error
```

**Note:** Environment variables are ignored if a compile-time level is set (to preserve zero-overhead optimization).

#### Runtime Control

```cpp
echo::set_level(echo::Level::Warn);
echo::info("Hidden");
echo::error("Visible");

auto current = echo::get_level();
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

## Performance

Echo is designed to be **invisible in production code** when used with compile-time log levels.

### Zero Overhead with Compile-Time Filtering

When you set `-DLOGLEVEL=Error` (or any level), filtered log statements are **completely removed** from the binary:

```cpp
// Compiled with -DLOGLEVEL=Error
echo::debug("expensive", calculate());  // ← Doesn't exist in binary (0 ns)
echo::info("expensive", calculate());   // ← Doesn't exist in binary (0 ns)
echo::error("This runs", calculate());  // ← Exists in binary
```

**The `calculate()` function is NEVER called for filtered logs!**

### Performance Numbers

| Scenario | Overhead | Notes |
|----------|----------|-------|
| **Compile-time filtered** | **0 ns** | Code doesn't exist in binary |
| Runtime filtered | ~8 ns | Just an integer comparison |
| `.once()` first call | ~50 ns | Hash map insert |
| `.once()` after first | ~30 ns | Hash map lookup + skip |
| Actually printing | ~2 μs | String format + mutex + I/O |

### Best Practices

**✅ DO: Use compile-time levels in production**
```cmake
# CMakeLists.txt
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(LOGLEVEL=Error)
endif()
```

**✅ DO: Use `.once()` in loops**
```cpp
for (int i = 0; i < 1000000; i++) {
    echo::warn("Warning").once();  // Prints once, then free!
}
```

**✅ DO: Don't worry about expensive args with compile-time filtering**
```cpp
// With -DLOGLEVEL=Error, expensive_calc() NEVER runs!
echo::debug("Result: ", expensive_calc());
```

**❌ DON'T: Use `echo()` in performance-critical loops**
```cpp
// echo() always prints - no filtering
for (int i = 0; i < 1000000; i++) {
    echo("iteration");  // BAD - prints 1M times
}

// Use log levels instead
for (int i = 0; i < 1000000; i++) {
    echo::debug("iteration").once();  // GOOD - compile-time filtered
}
```

See [PERFORMANCE.md](PERFORMANCE.md) for detailed benchmarks and optimization guide.

## Configuration

### Compile-Time Options

| Option | Description | Default |
|--------|-------------|---------|
| `LOGLEVEL` | Minimum log level (Trace\|Debug\|Info\|Warn\|Error\|Critical\|Off) | `Info` |
| `ECHOLEVEL` | Alternative to LOGLEVEL (LOGLEVEL takes precedence) | `Info` |
| `ECHO_ENABLE_TIMESTAMP` | Enable timestamps in HH:MM:SS format | Disabled |

### Build System Support

```bash
# Auto-detect build system
make build

# With log level
make build LOGLEVEL=Error
make build ECHOLEVEL=Debug

# Or specify build system explicitly
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

## Quick Reference

### All Features at a Glance

```cpp
#include <echo/echo.hpp>

// Simple printing (no log levels, always shows)
echo("Hello");
echo("Colored").red().bold().italic();
echo("Custom").hex("#FF1493").underline();
echo("RGB").rgb(255, 87, 51);

// Logging with levels (shows [level] prefix, respects filtering)
echo::trace("Trace");    // Level 0
echo::debug("Debug");    // Level 1
echo::info("Info");      // Level 2 (default)
echo::warn("Warning");   // Level 3
echo::error("Error");    // Level 4
echo::critical("Critical"); // Level 5

// Fluent interface - chain anything!
echo::info("Message").red().bold().italic().underline();
echo::warn("Custom").hex("#00FFFF").bold();
echo::error("RGB").rgb(255, 0, 0).italic();

// Print once (loop-safe)
for (int i = 0; i < 1000; i++) {
    echo::info("Prints once").once();
    echo("Also once").cyan().once();
}

// Log level control
#define LOGLEVEL Error        // Compile-time (zero overhead!)
LOGLEVEL=Debug ./myapp        // Environment variable
echo::set_level(Level::Warn); // Runtime API

// Named colors
.red() .green() .blue() .yellow() .cyan() .magenta() .white() .gray()

// Custom colors
.hex("#FF5733")  // HEX color
.rgb(255, 87, 51) // RGB color

// Text modifiers
.bold() .italic() .underline()

// Special
.once()  // Print only once (per location)
```

## Examples

See the `examples/` directory for complete working examples:

- `test_echo.cpp` - Simple echo() usage
- `test_fluent.cpp` - Fluent interface with colors
- `test_once.cpp` - .once() in loops
- `test_level_demo.cpp` - Log level control
- `performance_demo.cpp` - Performance benchmarks
- `color_demo.cpp` - Color and styling examples
- `advanced_progress_demo.cpp` - Progress bars and spinners

## Acknowledgments

See [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md) for credits and acknowledgments.
