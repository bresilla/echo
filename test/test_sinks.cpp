/**
 * @file test_sinks.cpp
 * @brief Test sink system integration
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>

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

TEST_CASE("Sink system") {
    SUBCASE("Add custom sink") {
        // Clear all sinks first
        echo::clear_sinks();
        REQUIRE(echo::sink_count() == 0);

        // Add a test sink
        auto test_sink = std::make_shared<TestSink>();
        echo::add_sink(test_sink);
        REQUIRE(echo::sink_count() == 1);

        // Log some messages
        echo::info("Message 1");
        echo::warn("Message 2");
        echo::error("Message 3");

        // Check that messages were captured
        REQUIRE(test_sink->message_count() == 3);

        // Verify message content contains expected text
        std::string msg1 = test_sink->get_message(0);
        std::string msg2 = test_sink->get_message(1);
        std::string msg3 = test_sink->get_message(2);

        CHECK(msg1.find("Message 1") != std::string::npos);
        CHECK(msg2.find("Message 2") != std::string::npos);
        CHECK(msg3.find("Message 3") != std::string::npos);
    }

    SUBCASE("Multiple sinks") {
        // Clear all sinks
        echo::clear_sinks();

        // Add two test sinks
        auto sink1 = std::make_shared<TestSink>();
        auto sink2 = std::make_shared<TestSink>();

        echo::add_sink(sink1);
        echo::add_sink(sink2);
        REQUIRE(echo::sink_count() == 2);

        // Log a message
        echo::info("Test message");

        // Both sinks should receive the message
        CHECK(sink1->message_count() == 1);
        CHECK(sink2->message_count() == 1);
    }

    SUBCASE("Sink level filtering") {
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
        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Remove sink") {
        // Clear all sinks
        echo::clear_sinks();

        // Add a test sink
        auto sink = std::make_shared<TestSink>();
        echo::add_sink(sink);
        REQUIRE(echo::sink_count() == 1);

        // Remove the sink
        echo::remove_sink(sink);
        REQUIRE(echo::sink_count() == 0);

        // Log a message (should not be captured)
        echo::info("Test message");
        CHECK(sink->message_count() == 0);
    }

    SUBCASE("print_proxy with sinks") {
        // Clear all sinks
        echo::clear_sinks();

        // Add a test sink
        auto sink = std::make_shared<TestSink>();
        echo::add_sink(sink);

        // Use print proxy (no log level)
        echo::print("Simple print message");

        // Should be captured by the sink
        REQUIRE(sink->message_count() == 1);

        std::string msg = sink->get_message(0);
        CHECK(msg.find("Simple print message") != std::string::npos);
    }
}
