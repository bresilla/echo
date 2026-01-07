/**
 * @file test_null_sink.cpp
 * @brief Test null sink functionality
 */

#define ECHO_ENABLE_NULL_SINK
#include <echo/echo.hpp>

#include <cassert>
#include <chrono>
#include <iostream>

void test_null_sink_basic() {
    std::cout << "Testing NullSink basic functionality...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a null sink
    auto null_sink = std::make_shared<echo::NullSink>();
    echo::add_sink(null_sink);
    assert(echo::sink_count() == 1 && "Should have 1 sink");

    // Log messages (should be discarded silently)
    echo::trace("Trace message");
    echo::debug("Debug message");
    echo::info("Info message");
    echo::warn("Warning message");
    echo::error("Error message");
    echo::critical("Critical message");

    // Flush (should do nothing)
    null_sink->flush();

    std::cout << "✓ NullSink basic functionality works\n";
}

void test_null_sink_level_filtering() {
    std::cout << "Testing NullSink level filtering...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a null sink with Info level
    auto null_sink = std::make_shared<echo::NullSink>();
    null_sink->set_level(echo::Level::Info);
    echo::add_sink(null_sink);

    // Log messages at different levels
    // Even though they're discarded, level filtering should still work
    echo::trace("Trace message");  // Should be filtered
    echo::debug("Debug message");  // Should be filtered
    echo::info("Info message");    // Should pass (but discarded)
    echo::warn("Warning message"); // Should pass (but discarded)
    echo::error("Error message");  // Should pass (but discarded)

    // Verify level setting
    assert(null_sink->get_level() == echo::Level::Info && "Level should be Info");
    assert(null_sink->should_log(echo::Level::Trace) == false && "Trace should be filtered");
    assert(null_sink->should_log(echo::Level::Debug) == false && "Debug should be filtered");
    assert(null_sink->should_log(echo::Level::Info) == true && "Info should pass");
    assert(null_sink->should_log(echo::Level::Warn) == true && "Warn should pass");
    assert(null_sink->should_log(echo::Level::Error) == true && "Error should pass");

    std::cout << "✓ NullSink level filtering works\n";
}

void test_null_sink_performance() {
    std::cout << "Testing NullSink performance (benchmarking)...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a null sink
    auto null_sink = std::make_shared<echo::NullSink>();
    echo::add_sink(null_sink);

    // Benchmark: Log many messages
    const int iterations = 100000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        echo::info("Test message ", i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "  Logged " << iterations << " messages in " << duration.count() << " ms\n";
    std::cout << "  Average: " << (duration.count() * 1000.0 / iterations) << " µs per message\n";

    // Performance should be reasonable (no I/O overhead)
    // This is just a sanity check, not a strict requirement
    assert(duration.count() < 5000 && "Should complete in reasonable time");

    std::cout << "✓ NullSink performance is acceptable\n";
}

void test_null_sink_with_console() {
    std::cout << "Testing NullSink alongside ConsoleSink...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add both null sink and console sink
    auto null_sink = std::make_shared<echo::NullSink>();
    auto console_sink = std::make_shared<echo::ConsoleSink>();

    echo::add_sink(null_sink);
    echo::add_sink(console_sink);
    assert(echo::sink_count() == 2 && "Should have 2 sinks");

    // Log a message (should go to console but not null)
    std::cout << "  The following message should appear on console:\n  ";
    echo::info("This message goes to both sinks");

    std::cout << "✓ NullSink works alongside other sinks\n";
}

void test_null_sink_formatter() {
    std::cout << "Testing NullSink with custom formatter...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a null sink with a custom formatter
    auto null_sink = std::make_shared<echo::NullSink>();
    auto formatter = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
    null_sink->set_formatter(formatter);
    echo::add_sink(null_sink);

    // Verify formatter is set
    assert(null_sink->get_formatter() != nullptr && "Formatter should be set");
    assert(null_sink->get_formatter() == formatter && "Formatter should match");

    // Log messages (formatter won't be used since messages are discarded)
    echo::info("Message with formatter");
    echo::warn("Warning with formatter");

    std::cout << "✓ NullSink accepts custom formatters\n";
}

int main() {
    std::cout << "=== Echo NullSink Tests ===\n\n";

    try {
        test_null_sink_basic();
        test_null_sink_level_filtering();
        test_null_sink_performance();
        test_null_sink_with_console();
        test_null_sink_formatter();

        std::cout << "\n=== All NullSink tests passed! ===\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
