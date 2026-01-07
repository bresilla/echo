# Echo

**A modern C++20 header-only logging library with modular architecture, multiple sinks, and zero-overhead performance.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-yes-green.svg)](https://github.com/robolibs/echo)

## Development Status

See [TODO.md](./TODO.md) for the complete development plan and current progress.

## Overview

Echo is a high-performance, header-only logging library designed for modern C++20 applications. It provides a fluent interface for logging with compile-time optimization, multiple output sinks, and rich formatting capabilities.

**Key Design Principles:**
- **Zero overhead** - Compile-time log filtering eliminates runtime cost (literally 0ns!)
- **Modular architecture** - Enable only the sinks you need via compile-time flags
- **Performance first** - std::format integration, memory pooling, thread-local storage
- **Developer friendly** - Fluent API, category filtering, visual widgets

Echo is built for applications that demand both flexibility and performance, from embedded systems to high-throughput servers.

### Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                            ECHO LIBRARY                                  │
├──────────────┬──────────────┬──────────────┬──────────────┬─────────────┤
│     Core     │    Sinks     │  Formatters  │   Filters    │   Utils     │
│              │              │              │              │             │
│  ┌────────┐  │  ┌────────┐  │  ┌────────┐  │  ┌────────┐  │ ┌────────┐ │
│  │ Proxy  │  │  │Console │  │  │Pattern │  │  │Category│  │ │ Color  │ │
│  │ Level  │  │  │  File  │  │  │ Custom │  │  │  Level │  │ │ Pool   │ │
│  │ Mutex  │  │  │Syslog* │  │  │        │  │  │        │  │ │Terminal│ │
│  │  Once  │  │  │Network*│  │  │        │  │  │        │  │ │  Hash  │ │
│  └────────┘  │  │ Null*  │  │  └────────┘  │  └────────┘  │ └────────┘ │
│              │  └────────┘  │              │              │             │
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
Format Message (std::format/ostringstream) → Memory Pool
    ↓
Sink Registry → [ConsoleSink, FileSink*, SyslogSink*, NetworkSink*]
    ↓
Output (stdout/stderr, file, syslog, network)
```

## Installation

### Quick Start (CMake FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(
  echo
  GIT_REPOSITORY https://github.com/robolibs/echo.git
  GIT_TAG main
)
FetchContent_MakeAvailable(echo)

target_link_libraries(your_target PRIVATE echo::echo)
```

### Recommended: XMake

[XMake](https://xmake.io/) is a modern, fast, and cross-platform build system.

**Install XMake:**
```bash
curl -fsSL https://xmake.io/shget.text | bash
```

**Add to your xmake.lua:**
```lua
add_requires("echo")

target("your_target")
    set_kind("binary")
    add_packages("echo")
    add_files("src/*.cpp")
```

**Build:**
```bash
xmake
xmake run
```

### Complete Development Environment (Nix + Direnv + Devbox)

For the ultimate reproducible development environment:

**1. Install Nix (package manager from NixOS):**
```bash
# Determinate Nix Installer (recommended)
curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
```
[Nix](https://nixos.org/) - Reproducible, declarative package management

**2. Install direnv (automatic environment switching):**
```bash
sudo apt install direnv

# Add to your shell (~/.bashrc or ~/.zshrc):
eval "$(direnv hook bash)"  # or zsh
```
[direnv](https://direnv.net/) - Load environment variables based on directory

**3. Install Devbox (Nix-powered development environments):**
```bash
curl -fsSL https://get.jetpack.io/devbox | bash
```
[Devbox](https://www.jetpack.io/devbox/) - Portable, isolated dev environments

**4. Use the environment:**
```bash
cd echo
direnv allow  # Allow .envrc (one-time)
# Environment automatically loaded! All dependencies available.

xmake        # or cmake, make, etc.
```

## Usage

### Basic Usage

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

### Advanced Usage - Multiple Sinks

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
    file->set_rotation_policy(echo::RotationPolicy::Daily);  // Rotate daily
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

### Category-Based Filtering

```cpp
#include <echo/echo.hpp>

int main() {
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

    return 0;
}
```

### Custom Formatters

```cpp
#include <echo/echo.hpp>

int main() {
    // Set global formatter with pattern
    auto formatter = std::make_shared<echo::PatternFormatter>(
        "[{timestamp}] [{level}] {message}"
    );
    echo::set_global_formatter(formatter);

    echo::info("Formatted message");
    // Output: [14:30:45] [info] Formatted message

    // Custom formatter for specific sink
    auto file = std::make_shared<echo::FileSink>("app.log");
    file->set_formatter(std::make_shared<echo::PatternFormatter>(
        "{timestamp} | {level:5} | {message}"
    ));
    echo::add_sink(file);

    return 0;
}
```

## Features

- **Modular Sink Architecture** - Enable only what you need via compile-time flags
  ```cpp
  #define ECHO_ENABLE_FILE_SINK      // File logging with rotation
  #define ECHO_ENABLE_SYSLOG_SINK    // Syslog integration (Unix)
  #define ECHO_ENABLE_NETWORK_SINK   // TCP/UDP logging
  #define ECHO_ENABLE_NULL_SINK      // Null sink for testing
  ```

- **Zero-Overhead Compile-Time Filtering** - Filtered logs don't exist in binary (0ns)
  ```bash
  # Compile with log level - filtered logs are completely removed
  cmake -DLOGLEVEL=Error ..
  make build LOGLEVEL=Error
  ```

- **Time-Based Log Rotation** - Daily, hourly, interval, or combined size+time policies
  ```cpp
  file->set_rotation_policy(echo::RotationPolicy::Daily);
  file->set_rotation_interval(std::chrono::hours(6));
  file->set_rotation_policy(echo::RotationPolicy::SizeOrTime);
  ```

- **Category/Module Filtering** - Hierarchical categories with wildcard support
  ```cpp
  echo::set_category_level("app.*", echo::Level::Debug);
  echo::category("app.network.tcp").info("Connected");
  ```

- **High-Performance String Formatting** - std::format integration (4x faster than ostringstream)
  - Automatic fallback to ostringstream for C++17
  - Custom type support via `pretty()`, `print()`, `to_string()`

- **Memory Pool** - Thread-local 4KB buffer for zero-allocation logging
  - Eliminates heap allocations for typical messages
  - Automatic fallback to heap for large messages
  - No contention between threads

- **Thread-Safe** - Concurrent logging without corruption
  - Lock-free fast paths where possible
  - Mutex-protected sink registry
  - Thread-local storage for performance

- **Fluent Interface** - Chain colors and modifiers
  ```cpp
  echo::info("Message").red().bold().italic().underline();
  echo::warn("Custom").hex("#FF00FF").bold();
  ```

- **Visual Widgets** - Progress bars, spinners, banners, boxes
  ```cpp
  echo::progress_bar bar(100);
  bar.set_theme(echo::BarTheme::ocean());
  bar.set_show_speed(true);
  ```

- **Sanitizer Testing** - ASAN, TSAN, UBSAN support via Makefile
  ```bash
  make test-asan    # AddressSanitizer
  make test-tsan    # ThreadSanitizer
  make test-ubsan   # UndefinedBehaviorSanitizer
  ```

- **Fuzzing Support** - libFuzzer harnesses for edge case discovery
  ```bash
  cd fuzz
  clang++ -fsanitize=fuzzer,address -I../include fuzz_log.cpp -o fuzz_log
  ./fuzz_log -max_total_time=60
  ```

- **Comprehensive Benchmarks** - Performance testing for all components
  ```bash
  # Build benchmarks
  cd benchmark
  g++ -O3 -I../include bench_logging.cpp -o bench_logging
  ./bench_logging
  ```

## Performance

Echo is designed for **invisible overhead** in production code.

### Zero Overhead with Compile-Time Filtering

When you set `-DLOGLEVEL=Error`, filtered log statements are **completely removed** from the binary:

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
| String formatting (std::format) | ~500 ns | 4x faster than ostringstream |
| String formatting (ostringstream) | ~2 μs | Fallback for C++17 |
| Memory pool allocation | ~0 ns | Thread-local, zero allocations |
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

**✅ DO: Enable only needed sinks**
```cpp
// Only enable what you need
#define ECHO_ENABLE_FILE_SINK
#include <echo/echo.hpp>
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

See [PERFORMANCE.md](misc/PERFORMANCE.md) for detailed benchmarks and optimization guide.

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
./build/linux/x86_64/release/color_demo
./build/linux/x86_64/release/advanced_progress_demo
```

## Documentation

Generate API documentation with Doxygen:

```bash
make docs TYPE=doxygen
# Open docs/doxygen/html/index.html in your browser
```

## Requirements

- C++20 or later
- Standard library only (no external dependencies)
- Unix/Linux/macOS for terminal width detection
- Optional: Clang for fuzzing tests

## License

MIT License - see [LICENSE](./LICENSE) for details.

## Acknowledgments

Made possible thanks to [these amazing projects](misc/ACKNOWLEDGMENTS.md).
