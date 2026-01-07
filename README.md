# Echo

**A modern C++20 logging library with modular architecture, rich formatting, visual widgets, and zero-overhead performance.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Version](https://img.shields.io/badge/version-0.0.21-blue.svg)](https://github.com/robolibs/echo)

## Overview

Echo is a high-performance, header-mostly logging library designed for modern C++20 applications. It combines powerful logging capabilities with rich terminal UI widgets, providing everything from simple console output to sophisticated multi-sink logging with custom formatters, category filtering, and visual progress indicators.

**Key Features:**
- **Zero overhead** - Compile-time log filtering eliminates runtime cost (literally 0ns!)
- **Modular architecture** - Enable only the sinks you need via compile-time flags
- **Performance first** - 17M+ ops/sec for filtered logs, 3-4M ops/sec for active logging
- **Rich formatting** - Pattern formatters, custom formatters, color support
- **Visual widgets** - Progress bars, spinners, banners, boxes with themes
- **Developer friendly** - Fluent API, category filtering, thread-safe

Echo is built for applications that demand both flexibility and performance, from embedded systems to high-throughput servers.

## Quick Start

```cpp
#include <echo/echo.hpp>

int main() {
    // Simple printing (no log levels)
    echo("Hello, world!");
    echo("Colored text").red().bold();

    // Logging with levels
    echo::info("Application started");
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

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                            ECHO LIBRARY                                  │
├──────────────┬──────────────┬──────────────┬──────────────┬─────────────┤
│     Core     │    Sinks     │  Formatters  │   Filters    │   Utils     │
│              │              │              │              │             │
│  ┌────────┐  │  ┌────────┐  │  ┌────────┐  │  ┌────────┐  │ ┌────────┐ │
│  │ Proxy  │  │  │Console │  │  │Pattern │  │  │Category│  │ │ Color  │ │
│  │ Level  │  │  │  File  │  │  │ Custom │  │  │  Level │  │ │Terminal│ │
│  │ Mutex  │  │  │Syslog* │  │  │Default │  │  │        │  │ │  Hash  │ │
│  │  Once  │  │  │Network*│  │  │        │  │  │        │  │ │        │ │
│  │  Time  │  │  │ Null*  │  │  │        │  │  │        │  │ │        │ │
│  └────────┘  │  └────────┘  │  └────────┘  │  └────────┘  │ └────────┘ │
│              │              │              │              │             │
└──────────────┴──────────────┴──────────────┴──────────────┴─────────────┘
       │               │               │              │              │
       └───────────────┴───────────────┴──────────────┴──────────────┘
                                    │
                            ┌───────▼────────┐
                            │   Your App     │
                            └────────────────┘

* Optional sinks (compile-time flags)
```

**Data Flow:**
```
Log Call → Compile-Time Filter → Runtime Filter → Category Filter
    ↓
Format Message → Formatter (Pattern/Custom/Default)
    ↓
Sink Registry → [ConsoleSink, FileSink*, SyslogSink*, NetworkSink*, NullSink*]
    ↓
Output (stdout/stderr, file, syslog, network, /dev/null)
```

## Installation

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
  echo
  GIT_REPOSITORY https://github.com/robolibs/echo.git
  GIT_TAG v0.0.21
)
FetchContent_MakeAvailable(echo)

target_link_libraries(your_target PRIVATE echo::echo)
```

### XMake

```lua
add_requires("echo")

target("your_target")
    set_kind("binary")
    add_packages("echo")
    add_files("src/*.cpp")
```

### Manual Installation

```bash
git clone https://github.com/robolibs/echo.git
cd echo

# Using Make (auto-detects cmake/xmake)
make config
make build
make test

# Or directly with CMake
mkdir build && cd build
cmake -DECHO_BUILD_EXAMPLES=ON -DECHO_ENABLE_TESTS=ON ..
make -j$(nproc)
ctest

# Or with XMake
xmake f --examples=y --tests=y
xmake
xmake test
```

## Core Features

### 1. Log Levels

Six log levels with compile-time and runtime filtering:

```cpp
echo::trace("Detailed trace information");
echo::debug("Debug information");
echo::info("Informational message");
echo::warn("Warning message");
echo::error("Error message");
echo::critical("Critical error");
```

**Compile-time filtering** (zero overhead):
```bash
# CMake
cmake -DLOGLEVEL=Error ..

# XMake
xmake f -c -DLOGLEVEL=Error

# Result: trace/debug/info/warn calls are completely removed from binary
```

**Runtime filtering**:
```cpp
echo::set_level(echo::Level::Warn);  // Only Warn and above
```

### 2. Fluent Interface with Colors

Chain colors, styles, and modifiers:

```cpp
// Standard colors
echo::info("Message").red().bold();
echo::warn("Warning").yellow().italic();

// Custom colors
echo::info("HEX color").hex("#FF1493");
echo::error("RGB color").rgb(255, 87, 51);

// Multiple modifiers
echo::critical("Alert!").red().bold().underline();
```

**Available colors**: red, green, yellow, blue, magenta, cyan, white, gray

**Available styles**: bold, italic, underline

### 3. Rate Limiting & Conditional Logging

```cpp
// Print only once (useful in loops)
for (int i = 0; i < 1000000; i++) {
    echo::warn("This prints only once!").once();
}

// Rate-limited printing (every N milliseconds)
for (int i = 0; i < 1000000; i++) {
    echo::info("Prints every 1 second").every(1000);
}

// Conditional printing
bool debug_mode = true;
echo::debug("Debug info").when(debug_mode);
```

### 4. In-place Updates

Perfect for progress indicators and status updates:

```cpp
for (int i = 0; i <= 100; i++) {
    echo("Progress: ", i, "%").inplace();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
echo("\n");  // Move to next line when done
```

### 5. Multiple Sinks

Route logs to multiple destinations simultaneously:

```cpp
#define ECHO_ENABLE_FILE_SINK
#define ECHO_ENABLE_SYSLOG_SINK
#include <echo/echo.hpp>

int main() {
    // Clear default console sink
    echo::clear_sinks();

    // Add console sink with custom level
    auto console = std::make_shared<echo::ConsoleSink>();
    console->set_level(echo::Level::Info);
    echo::add_sink(console);

    // Add file sink with rotation
    auto file = std::make_shared<echo::FileSink>("app.log");
    file->enable_rotation(5 * 1024 * 1024, 3);  // 5MB, keep 3 files
    echo::add_sink(file);

    // Add syslog sink (Unix only)
    auto syslog = std::make_shared<echo::SyslogSink>("myapp");
    echo::add_sink(syslog);

    // Logs go to all sinks
    echo::info("Application started");
    echo::error("Error logged to console, file, and syslog");

    return 0;
}
```

**Available sinks:**
- **ConsoleSink** - Always available (stdout/stderr)
- **FileSink** - File logging with rotation (`-DECHO_ENABLE_FILE_SINK`)
- **SyslogSink** - Unix syslog integration (`-DECHO_ENABLE_SYSLOG_SINK`)
- **NetworkSink** - TCP/UDP logging (`-DECHO_ENABLE_NETWORK_SINK`)
- **NullSink** - Discard output (`-DECHO_ENABLE_NULL_SINK`)

### 6. Custom Formatters

Three formatter types for maximum flexibility:

**DefaultFormatter** - Simple timestamp + level:
```cpp
auto formatter = std::make_shared<echo::DefaultFormatter>(true, true);
// Output: [14:30:45][info] Message
```

**PatternFormatter** - Customizable patterns:
```cpp
auto formatter = std::make_shared<echo::PatternFormatter>(
    "[{timestamp}] [{level}] {file}:{line} - {message}"
);
echo::set_global_formatter(formatter);
// Output: [2026-01-08 14:30:45] [INFO] main.cpp:42 - Message
```

**Pattern placeholders:**
- `{timestamp}` or `{time}` - Timestamp
- `{level}` - Log level
- `{message}` or `{msg}` - Log message
- `{file}` - Source file
- `{line}` - Line number
- `{function}` or `{func}` - Function name
- `{thread}` - Thread ID

**CustomFormatter** - Lambda-based:
```cpp
auto json_formatter = std::make_shared<echo::CustomFormatter>(
    [](const echo::LogRecord& rec) {
        return "{\"time\":\"" + rec.timestamp + 
               "\",\"level\":\"" + std::string(echo::detail::level_name(rec.level)) + 
               "\",\"msg\":\"" + rec.message + "\"}";
    }
);
```

### 7. Category-Based Filtering

Hierarchical category system with wildcard support:

```cpp
// Set category-specific log levels
echo::set_category_level("network", echo::Level::Debug);
echo::set_category_level("database", echo::Level::Warn);
echo::set_category_level("app.*", echo::Level::Info);  // Wildcard

// Log with categories
echo::category("network").debug("TCP connection established");
echo::category("network").info("Received 1024 bytes");

echo::category("database").debug("Query executed");  // Filtered (< Warn)
echo::category("database").warn("Slow query detected");  // Logged

echo::category("app.auth").info("User logged in");  // Matches app.*
echo::category("app.api").error("Request failed");
```

**Hierarchical matching:**
```
Category: "app.network.tcp"
Search order:
1. "app.network.tcp" (exact match)
2. "app.network.*" (parent wildcard)
3. "app.*" (grandparent wildcard)
4. Global level (fallback)
```

## Visual Widgets

### Progress Bars

Rich progress bars with themes, time tracking, and byte formatting:

```cpp
#include <echo/widget.hpp>

// Basic progress bar
echo::progress_bar bar(100);
for (int i = 0; i <= 100; ++i) {
    bar.set_progress(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
bar.finish();

// Themed progress bar with time and speed
echo::progress_bar download(1024 * 1024 * 100);  // 100MB
download.set_theme(echo::BarTheme::ocean());
download.set_show_elapsed(true);
download.set_show_remaining(true);
download.set_show_bytes(true, 1024);  // Show as KB/MB/GB
download.set_show_speed(true);

for (size_t i = 0; i <= 1024 * 1024 * 100; i += 1024 * 100) {
    download.set_progress(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
download.finish();
```

**6 Bar Styles:**
- Classic: `[===>    ]`
- Blocks: `[███▓▒░  ]`
- Smooth: `[████▌   ]`
- Arrows: `[→→→→    ]`
- Dots: `[●●●●○○○○]`
- ASCII: `[###>... ]`

**5 Pre-configured Themes:**
- fire() - Red-orange-yellow gradient
- ocean() - Dark blue to cyan
- forest() - Dark green to light green
- sunset() - Orange-red-gold
- neon() - Magenta-cyan-yellow

### Spinners

15 animated spinner styles:

```cpp
echo::spinner spin(echo::SpinnerStyle::dots_scrolling);
spin.set_message("Loading...");
spin.set_color("#00FF00");

for (int i = 0; i < 50; ++i) {
    spin.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(
        spin.get_interval_ms()
    ));
}
spin.stop("Done!");
```

**Spinner styles:** line, pipe, simple_dots, dots_scrolling, flip, toggle, layer, point, dqpb, bouncing_bar, bouncing_ball, aesthetic, binary, grow_vertical, grow_horizontal

### Step Indicators

Track multi-step workflows:

```cpp
echo::steps workflow({"Initialize", "Process", "Validate", "Complete"});
workflow.set_color("#00FFFF");

workflow.next();  // Initialize
std::this_thread::sleep_for(std::chrono::seconds(1));
workflow.complete();

workflow.next();  // Process
std::this_thread::sleep_for(std::chrono::seconds(1));
workflow.complete();

workflow.next();  // Validate
std::this_thread::sleep_for(std::chrono::seconds(1));
workflow.fail();  // Mark as failed

workflow.next();  // Complete
```

### Banners, Boxes, and Separators

```cpp
// Separator
echo::separator("Section Title", '=');
echo::separator("Gradient", '─', {"#FF0000", "#00FF00", "#0000FF"});

// Box (6 styles: Single, Double, Rounded, Heavy, Dashed, ASCII)
echo::box("Important Message", echo::BoxStyle::Double);
echo::box("Colored Box", echo::BoxStyle::Rounded, "#FF00FF");

// Header
echo::header("Application Name");
echo::header("Colored Header", "#00FFFF");

// Title
echo::title("Chapter 1", '=');

// Banner
echo::banner("WELCOME", echo::BoxStyle::Heavy);
```

## Advanced Color Utilities

Comprehensive color manipulation library:

```cpp
#include <echo/utils/color.hpp>

// Color conversions
auto rgb = echo::hex_to_rgb("#FF1493");
auto hex = echo::to_hex(255, 20, 147);

// Color mixing (7 modes)
auto mixed = echo::mix(color1, color2, 0.5);
auto added = echo::add(color1, color2);
auto multiplied = echo::multiply(color1, color2);
auto screened = echo::screen(color1, color2);
auto overlayed = echo::overlay(color1, color2);

// Color adjustments (8 operations)
auto lighter = echo::lighten(rgb, 0.2);
auto darker = echo::darken(rgb, 0.2);
auto brighter = echo::brighten(rgb, 50);
auto dimmer = echo::dim(rgb, 50);
auto saturated = echo::saturate(rgb, 0.3);
auto desaturated = echo::desaturate(rgb, 0.3);
auto inverted = echo::invert(rgb);
auto gray = echo::grayscale(rgb);

// Color analysis
auto lum = echo::luminance(rgb);
bool dark = echo::is_dark(rgb);
bool light = echo::is_light(rgb);
auto dist = echo::distance(color1, color2);
auto contrast = echo::contrast_ratio(color1, color2);

// Color schemes (7 generators)
auto comp = echo::complementary(rgb);
auto analog = echo::analogous(rgb, 5, 30);
auto triadic = echo::triadic(rgb);
auto tints = echo::tints(rgb, 5);
auto shades = echo::shades(rgb, 5);
auto tones = echo::tones(rgb, 5);
auto temp = echo::from_temperature(0.5);  // Warm color
```

## String Formatting

Powerful chainable string formatting with 60+ methods:

```cpp
#include <echo/format.hpp>

// Colors and styles
auto str = echo::format::String("Hello")
    .red().bold().underline();

// RGB/HEX colors
auto custom = echo::format::String("Custom")
    .fg(255, 100, 50)
    .bg("#1E1E1E");

// Text alignment
auto aligned = echo::format::String("Text")
    .center(20)
    .border('*');

// Text transformation
auto transformed = echo::format::String("hello world")
    .title_case()
    .bold();

// Truncation and wrapping
auto truncated = echo::format::String("Long text...")
    .ellipsis(10);

auto wrapped = echo::format::String("Very long text that needs wrapping")
    .wrap(20);

// Numeric formatting
auto bytes = echo::format::String("1048576")
    .format_bytes();  // "1.00 MB"

auto duration = echo::format::String("3665")
    .format_duration();  // "1h 1m 5s"

// Special formatting
auto progress = echo::format::String("50")
    .progress(50);  // Progress bar representation

auto badge = echo::format::String("value")
    .badge("label");  // [label: value]
```

## Performance

Echo is designed for **invisible overhead** in production code.

### Benchmark Results

| Scenario | Latency | Throughput | Notes |
|----------|---------|------------|-------|
| **Compile-time filtered** | **0 ns** | **∞** | Code doesn't exist in binary |
| Runtime filtered | 56-60 ns | 16.7-17.7M ops/s | Just integer comparison |
| Active logging (NullSink) | 260-290 ns | 3.4-3.8M ops/s | Formatting only |
| Active logging (file) | 350 ns | 2.9M ops/s | Includes file I/O |
| Active logging (console) | 650 ns | 1.5M ops/s | Includes terminal I/O |
| `.once()` overhead | +3-5 ns | - | After first call |
| Category filtering | +30-40 ns | - | Hash lookup |

### Zero-Cost Abstraction

When you compile with `-DLOGLEVEL=Error`, filtered log statements are **completely removed** from the binary:

```cpp
// Compiled with -DLOGLEVEL=Error
echo::debug("expensive", calculate());  // ← Doesn't exist in binary (0 ns)
echo::info("expensive", calculate());   // ← Doesn't exist in binary (0 ns)
echo::error("This runs", calculate());  // ← Exists in binary
```

**The `calculate()` function is NEVER called for filtered logs!**

### Best Practices

**✅ DO: Use compile-time levels in production**
```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(LOGLEVEL=Error)
endif()
```

**✅ DO: Use `.once()` in loops**
```cpp
for (int i = 0; i < 1000000; i++) {
    echo::warn("Warning").once();  // Prints once, then ~60ns overhead
}
```

**✅ DO: Enable only needed sinks**
```cpp
#define ECHO_ENABLE_FILE_SINK  // Only enable what you need
#include <echo/echo.hpp>
```

**❌ DON'T: Use `echo()` in performance-critical loops**
```cpp
// BAD - echo() always prints (no filtering)
for (int i = 0; i < 1000000; i++) {
    echo("iteration");  // Prints 1M times
}

// GOOD - Use log levels with compile-time filtering
for (int i = 0; i < 1000000; i++) {
    echo::debug("iteration").once();  // Compile-time filtered
}
```

See [misc/PERFORMANCE.md](misc/PERFORMANCE.md) for detailed benchmarks and optimization guide.

## Testing

```bash
# Run all tests
make test

# Run specific test
make test TEST=test_category

# Run with sanitizers
make test-asan    # AddressSanitizer
make test-tsan    # ThreadSanitizer
make test-ubsan   # UndefinedBehaviorSanitizer
make test-sanitizers  # All sanitizers

# Build and run examples
make build
./build/color_demo
./build/advanced_progress_demo
```

**Test Coverage:**
- 17 test files (150+ test cases)
- Core features: levels, sinks, formatters, categories
- Edge cases: concurrency, invalid inputs, long messages, Unicode
- Integration: multi-sink, real-world scenarios
- Thread safety: up to 20 concurrent threads

## Build System

Echo supports **three build systems** with automatic detection:

### Make (Unified Interface)

```bash
make build        # Build project (auto-detects cmake/xmake)
make config       # Configure build
make reconfig     # Full reconfigure (cleans cache)
make test         # Run tests
make clean        # Clean build directory
make help         # Show all targets
```

### CMake

```bash
mkdir build && cd build
cmake -DECHO_BUILD_EXAMPLES=ON -DECHO_ENABLE_TESTS=ON ..
make -j$(nproc)
ctest
```

**CMake Options:**
- `-DECHO_BUILD_EXAMPLES=ON` - Build examples
- `-DECHO_ENABLE_TESTS=ON` - Enable tests
- `-DECHO_BUILD_BENCHMARKS=ON` - Build benchmarks
- `-DCOMPILER=gcc|clang` - Compiler selection
- `-DECHO_ENABLE_SIMD=ON` - SIMD optimizations (default: ON)

### XMake

```bash
xmake f --examples=y --tests=y
xmake
xmake test
```

**XMake Options:**
- `--examples=y` - Build examples
- `--tests=y` - Enable tests
- `--toolchain=gcc|clang` - Compiler selection
- `--simd=true` - SIMD optimizations (default: true)

## Compiler Support

- **C++20 or later** (required)
- **GCC** 10+ (recommended)
- **Clang** 12+ (recommended)
- **MSVC** 19.29+ (Visual Studio 2019 16.11+)

**SIMD Support:**
- x86_64: AVX, AVX2, FMA
- ARM64: NEON (enabled by default)
- ARM32: NEON with hard float

## Requirements

- C++20 compiler
- Standard library only (no external dependencies)
- Unix/Linux/macOS for full terminal features
- Optional: doctest for tests

## Documentation

Generate API documentation:

```bash
make docs TYPE=doxygen
# Open docs/doxygen/html/index.html
```

## Examples

The project includes 36 comprehensive examples:

**Core Logging:**
- main.cpp - Basic logging
- performance_demo.cpp - Performance characteristics
- test_once.cpp, test_every_when.cpp - Rate limiting

**Colors:**
- color_demo.cpp - HEX color support
- color_manipulation_demo.cpp - RGB utilities
- format_demo.cpp - format::String colors
- format_text_demo.cpp - Text manipulation

**Formatters:**
- formatter_demo.cpp - Custom formatters
- formatter_basic_demo.cpp - Direct formatter usage
- global_formatter_demo.cpp - Global formatter API

**Sinks:**
- sink_demo.cpp - Sink system
- null_sink_demo.cpp - NullSink for testing
- file_logging_demo.cpp - File logging

**Widgets:**
- wait_demo.cpp - Progress bars, spinners, steps
- advanced_progress_demo.cpp - Advanced progress features
- separator_demo.cpp - Banners and separators
- fullwidth_demo.cpp - Auto-sizing progress bars

**Benchmarks (11 files):**
- bench_basic.cpp, bench_levels.cpp, bench_sinks.cpp
- bench_formatters.cpp, bench_categories.cpp
- bench_compile_time.cpp, bench_once.cpp
- bench_threading.cpp, bench_memory.cpp
- bench_latency.cpp, bench_vs_spdlog.cpp

## License

MIT License - see [LICENSE](./LICENSE) for details.

## Acknowledgments

Made possible thanks to [these amazing projects](ACKNOWLEDGMENTS.md).

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.
