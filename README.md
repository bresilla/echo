# Echo 🎵

> **A modern, beautiful C++20+ header-only logging library**  
> Simple. Colorful. Powerful. Zero dependencies.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-yes-green.svg)](https://github.com/robolibs/echo)

---

## ✨ Features at a Glance

| Feature | Description |
|---------|-------------|
| 🎨 **Beautiful Output** | Colored logs, HEX color support, gradients |
| 📊 **Progress Bars** | 6 styles, auto-sizing, byte/speed formatting |
| 🎯 **Visual Elements** | Boxes, banners, separators with Unicode art |
| ⚡ **Zero Overhead** | Compile-time log levels, header-only |
| 🧵 **Thread-Safe** | Concurrent logging without corruption |
| 🔧 **Flexible** | Runtime control, structured logging, custom types |
| 📦 **No Dependencies** | Standard library only, works everywhere |

---

## 🚀 Quick Start

```cpp
#include <echo/echo.hpp>

int main() {
    echo::info("Hello, world!");
    echo::debug("Value: ", 42);
    echo::error("Something went wrong!");
    return 0;
}
```

**Output:**
```
[info] Hello, world!
[debug] Value: 42
[error] Something went wrong!
```

---

## 📦 Installation

### Header-Only (Simplest)

Just copy the headers to your project:

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

### Manual

```bash
git clone https://github.com/robolibs/echo.git
# Add echo/include to your include path
```

---

## 📚 Core Logging

### Log Levels

Six log levels with automatic coloring:

```cpp
echo::trace("Detailed trace information");    // Gray
echo::debug("Debug information");             // Cyan
echo::info("General information");            // Green
echo::warn("Warning message");                // Yellow
echo::error("Error occurred");                // Red
echo::critical("Critical failure!");          // Magenta
```

### Multiple Arguments

Log anything, anywhere:

```cpp
echo::info("User: ", username, " logged in at ", timestamp);
echo::debug("Position: x=", x, " y=", y, " z=", z);
echo::error("Failed to open file: ", filename, " (error: ", errno, ")");
```

### Compile-Time Control

Zero overhead for disabled log levels:

```cpp
#define LOGLEVEL Debug  // Only Debug and above
#include <echo/echo.hpp>

echo::trace("This is compiled out!");  // No runtime cost
echo::debug("This appears");           // Included
```

Available levels: `Trace`, `Debug`, `Info`, `Warn`, `Error`, `Critical`, `Off`

### Runtime Control

Change levels on the fly:

```cpp
echo::set_level(echo::Level::Warn);  // Only warnings and errors
echo::info("Hidden");                // Filtered out
echo::error("Visible");              // Shown

auto level = echo::get_level();      // Query current level
```

### Timestamps

```cpp
#define ECHO_ENABLE_TIMESTAMP
#include <echo/echo.hpp>

echo::info("Message with timestamp");
// Output: [14:30:45][info] Message with timestamp
```

### Structured Logging

Key-value pairs for machine-readable logs:

```cpp
echo::info("User login: ", echo::kv("user", "alice", "age", 30, "role", "admin"));
// Output: [info] User login: user=alice age=30 role=admin

echo::warn("Connection failed: ", echo::kv("host", "localhost", "port", 8080, "retry", 3));
// Output: [warning] Connection failed: host=localhost port=8080 retry=3
```

### Custom Types

Log your own types by implementing `pretty()`, `print()`, or `to_string()`:

```cpp
struct Point {
    int x, y;
    
    std::string pretty() const {
        return "Point { x: " + std::to_string(x) + ", y: " + std::to_string(y) + " }";
    }
};

Point p{10, 20};
echo::info("Position: ", p);
// Output: [info] Position: Point { x: 10, y: 20 }
```

---

## 🎨 Visual Elements (`banner.hpp`)

Beautiful terminal UI elements with HEX color support!

```cpp
#include <echo/banner.hpp>  // Includes echo.hpp automatically
```

### Separators

Auto-sizing separators that adapt to terminal width:

```cpp
echo::separator();                    // Full-width line
echo::separator("Section 1");         // Centered text
echo::separator("IMPORTANT", '=');    // Custom character

// With colors!
echo::separator("Success", '-', "#00FF00");                    // Green
echo::separator("Warning", '=', {"#FF0000", "#FFFF00"});      // Red→Yellow gradient
```

**Output:**
```
────────────────────────────────────────────────────────────────────────────────
─────────────────────────────[ Section 1 ]──────────────────────────────────────
================================[ IMPORTANT ]===================================
```

### Boxes

Six beautiful box styles:

```cpp
echo::box("Hello World");                    // Single line
echo::box("Title", echo::BoxStyle::Double);  // Double line
echo::box("Info", echo::BoxStyle::Rounded);  // Rounded corners
echo::box("Alert", echo::BoxStyle::Heavy);   // Heavy/bold lines
echo::box("Note", echo::BoxStyle::Dashed);   // Dashed lines
echo::box("Plain", echo::BoxStyle::ASCII);   // ASCII compatible

// With colors!
echo::box("Success", echo::BoxStyle::Double, "#00FF00");
echo::box("Fire", echo::BoxStyle::Heavy, {"#FF0000", "#FF7F00", "#FFFF00"});
```

**Output:**
```
┌─────────────┐
│ Hello World │
└─────────────┘

╔═══════╗
║ Title ║
╚═══════╝

╭──────╮
│ Info │
╰──────╯
```

### Headers & Titles

```cpp
echo::header("Application Started");
echo::title("My Application");
echo::title("Section", '-');

// With colors!
echo::header("Welcome", "#00FFFF");
echo::title("Success", '=', {"#00FF00", "#FFFF00"});
```

### Banners

Large decorative banners for important messages:

```cpp
echo::banner("WELCOME");
echo::banner("SUCCESS", echo::BoxStyle::Double);
echo::banner("ERROR", echo::BoxStyle::Rounded);

// With colors!
echo::banner("READY", echo::BoxStyle::Heavy, "#00FF00");
```

**Output:**
```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                        ┃
┃              WELCOME                   ┃
┃                                        ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

---

## 📊 Progress Bars (`wait.hpp`)

Professional progress indicators with auto-sizing, themes, and advanced features!

```cpp
#include <echo/wait.hpp>  // Includes echo.hpp automatically
```

### Basic Progress Bar

```cpp
echo::progress_bar bar(100);  // Total steps
bar.set_prefix("Loading");
for (int i = 0; i <= 100; ++i) {
    bar.tick();  // Increment by 1
}
bar.finish();
```

**Output:**
```
Loading [████████████████████████████████████████████████████████] 100%
```

### 🎨 Bar Styles

Six beautiful visual styles:

```cpp
// Classic ASCII
bar.set_bar_style(echo::BarStyle::Classic);
// [===>                                                        ]

// Unicode Blocks
bar.set_bar_style(echo::BarStyle::Blocks);
// [███▓▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░]

// Smooth (sub-pixel precision!)
bar.set_bar_style(echo::BarStyle::Smooth);
// [████████▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░]

// Arrows
bar.set_bar_style(echo::BarStyle::Arrows);
// [→→→→⇒                                                       ]

// Dots
bar.set_bar_style(echo::BarStyle::Dots);
// [●●●●◉○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○○]

// Pure ASCII (compatible)
bar.set_bar_style(echo::BarStyle::ASCII);
// [###>........................................................]
```

### 🎨 Pre-configured Themes

11 beautiful themes combining style + colors:

```cpp
bar.set_theme(echo::BarTheme::fire());     // Red→Orange→Yellow gradient
bar.set_theme(echo::BarTheme::ocean());    // Blue gradient
bar.set_theme(echo::BarTheme::forest());   // Green gradient
bar.set_theme(echo::BarTheme::sunset());   // Orange→Gold gradient
bar.set_theme(echo::BarTheme::neon());     // Magenta→Cyan→Yellow
```

### 📏 Auto-Sizing (Full Terminal Width)

Progress bars automatically adapt to your terminal size!

```cpp
// No set_bar_width() call = auto-sizing to terminal width
echo::progress_bar bar(100);
bar.set_prefix("Download");
bar.set_bar_style(echo::BarStyle::Smooth);
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});

// Intelligently calculates available space after:
// - Prefix text
// - Percentage display
// - Time tracking
// - Speed display
// - Postfix text
```

**Output on 80-column terminal:**
```
Download [████████████████████▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░] 42%
```

**Output on 120-column terminal:**
```
Download [████████████████████████████████▌░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░] 42%
```

### 📦 Byte Formatting

Display file sizes with automatic unit conversion:

```cpp
const size_t total_bytes = 50 * 1024 * 1024;  // 50 MB
const size_t chunk_size = 512 * 1024;         // 512 KB chunks
const size_t total_chunks = total_bytes / chunk_size;

echo::progress_bar download(total_chunks);
download.set_prefix("Download");
download.set_show_bytes(true, chunk_size);
download.set_show_speed(true);
download.set_show_elapsed(true);
download.set_bar_style(echo::BarStyle::Smooth);
download.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});

for (size_t i = 0; i <= total_chunks; ++i) {
    download.set_progress(i);
}
download.finish();
```

**Output:**
```
Download [████████████████████████████████████] 25.0 MB / 50.0 MB [5s, 5.0 MB/s]
```

### ⏱️ Time & Speed Tracking

```cpp
bar.set_show_elapsed(true);     // Show elapsed time
bar.set_show_remaining(true);   // Show estimated remaining time
bar.set_show_speed(true);       // Show transfer speed
```

**Output:**
```
Processing [████████████████░░░░░░░░░░░░] 60% [1m30s < 1m0s, 1.2 MB/s]
```

### 🎨 Colors & Gradients

```cpp
// Single color
bar.set_color("#00FFFF");  // Cyan

// Gradient (smooth color transition)
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});  // Green→Yellow→Red
bar.set_gradient({"#FF00FF", "#00FFFF", "#FFFF00"});  // Rainbow
```

### 🔧 Custom Width

Override auto-sizing with a fixed width:

```cpp
bar.set_bar_width(40);  // Fixed 40-character bar
```

---

## 🌀 Spinners

15 animated spinner styles for indeterminate operations:

```cpp
echo::spinner spin(echo::spinner_style::aesthetic);
spin.set_message("Processing...");

while (processing) {
    spin.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(spin.get_interval_ms()));
}
spin.stop("✓ Done!");
```

**Available styles:**
- `line` - Classic: `- \ | /`
- `pipe` - Box drawing: `┤ ┘ ┴ └ ├ ┌ ┬ ┐`
- `simple_dots` - Simple: `. .. ...`
- `dots_scrolling` - Animated: `. .. ... .. .`
- `flip` - Flip: `_ - \` ' ´`
- `toggle` - Toggle: `= * -`
- `layer` - Layers: `- = ≡`
- `point` - Moving dot: `∙∙∙ ●∙∙ ∙●∙ ∙∙●`
- `dqpb` - Letters: `d q p b`
- `bouncing_bar` - Bar: `[====]`
- `bouncing_ball` - Ball: `( ● )`
- `aesthetic` - Progress-like: `▰▰▰▰▰▰▰`
- `binary` - Binary: `010010 001100`
- `grow_vertical` - Vertical: `▁ ▃ ▄ ▅ ▆ ▇`
- `grow_horizontal` - Horizontal: `▏ ▎ ▍ ▌ ▋ ▊ ▉`

**With colors:**
```cpp
spin.set_color("#FF00FF");  // Magenta
spin.set_gradient({"#FF0000", "#00FF00", "#0000FF"});  // RGB cycle
```

---

## 📝 Step Indicators

Track multi-step workflows:

```cpp
// Finite steps (known total)
echo::steps workflow({"Initialize", "Load", "Process", "Save"});
workflow.next();      // Step 1/4: Initialize
workflow.complete();  // ✓ Initialize - Complete
workflow.next();      // Step 2/4: Load
workflow.complete();  // ✓ Load - Complete

// Infinite steps (unknown total)
echo::steps infinite;
infinite.add_step("Connecting");
infinite.next();      // Step 1: Connecting
infinite.complete();  // ✓ Connecting - Complete

// Handle failures
workflow.next();
workflow.fail();  // ✗ Process - Failed

// With colors
workflow.set_color("#00FF00");  // Green checkmarks
```

---

## 🎯 Complete Example

```cpp
#include <echo/banner.hpp>
#include <echo/wait.hpp>

int main() {
    // Beautiful banner
    echo::banner("FILE PROCESSOR", echo::BoxStyle::Double, "#00FFFF");
    std::cout << "\n";
    
    // Multi-step workflow
    echo::steps workflow({"Initialize", "Scan Files", "Process", "Complete"});
    
    // Step 1: Initialize
    workflow.next();
    echo::info("Loading configuration...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    workflow.complete();
    
    // Step 2: Scan Files
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
    
    // Step 3: Process Files
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
    
    // Step 4: Complete
    workflow.next();
    workflow.complete();
    
    std::cout << "\n";
    echo::separator("SUCCESS", '=', "#00FF00");
    echo::info("All files processed successfully!");
    
    return 0;
}
```

---

## 🔧 Configuration

### Compile-Time Options

| Option | Description | Default |
|--------|-------------|---------|
| `LOGLEVEL` | Minimum log level (Trace\|Debug\|Info\|Warn\|Error\|Critical\|Off) | `Info` |
| `ECHO_ENABLE_TIMESTAMP` | Enable timestamps in HH:MM:SS format | Disabled |

### Build System Support

Echo works with CMake, xmake, and Zig:

```bash
# Auto-detect build system
make build

# Or specify explicitly
BUILD_SYSTEM=cmake make build
BUILD_SYSTEM=xmake make build
BUILD_SYSTEM=zig make build
```

---

## 🧪 Testing

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

---

## 📋 Requirements

- **C++20** or later
- **Standard library only** - no external dependencies
- **Unix/Linux/macOS** for terminal width detection (Windows support coming soon)

---

## 🎨 Color Support

Echo supports **24-bit true color** (16.7 million colors) using HEX codes:

```cpp
// Single colors
echo::separator("Title", '-', "#FF5733");
echo::box("Message", echo::BoxStyle::Double, "#00FF00");

// Gradients (smooth transitions)
echo::separator("Rainbow", '=', {"#FF0000", "#FF7F00", "#FFFF00", "#00FF00", "#0000FF", "#8B00FF"});
echo::box("Fire", echo::BoxStyle::Heavy, {"#FF0000", "#FF7F00", "#FFFF00"});

// Progress bars
bar.set_color("#00FFFF");
bar.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});  // Green→Yellow→Red

// Spinners
spin.set_color("#FF00FF");
spin.set_gradient({"#FF0000", "#00FF00", "#0000FF"});  // RGB cycle
```

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

See [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md) for credits and acknowledgments.

---

## 🌟 Star History

If you find Echo useful, please consider giving it a star! ⭐

---

<div align="center">

**Made with ❤️ by the Echo team**

[Documentation](https://github.com/robolibs/echo) • [Examples](examples/) • [Issues](https://github.com/robolibs/echo/issues)

</div>
