/**
 * @file fuzz_log.cpp
 * @brief Fuzz test for log message content
 *
 * Tests arbitrary strings as log messages to find crashes or undefined behavior.
 *
 * Compile with:
 *   clang++ -fsanitize=fuzzer,address -g -O1 -I../include fuzz_log.cpp -o fuzz_log
 *
 * Run:
 *   ./fuzz_log -max_total_time=60
 */

#include <echo/echo.hpp>

#include <cstdint>
#include <cstdlib>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Clear sinks to avoid I/O overhead during fuzzing
    static bool initialized = false;
    if (!initialized) {
        echo::clear_sinks();
        initialized = true;
    }

    // Convert fuzzer input to string
    std::string input_msg(reinterpret_cast<const char *>(data), size);

    // Test all log levels with arbitrary input
    echo::trace(input_msg);
    echo::debug(input_msg);
    echo::info(input_msg);
    echo::warn(input_msg);
    echo::error(input_msg);
    echo::critical(input_msg);

    // Test simple echo (use braces to avoid variable name conflict)
    {
        auto result = echo(input_msg);
        (void)result; // Suppress unused variable warning
    }

    // Test with multiple arguments
    if (size > 0) {
        echo::info("Prefix: ", input_msg, " suffix");
    }

    return 0;
}
