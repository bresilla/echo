# Echo

**Echo** is a modern, opinionated C++20+ header-only logging library. Simple, concise, and easy to use. Not pretending to be the fastest, just pretending to be simple.

## Features

- ✨ **Header-only** - Just include and use
- 🎨 **Colored output** - Different colors for each log level
- ⚡ **Compile-time log levels** - Zero overhead for disabled levels
- 🔧 **Runtime log level control** - Change levels on the fly
- ⏰ **Optional timestamps** - HH:MM:SS format
- 🧵 **Thread-safe** - Safe concurrent logging
- 📊 **Structured logging** - Key-value pairs support
- 🎯 **Custom types** - Log your own types easily

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

Echo is header-only. Just copy `include/echo/echo.hpp` to your project or add it to your include path.

### CMake

```cmake
# Add as subdirectory
add_subdirectory(echo)
target_link_libraries(your_target echo::echo)

# Or use FetchContent
include(FetchContent)
FetchContent_Declare(
    echo
    GIT_REPOSITORY https://github.com/robolibs/echo.git
    GIT_TAG main
)
FetchContent_MakeAvailable(echo)
target_link_libraries(your_target echo::echo)
```

## Usage

### Basic Logging

Echo provides six log levels:

```cpp
echo::trace("Detailed trace information");
echo::debug("Debug information");
echo::info("General information");
echo::warn("Warning message");
echo::error("Error occurred");
echo::critical("Critical failure!");
```

Output:
```
[trace] Detailed trace information
[debug] Debug information
[info] General information
[warning] Warning message
[error] Error occurred
[critical] Critical failure!
```

### Multiple Arguments

Log multiple values in one call:

```cpp
echo::info("User: ", username, " logged in at ", timestamp);
echo::debug("Position: x=", x, " y=", y, " z=", z);
```

### Log Level Control

#### Compile-Time (Recommended)

Set the minimum log level at compile time for zero overhead:

```cpp
// In your code before including
#define LOGLEVEL Debug
#include <echo/echo.hpp>

// Or via build system
// -DLOGLEVEL=Debug
```

Available levels: `Trace`, `Debug`, `Info`, `Warn`, `Error`, `Critical`, `Off`

#### Runtime

Change log level during execution:

```cpp
echo::set_level(echo::Level::Debug);  // Only Debug and above
echo::info("This appears");
echo::trace("This is filtered");

auto level = echo::get_level();  // Query current level
```

### Timestamps

Enable timestamps with a compile-time macro:

```cpp
#define ECHO_ENABLE_TIMESTAMP
#include <echo/echo.hpp>

echo::info("Message with timestamp");
// Output: [14:30:45][info] Message with timestamp
```

### Custom Types

Log your own types by implementing one of these methods:

```cpp
struct Point {
    int x, y;
    
    // Option 1: pretty() - preferred
    std::string pretty() const {
        return "Point { x: " + std::to_string(x) + ", y: " + std::to_string(y) + " }";
    }
    
    // Option 2: print()
    std::string print() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
    
    // Option 3: to_string()
    std::string to_string() const {
        return std::to_string(x) + "," + std::to_string(y);
    }
};

Point p{10, 20};
echo::info("Position: ", p);
```

Priority: `pretty()` > `print()` > `to_string()` > `operator<<`

### Structured Logging

Use key-value pairs for structured logs:

```cpp
echo::info("User login: ", echo::kv("user", "alice", "age", 30, "role", "admin"));
// Output: [info] User login: user=alice age=30 role=admin

echo::warn("Connection failed: ", echo::kv("host", "localhost", "port", 8080, "retry", 3));
// Output: [warning] Connection failed: host=localhost port=8080 retry=3
```

### Thread Safety

Echo is thread-safe by default. Multiple threads can log concurrently without message corruption:

```cpp
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([i]() {
        echo::info("Thread ", i, " logging");
    });
}
for (auto& t : threads) t.join();
```

## Building and Testing

### Build

```bash
# Using make (auto-detects build system)
make build

# Or specify build system
BUILD_SYSTEM=cmake make build
BUILD_SYSTEM=xmake make build
BUILD_SYSTEM=zig make build
```

### Run Tests

```bash
make test

# Run specific test
make test TEST=test_basic
```

### Build Examples

```bash
make build
./build/main  # or appropriate path for your build system
```

## Configuration Options

| Option | Description | Default |
|--------|-------------|---------|
| `LOGLEVEL` | Compile-time log level (Trace\|Debug\|Info\|Warn\|Error\|Critical\|Off) | `Info` |
| `ECHO_ENABLE_TIMESTAMP` | Enable timestamps in HH:MM:SS format | Disabled |

## Requirements

- C++20 or later
- Standard library only (no external dependencies)

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Acknowledgments

See [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md) for credits and acknowledgments.
