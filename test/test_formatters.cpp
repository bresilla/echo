/**
 * @file test_formatters.cpp
 * @brief Test formatter system
 */

#include <echo/echo.hpp>

#include <cassert>
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

void test_default_formatter() {
    std::cout << "Testing DefaultFormatter...\n";

    auto formatter = std::make_shared<echo::DefaultFormatter>();

    echo::LogRecord record;
    record.level = echo::Level::Info;
    record.message = "Test message";
    record.timestamp = "2026-01-07 12:34:56";

    std::string formatted = formatter->format(record);

    // Should contain timestamp, level, and message
    assert(formatted.find("2026-01-07 12:34:56") != std::string::npos && "Should contain timestamp");
    assert(formatted.find("INFO") != std::string::npos && "Should contain level");
    assert(formatted.find("Test message") != std::string::npos && "Should contain message");

    std::cout << "  Formatted: " << formatted << "\n";
    std::cout << "✓ DefaultFormatter works\n";
}

void test_default_formatter_options() {
    std::cout << "Testing DefaultFormatter with options...\n";

    // Test without timestamp
    auto formatter1 = std::make_shared<echo::DefaultFormatter>(false, true);
    echo::LogRecord record;
    record.level = echo::Level::Warn;
    record.message = "Warning message";
    record.timestamp = "2026-01-07 12:34:56";

    std::string formatted1 = formatter1->format(record);
    assert(formatted1.find("2026-01-07") == std::string::npos && "Should not contain timestamp");
    assert(formatted1.find("WARN") != std::string::npos && "Should contain level");

    // Test without level
    auto formatter2 = std::make_shared<echo::DefaultFormatter>(true, false);
    std::string formatted2 = formatter2->format(record);
    assert(formatted2.find("2026-01-07 12:34:56") != std::string::npos && "Should contain timestamp");
    assert(formatted2.find("WARN") == std::string::npos && "Should not contain level");

    // Test without both
    auto formatter3 = std::make_shared<echo::DefaultFormatter>(false, false);
    std::string formatted3 = formatter3->format(record);
    assert(formatted3.find("2026-01-07") == std::string::npos && "Should not contain timestamp");
    assert(formatted3.find("WARN") == std::string::npos && "Should not contain level");
    assert(formatted3.find("Warning message") != std::string::npos && "Should contain message");

    std::cout << "✓ DefaultFormatter options work\n";
}

void test_pattern_formatter() {
    std::cout << "Testing PatternFormatter...\n";

    auto formatter = std::make_shared<echo::PatternFormatter>("[{time}][{level}] {msg}");

    echo::LogRecord record;
    record.level = echo::Level::Error;
    record.message = "Error occurred";
    record.timestamp = "2026-01-07 12:34:56";

    std::string formatted = formatter->format(record);

    // Should match the pattern
    assert(formatted.find("[2026-01-07 12:34:56]") != std::string::npos && "Should contain formatted timestamp");
    assert(formatted.find("[ERROR]") != std::string::npos && "Should contain formatted level");
    assert(formatted.find("Error occurred") != std::string::npos && "Should contain message");

    std::cout << "  Formatted: " << formatted << "\n";
    std::cout << "✓ PatternFormatter works\n";
}

void test_pattern_formatter_custom_patterns() {
    std::cout << "Testing PatternFormatter with custom patterns...\n";

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
    assert(formatted1 == "DEBUG: Debug info" && "Simple pattern should work");

    auto formatter2 = std::make_shared<echo::PatternFormatter>("[{file}:{line}] {msg}");
    std::string formatted2 = formatter2->format(record);
    assert(formatted2.find("[test.cpp:42]") != std::string::npos && "File and line pattern should work");

    auto formatter3 = std::make_shared<echo::PatternFormatter>("{func} - {msg}");
    std::string formatted3 = formatter3->format(record);
    assert(formatted3.find("test_function - Debug info") != std::string::npos && "Function pattern should work");

    auto formatter4 = std::make_shared<echo::PatternFormatter>("[{thread}] {msg}");
    std::string formatted4 = formatter4->format(record);
    assert(formatted4.find("[12345]") != std::string::npos && "Thread pattern should work");

    std::cout << "✓ PatternFormatter custom patterns work\n";
}

void test_custom_formatter() {
    std::cout << "Testing CustomFormatter...\n";

    // Create a custom formatter with a lambda
    auto formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        return ">>> " + std::string(echo::detail::level_name(rec.level)) + ": " + rec.message + " <<<";
    });

    echo::LogRecord record;
    record.level = echo::Level::Info;
    record.message = "Custom formatted message";

    std::string formatted = formatter->format(record);

    assert(formatted == ">>> INFO: Custom formatted message <<<" && "Custom formatter should work");

    std::cout << "  Formatted: " << formatted << "\n";
    std::cout << "✓ CustomFormatter works\n";
}

void test_custom_formatter_with_json() {
    std::cout << "Testing CustomFormatter with JSON-like output...\n";

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

    assert(formatted.find("\"level\":\"WARN\"") != std::string::npos && "Should contain JSON level");
    assert(formatted.find("\"message\":\"JSON warning\"") != std::string::npos && "Should contain JSON message");
    assert(formatted.find("\"timestamp\":\"2026-01-07 12:34:56\"") != std::string::npos &&
           "Should contain JSON timestamp");

    std::cout << "  Formatted: " << formatted << "\n";
    std::cout << "✓ CustomFormatter JSON output works\n";
}

void test_formatter_with_sink() {
    std::cout << "Testing formatter integration with sink...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Create a test sink with a custom formatter
    auto sink = std::make_shared<FormatterTestSink>();
    auto formatter = std::make_shared<echo::PatternFormatter>("{level} | {msg}");
    sink->set_formatter(formatter);

    echo::add_sink(sink);

    // Log a message
    echo::info("Test message with formatter");

    // Check that the message was formatted correctly
    assert(sink->message_count() == 1 && "Should have 1 message");
    std::string msg = sink->get_message(0);
    assert(msg.find("INFO | Test message with formatter") != std::string::npos && "Message should be formatted");

    std::cout << "  Formatted message: " << msg << "\n";
    std::cout << "✓ Formatter integration with sink works\n";
}

void test_multiple_sinks_different_formatters() {
    std::cout << "Testing multiple sinks with different formatters...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Create two sinks with different formatters
    auto sink1 = std::make_shared<FormatterTestSink>();
    auto formatter1 = std::make_shared<echo::PatternFormatter>("[{level}] {msg}");
    sink1->set_formatter(formatter1);

    auto sink2 = std::make_shared<FormatterTestSink>();
    auto formatter2 =
        std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) { return "CUSTOM: " + rec.message; });
    sink2->set_formatter(formatter2);

    echo::add_sink(sink1);
    echo::add_sink(sink2);

    // Log a message
    echo::warn("Multi-sink test");

    // Check that both sinks received the message with their own formatting
    assert(sink1->message_count() == 1 && "Sink 1 should have 1 message");
    assert(sink2->message_count() == 1 && "Sink 2 should have 1 message");

    std::string msg1 = sink1->get_message(0);
    std::string msg2 = sink2->get_message(0);

    assert(msg1.find("[WARN] Multi-sink test") != std::string::npos && "Sink 1 should use pattern formatter");
    assert(msg2.find("CUSTOM: Multi-sink test") != std::string::npos && "Sink 2 should use custom formatter");

    std::cout << "  Sink 1: " << msg1 << "\n";
    std::cout << "  Sink 2: " << msg2 << "\n";
    std::cout << "✓ Multiple sinks with different formatters work\n";
}

void test_formatter_clone() {
    std::cout << "Testing formatter cloning...\n";

    // Test DefaultFormatter clone
    auto default_formatter = std::make_shared<echo::DefaultFormatter>(true, false);
    auto default_clone = default_formatter->clone();

    echo::LogRecord record;
    record.level = echo::Level::Info;
    record.message = "Clone test";
    record.timestamp = "2026-01-07 12:34:56";

    std::string original = default_formatter->format(record);
    std::string cloned = default_clone->format(record);
    assert(original == cloned && "Cloned DefaultFormatter should produce same output");

    // Test PatternFormatter clone
    auto pattern_formatter = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
    auto pattern_clone = pattern_formatter->clone();

    std::string pattern_original = pattern_formatter->format(record);
    std::string pattern_cloned = pattern_clone->format(record);
    assert(pattern_original == pattern_cloned && "Cloned PatternFormatter should produce same output");

    // Test CustomFormatter clone
    auto custom_formatter =
        std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) { return "CUSTOM: " + rec.message; });
    auto custom_clone = custom_formatter->clone();

    std::string custom_original = custom_formatter->format(record);
    std::string custom_cloned = custom_clone->format(record);
    assert(custom_original == custom_cloned && "Cloned CustomFormatter should produce same output");

    std::cout << "✓ Formatter cloning works\n";
}

void test_global_set_pattern() {
    std::cout << "Testing global set_pattern()...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Create two test sinks
    auto sink1 = std::make_shared<FormatterTestSink>();
    auto sink2 = std::make_shared<FormatterTestSink>();

    echo::add_sink(sink1);
    echo::add_sink(sink2);

    // Set a global pattern for all sinks
    echo::set_pattern("{level} | {msg}");

    // Log a message
    echo::info("Global pattern test");

    // Both sinks should have the same formatted message
    assert(sink1->message_count() == 1 && "Sink 1 should have 1 message");
    assert(sink2->message_count() == 1 && "Sink 2 should have 1 message");

    std::string msg1 = sink1->get_message(0);
    std::string msg2 = sink2->get_message(0);

    assert(msg1.find("INFO | Global pattern test") != std::string::npos && "Sink 1 should use global pattern");
    assert(msg2.find("INFO | Global pattern test") != std::string::npos && "Sink 2 should use global pattern");

    std::cout << "  Sink 1: " << msg1 << "\n";
    std::cout << "  Sink 2: " << msg2 << "\n";
    std::cout << "✓ Global set_pattern() works\n";
}

void test_global_set_formatter() {
    std::cout << "Testing global set_formatter()...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Create two test sinks
    auto sink1 = std::make_shared<FormatterTestSink>();
    auto sink2 = std::make_shared<FormatterTestSink>();

    echo::add_sink(sink1);
    echo::add_sink(sink2);

    // Set a global custom formatter for all sinks
    echo::set_formatter(std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        return "CUSTOM: " + std::string(echo::detail::level_name(rec.level)) + " - " + rec.message;
    }));

    // Log a message
    echo::warn("Global formatter test");

    // Both sinks should have the same formatted message
    assert(sink1->message_count() == 1 && "Sink 1 should have 1 message");
    assert(sink2->message_count() == 1 && "Sink 2 should have 1 message");

    std::string msg1 = sink1->get_message(0);
    std::string msg2 = sink2->get_message(0);

    assert(msg1.find("CUSTOM: warning - Global formatter test") != std::string::npos &&
           "Sink 1 should use global formatter");
    assert(msg2.find("CUSTOM: warning - Global formatter test") != std::string::npos &&
           "Sink 2 should use global formatter");

    std::cout << "  Sink 1: " << msg1 << "\n";
    std::cout << "  Sink 2: " << msg2 << "\n";
    std::cout << "✓ Global set_formatter() works\n";
}

void test_global_formatter_override() {
    std::cout << "Testing global formatter with per-sink override...\n";

    // Clear all sinks
    echo::clear_sinks();

    // Create two test sinks
    auto sink1 = std::make_shared<FormatterTestSink>();
    auto sink2 = std::make_shared<FormatterTestSink>();

    echo::add_sink(sink1);
    echo::add_sink(sink2);

    // Set a global pattern
    echo::set_pattern("[GLOBAL] {level}: {msg}");

    // Override sink2 with a custom formatter
    sink2->set_formatter(std::make_shared<echo::CustomFormatter>(
        [](const echo::LogRecord &rec) { return "[OVERRIDE] " + rec.message; }));

    // Log a message
    echo::error("Override test");

    // Sink 1 should use global pattern, sink 2 should use override
    assert(sink1->message_count() == 1 && "Sink 1 should have 1 message");
    assert(sink2->message_count() == 1 && "Sink 2 should have 1 message");

    std::string msg1 = sink1->get_message(0);
    std::string msg2 = sink2->get_message(0);

    assert(msg1.find("[GLOBAL] ERROR: Override test") != std::string::npos && "Sink 1 should use global pattern");
    assert(msg2.find("[OVERRIDE] Override test") != std::string::npos && "Sink 2 should use override formatter");

    std::cout << "  Sink 1 (global): " << msg1 << "\n";
    std::cout << "  Sink 2 (override): " << msg2 << "\n";
    std::cout << "✓ Global formatter with per-sink override works\n";
}

int main() {
    std::cout << "=== Echo Formatter System Tests ===\n\n";

    try {
        test_default_formatter();
        test_default_formatter_options();
        test_pattern_formatter();
        test_pattern_formatter_custom_patterns();
        test_custom_formatter();
        test_custom_formatter_with_json();
        test_formatter_with_sink();
        test_multiple_sinks_different_formatters();
        test_formatter_clone();
        test_global_set_pattern();
        test_global_set_formatter();
        test_global_formatter_override();

        std::cout << "\n=== All formatter tests passed! ===\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
