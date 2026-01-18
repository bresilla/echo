/**
 * @file test_formatters.cpp
 * @brief Test formatter system
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <iostream>
#include <sstream>
#include <vector>

// Custom test sink that captures formatted messages
class FormatterTestSink : public echo::Sink {
  private:
    std::vector<std::string> messages_;
    mutable std::mutex mutex_;

  public:
    FormatterTestSink() = default;

    void write(echo::Level level, const std::string &message) override {
        if (!should_log(level)) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);

        // If we have a custom formatter, use it
        if (formatter_) {
            echo::LogRecord record;
            record.level = level;
            record.message = message;
            record.timestamp = "2026-01-07 12:34:56";
            record.file = "test.cpp";
            record.line = 42;
            record.function = "test_function";
            record.thread_id = 12345;

            std::string formatted = formatter_->format(record);
            messages_.push_back(formatted);
        } else {
            // No formatter, just store the raw message
            messages_.push_back(message);
        }
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

TEST_CASE("Formatter system") {
    SUBCASE("DefaultFormatter") {
        auto formatter = std::make_shared<echo::DefaultFormatter>();

        echo::LogRecord record;
        record.level = echo::Level::Info;
        record.message = "Test message";
        record.timestamp = "2026-01-07 12:34:56";

        std::string formatted = formatter->format(record);

        // Should contain timestamp, level, and message
        CHECK(formatted.find("2026-01-07 12:34:56") != std::string::npos);
        CHECK(formatted.find("info") != std::string::npos);
        CHECK(formatted.find("Test message") != std::string::npos);
    }

    SUBCASE("DefaultFormatter with options") {
        // Test without timestamp
        auto formatter1 = std::make_shared<echo::DefaultFormatter>(false, true);
        echo::LogRecord record;
        record.level = echo::Level::Warn;
        record.message = "Warning message";
        record.timestamp = "2026-01-07 12:34:56";

        std::string formatted1 = formatter1->format(record);
        CHECK(formatted1.find("2026-01-07") == std::string::npos);
        CHECK(formatted1.find("warning") != std::string::npos);

        // Test without level
        auto formatter2 = std::make_shared<echo::DefaultFormatter>(true, false);
        std::string formatted2 = formatter2->format(record);
        CHECK(formatted2.find("2026-01-07 12:34:56") != std::string::npos);
        CHECK(formatted2.find("warning") == std::string::npos);

        // Test without both
        auto formatter3 = std::make_shared<echo::DefaultFormatter>(false, false);
        std::string formatted3 = formatter3->format(record);
        CHECK(formatted3.find("2026-01-07") == std::string::npos);
        CHECK(formatted3.find("warning") == std::string::npos);
        CHECK(formatted3.find("Warning message") != std::string::npos);
    }

    SUBCASE("PatternFormatter") {
        auto formatter = std::make_shared<echo::PatternFormatter>("[{time}][{level}] {msg}");

        echo::LogRecord record;
        record.level = echo::Level::Error;
        record.message = "Error occurred";
        record.timestamp = "2026-01-07 12:34:56";

        std::string formatted = formatter->format(record);

        // Should match the pattern
        CHECK(formatted.find("[2026-01-07 12:34:56]") != std::string::npos);
        CHECK(formatted.find("[error]") != std::string::npos);
        CHECK(formatted.find("Error occurred") != std::string::npos);
    }

    SUBCASE("PatternFormatter custom patterns") {
        echo::LogRecord record;
        record.level = echo::Level::Debug;
        record.message = "Debug info";
        record.timestamp = "2026-01-07 12:34:56";
        record.file = "test.cpp";
        record.line = 42;
        record.function = "test_function";
        record.thread_id = 12345;

        // Test different patterns
        auto formatter1 = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
        std::string formatted1 = formatter1->format(record);
        CHECK(formatted1 == "debug: Debug info");

        auto formatter2 = std::make_shared<echo::PatternFormatter>("[{file}:{line}] {msg}");
        std::string formatted2 = formatter2->format(record);
        CHECK(formatted2.find("[test.cpp:42]") != std::string::npos);

        auto formatter3 = std::make_shared<echo::PatternFormatter>("{func} - {msg}");
        std::string formatted3 = formatter3->format(record);
        CHECK(formatted3.find("test_function - Debug info") != std::string::npos);

        auto formatter4 = std::make_shared<echo::PatternFormatter>("[{thread}] {msg}");
        std::string formatted4 = formatter4->format(record);
        CHECK(formatted4.find("[12345]") != std::string::npos);
    }

    SUBCASE("CustomFormatter") {
        // Create a custom formatter with a lambda
        auto formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
            return ">>> " + std::string(echo::detail::level_name(rec.level)) + ": " + rec.message + " <<<";
        });

        echo::LogRecord record;
        record.level = echo::Level::Info;
        record.message = "Custom formatted message";

        std::string formatted = formatter->format(record);

        CHECK(formatted == ">>> info: Custom formatted message <<<");
    }

    SUBCASE("CustomFormatter with JSON-like output") {
        auto formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
            std::ostringstream oss;
            oss << "{\"level\":\"" << echo::detail::level_name(rec.level) << "\","
                << "\"message\":\"" << rec.message << "\"";
            if (!rec.timestamp.empty()) {
                oss << ",\"timestamp\":\"" << rec.timestamp << "\"";
            }
            oss << "}";
            return oss.str();
        });

        echo::LogRecord record;
        record.level = echo::Level::Warn;
        record.message = "JSON warning";
        record.timestamp = "2026-01-07 12:34:56";

        std::string formatted = formatter->format(record);

        CHECK(formatted.find("\"level\":\"warning\"") != std::string::npos);
        CHECK(formatted.find("\"message\":\"JSON warning\"") != std::string::npos);
        CHECK(formatted.find("\"timestamp\":\"2026-01-07 12:34:56\"") != std::string::npos);
    }

    SUBCASE("Formatter integration with sink") {
        // Clear all sinks
        echo::clear_sinks();

        // Create a test sink (without custom formatter - library formats messages)
        auto sink = std::make_shared<FormatterTestSink>();
        echo::add_sink(sink);

        // Log a message
        echo::info("Test message with formatter");

        // Check that the message was captured and contains the content
        REQUIRE(sink->message_count() == 1);
        std::string msg = sink->get_message(0);
        CHECK(msg.find("Test message with formatter") != std::string::npos);
        CHECK(msg.find("info") != std::string::npos);
    }

    SUBCASE("Multiple sinks") {
        // Clear all sinks
        echo::clear_sinks();

        // Create two sinks (library formats messages before passing to sinks)
        auto sink1 = std::make_shared<FormatterTestSink>();
        auto sink2 = std::make_shared<FormatterTestSink>();

        echo::add_sink(sink1);
        echo::add_sink(sink2);

        // Log a message
        echo::warn("Multi-sink test");

        // Check that both sinks received the message
        REQUIRE(sink1->message_count() == 1);
        REQUIRE(sink2->message_count() == 1);

        std::string msg1 = sink1->get_message(0);
        std::string msg2 = sink2->get_message(0);

        CHECK(msg1.find("Multi-sink test") != std::string::npos);
        CHECK(msg2.find("Multi-sink test") != std::string::npos);
    }

    SUBCASE("Formatter cloning") {
        // Test DefaultFormatter clone
        auto default_formatter = std::make_shared<echo::DefaultFormatter>(true, false);
        auto default_clone = default_formatter->clone();

        echo::LogRecord record;
        record.level = echo::Level::Info;
        record.message = "Clone test";
        record.timestamp = "2026-01-07 12:34:56";

        std::string original = default_formatter->format(record);
        std::string cloned = default_clone->format(record);
        CHECK(original == cloned);

        // Test PatternFormatter clone
        auto pattern_formatter = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
        auto pattern_clone = pattern_formatter->clone();

        std::string pattern_original = pattern_formatter->format(record);
        std::string pattern_cloned = pattern_clone->format(record);
        CHECK(pattern_original == pattern_cloned);

        // Test CustomFormatter clone
        auto custom_formatter =
            std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) { return "CUSTOM: " + rec.message; });
        auto custom_clone = custom_formatter->clone();

        std::string custom_original = custom_formatter->format(record);
        std::string custom_cloned = custom_clone->format(record);
        CHECK(custom_original == custom_cloned);
    }

    SUBCASE("Messages reach multiple sinks") {
        // Clear all sinks
        echo::clear_sinks();

        // Create two test sinks
        auto sink1 = std::make_shared<FormatterTestSink>();
        auto sink2 = std::make_shared<FormatterTestSink>();

        echo::add_sink(sink1);
        echo::add_sink(sink2);

        // Log messages at different levels
        echo::info("Info message");
        echo::warn("Warning message");
        echo::error("Error message");

        // Both sinks should receive all messages
        CHECK(sink1->message_count() == 3);
        CHECK(sink2->message_count() == 3);

        // Verify content is present
        CHECK(sink1->get_message(0).find("Info message") != std::string::npos);
        CHECK(sink1->get_message(1).find("Warning message") != std::string::npos);
        CHECK(sink1->get_message(2).find("Error message") != std::string::npos);
    }
}
