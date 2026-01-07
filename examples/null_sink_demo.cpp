/**
 * @file null_sink_demo.cpp
 * @brief Demonstrates NullSink usage for benchmarking and testing
 */

#define ECHO_ENABLE_NULL_SINK
#include <echo/echo.hpp>

#include <chrono>
#include <iostream>

void benchmark_with_null_sink() {
    std::cout << "=== Benchmarking with NullSink ===\n\n";

    // Clear default sinks
    echo::clear_sinks();

    // Add null sink for benchmarking
    auto null_sink = std::make_shared<echo::NullSink>();
    echo::add_sink(null_sink);

    std::cout << "Benchmarking logging performance (no I/O overhead)...\n";

    const int iterations = 1000000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        echo::info("Benchmark message ", i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nResults:\n";
    std::cout << "  Total messages: " << iterations << "\n";
    std::cout << "  Total time: " << duration.count() << " ms\n";
    std::cout << "  Average: " << (duration.count() * 1000.0 / iterations) << " µs per message\n";
    std::cout << "  Throughput: " << (iterations * 1000.0 / duration.count()) << " messages/second\n";
}

void compare_with_console_sink() {
    std::cout << "\n=== Comparing NullSink vs ConsoleSink ===\n\n";

    const int iterations = 10000;

    // Benchmark with ConsoleSink
    echo::clear_sinks();
    auto console_sink = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console_sink);

    std::cout << "Benchmarking with ConsoleSink (with I/O)...\n";
    auto start_console = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        echo::info("Console message ", i);
    }

    auto end_console = std::chrono::high_resolution_clock::now();
    auto duration_console = std::chrono::duration_cast<std::chrono::milliseconds>(end_console - start_console);

    // Benchmark with NullSink
    echo::clear_sinks();
    auto null_sink = std::make_shared<echo::NullSink>();
    echo::add_sink(null_sink);

    std::cout << "\nBenchmarking with NullSink (no I/O)...\n";
    auto start_null = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        echo::info("Null message ", i);
    }

    auto end_null = std::chrono::high_resolution_clock::now();
    auto duration_null = std::chrono::duration_cast<std::chrono::milliseconds>(end_null - start_null);

    std::cout << "\nComparison:\n";
    std::cout << "  ConsoleSink: " << duration_console.count() << " ms\n";
    std::cout << "  NullSink:    " << duration_null.count() << " ms\n";
    std::cout << "  Speedup:     " << (duration_console.count() / (double)duration_null.count()) << "x\n";
    std::cout << "  I/O overhead: " << (duration_console.count() - duration_null.count()) << " ms\n";
}

void demonstrate_testing_use_case() {
    std::cout << "\n=== Using NullSink for Testing ===\n\n";

    // Simulate a function that logs internally
    auto process_data = [](int count) {
        for (int i = 0; i < count; ++i) {
            echo::debug("Processing item ", i);
            if (i % 100 == 0) {
                echo::info("Progress: ", i, "/", count);
            }
        }
        echo::info("Processing complete!");
    };

    // Test the function without producing output
    std::cout << "Testing process_data() with NullSink (no output)...\n";
    echo::clear_sinks();
    auto null_sink = std::make_shared<echo::NullSink>();
    echo::add_sink(null_sink);

    process_data(1000);

    std::cout << "✓ Function executed without producing log output\n";
}

void demonstrate_selective_output() {
    std::cout << "\n=== Selective Output with Multiple Sinks ===\n\n";

    echo::clear_sinks();

    // Add console sink for important messages only
    auto console_sink = std::make_shared<echo::ConsoleSink>();
    console_sink->set_level(echo::Level::Warn);
    echo::add_sink(console_sink);

    // Add null sink for everything (to avoid losing debug info)
    auto null_sink = std::make_shared<echo::NullSink>();
    null_sink->set_level(echo::Level::Trace);
    echo::add_sink(null_sink);

    std::cout << "Logging with selective output (only warnings+ to console):\n\n";

    echo::trace("Trace message (discarded)");
    echo::debug("Debug message (discarded)");
    echo::info("Info message (discarded)");
    echo::warn("Warning message (shown)");
    echo::error("Error message (shown)");

    std::cout << "\n✓ Only warnings and errors were shown\n";
}

int main() {
    std::cout << "=== Echo NullSink Demo ===\n\n";

    benchmark_with_null_sink();
    compare_with_console_sink();
    demonstrate_testing_use_case();
    demonstrate_selective_output();

    // Restore default console sink
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::ConsoleSink>());

    std::cout << "\n=== Demo Complete ===\n";
    return 0;
}
