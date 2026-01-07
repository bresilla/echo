/**
 * @file test_sinks.cpp
 * @brief Test sink system integration
 */

#include <echo/echo.hpp>

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

// Custom test sink that captures messages
class TestSink : public echo::Sink {
  private:
    std::vector<std::string> messages_;
    mutable std::mutex mutex_;

  public:
    TestSink() = default;

    void write(echo::Level level, const std::string &message) override {
        if (!should_log(level)) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.push_back(message);
    }

    void flush() override {
        // No-op for test sink
    }

    [[nodiscard]] size_t message_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_.size();
    }

    [[nodiscard]] std::string get_message(size_t index) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (index < messages_.size()) {
            return messages_[index];
        }
        return "";
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.clear();
    }
};

void test_default_console_sink() {
    std::cout << "Testing default ConsoleSink...\n";

    // By default, there should be 1 sink (ConsoleSink)
    size_t initial_count = echo::sink_count();
    assert(initial_count == 1 && "Should have default ConsoleSink");

    // Log a message (should go to console)
    echo::info("Test message to console");

    std::cout << "✓ Default ConsoleSink works\n";
}

void test_add_custom_sink() {
    std::cout << "Testing custom sink addition...\n";

    // Clear all sinks first
    echo::clear_sinks();
    assert(echo::sink_count() == 0 && "Should have no sinks after clear");

    // Add a test sink
    auto test_sink = std::make_shared<TestSink>();
    echo::add_sink(test_sink);
    assert(echo::sink_count() == 1 && "Should have 1 sink after add");

    // Log some messages
    echo::info("Message 1");
    echo::warn("Message 2");
    echo::error("Message 3");

    // Check that messages were captured
    assert(test_sink->message_count() == 3 && "Should have captured 3 messages");

    // Verify message content contains expected text
    std::string msg1 = test_sink->get_message(0);
    std::string msg2 = test_sink->get_message(1);
    std::string msg3 = test_sink->get_message(2);

    assert(msg1.find("Message 1") != std::string::npos && "Message 1 should be captured");
    assert(msg2.find("Message 2") != std::string::npos && "Message 2 should be captured");
    assert(msg3.find("Message 3") != std::string::npos && "Message 3 should be captured");

    std::cout << "✓ Custom sink addition works\n";
}

void test_multiple_sinks() {
    std::cout << "Testing multiple sinks...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add two test sinks
    auto sink1 = std::make_shared<TestSink>();
    auto sink2 = std::make_shared<TestSink>();

    echo::add_sink(sink1);
    echo::add_sink(sink2);
    assert(echo::sink_count() == 2 && "Should have 2 sinks");

    // Log a message
    echo::info("Test message");

    // Both sinks should receive the message
    assert(sink1->message_count() == 1 && "Sink 1 should have 1 message");
    assert(sink2->message_count() == 1 && "Sink 2 should have 1 message");

    std::cout << "✓ Multiple sinks work\n";
}

void test_sink_level_filtering() {
    std::cout << "Testing sink level filtering...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a test sink with Info level
    auto sink = std::make_shared<TestSink>();
    sink->set_level(echo::Level::Info);
    echo::add_sink(sink);

    // Log messages at different levels
    echo::trace("Trace message"); // Should be filtered out
    echo::debug("Debug message"); // Should be filtered out
    echo::info("Info message");   // Should pass
    echo::warn("Warn message");   // Should pass
    echo::error("Error message"); // Should pass

    // Only Info, Warn, and Error should be captured
    assert(sink->message_count() == 3 && "Should have captured 3 messages (Info, Warn, Error)");

    std::cout << "✓ Sink level filtering works\n";
}

void test_remove_sink() {
    std::cout << "Testing sink removal...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a test sink
    auto sink = std::make_shared<TestSink>();
    echo::add_sink(sink);
    assert(echo::sink_count() == 1 && "Should have 1 sink");

    // Remove the sink
    echo::remove_sink(sink);
    assert(echo::sink_count() == 0 && "Should have 0 sinks after removal");

    // Log a message (should not be captured)
    echo::info("Test message");
    assert(sink->message_count() == 0 && "Removed sink should not receive messages");

    std::cout << "✓ Sink removal works\n";
}

void test_print_proxy_with_sinks() {
    std::cout << "Testing print_proxy with sinks...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Add a test sink
    auto sink = std::make_shared<TestSink>();
    echo::add_sink(sink);

    // Use print proxy (no log level)
    echo("Simple print message");

    // Should be captured by the sink
    assert(sink->message_count() == 1 && "Print message should be captured");

    std::string msg = sink->get_message(0);
    assert(msg.find("Simple print message") != std::string::npos && "Print message should contain text");

    std::cout << "✓ print_proxy with sinks works\n";
}

int main() {
    std::cout << "=== Echo Sink System Tests ===\n\n";

    try {
        test_default_console_sink();
        test_add_custom_sink();
        test_multiple_sinks();
        test_sink_level_filtering();
        test_remove_sink();
        test_print_proxy_with_sinks();

        std::cout << "\n=== All sink tests passed! ===\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
