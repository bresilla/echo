/**
 * @file formatter_demo.cpp
 * @brief Demonstrates custom formatters with sinks
 */

#include <echo/echo.hpp>

#include <iostream>
#include <sstream>

int main() {
    std::cout << "=== Echo Formatter Demo ===\n\n";

    // Clear default sinks
    echo::clear_sinks();

    // ========================================
    // 1. Default Formatter
    // ========================================
    std::cout << "1. Default Formatter (with timestamp and level):\n";
    auto console_sink1 = std::make_shared<echo::ConsoleSink>();
    auto default_formatter = std::make_shared<echo::DefaultFormatter>(true, true);
    console_sink1->set_formatter(default_formatter);
    echo::add_sink(console_sink1);

    echo::info("This is an info message");
    echo::warn("This is a warning message");
    echo::error("This is an error message");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 2. Default Formatter (message only)
    // ========================================
    std::cout << "2. Default Formatter (message only, no timestamp/level):\n";
    auto console_sink2 = std::make_shared<echo::ConsoleSink>();
    auto minimal_formatter = std::make_shared<echo::DefaultFormatter>(false, false);
    console_sink2->set_formatter(minimal_formatter);
    echo::add_sink(console_sink2);

    echo::info("Clean message without metadata");
    echo::warn("Another clean message");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 3. Pattern Formatter - Simple
    // ========================================
    std::cout << "3. Pattern Formatter (simple pattern):\n";
    auto console_sink3 = std::make_shared<echo::ConsoleSink>();
    auto pattern_formatter1 = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
    console_sink3->set_formatter(pattern_formatter1);
    echo::add_sink(console_sink3);

    echo::info("Simple pattern format");
    echo::warn("Warning with simple pattern");
    echo::error("Error with simple pattern");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 4. Pattern Formatter - Detailed
    // ========================================
    std::cout << "4. Pattern Formatter (detailed with timestamp):\n";
    auto console_sink4 = std::make_shared<echo::ConsoleSink>();
    auto pattern_formatter2 = std::make_shared<echo::PatternFormatter>("[{time}] {level:5} | {msg}");
    console_sink4->set_formatter(pattern_formatter2);
    echo::add_sink(console_sink4);

    echo::info("Detailed pattern with timestamp");
    echo::warn("Warning with detailed pattern");
    echo::error("Error with detailed pattern");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 5. Custom Formatter - Uppercase
    // ========================================
    std::cout << "5. Custom Formatter (uppercase):\n";
    auto console_sink5 = std::make_shared<echo::ConsoleSink>();
    auto uppercase_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::string msg = rec.message;
        std::string level = echo::detail::level_name(rec.level);

        // Convert to uppercase
        for (char &c : msg) {
            c = std::toupper(c);
        }
        for (char &c : level) {
            c = std::toupper(c);
        }

        return "[" + level + "] " + msg;
    });
    console_sink5->set_formatter(uppercase_formatter);
    echo::add_sink(console_sink5);

    echo::info("This message will be uppercase");
    echo::warn("Warning in uppercase");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 6. Custom Formatter - JSON
    // ========================================
    std::cout << "6. Custom Formatter (JSON output):\n";
    auto console_sink6 = std::make_shared<echo::ConsoleSink>();
    auto json_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::ostringstream oss;
        oss << "{"
            << "\"level\":\"" << echo::detail::level_name(rec.level) << "\","
            << "\"message\":\"" << rec.message << "\"";

        if (!rec.timestamp.empty()) {
            oss << ",\"timestamp\":\"" << rec.timestamp << "\"";
        }

        oss << "}";
        return oss.str();
    });
    console_sink6->set_formatter(json_formatter);
    echo::add_sink(console_sink6);

    echo::info("JSON formatted message");
    echo::warn("JSON warning");
    echo::error("JSON error");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 7. Custom Formatter - Emoji
    // ========================================
    std::cout << "7. Custom Formatter (with emojis):\n";
    auto console_sink7 = std::make_shared<echo::ConsoleSink>();
    auto emoji_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::string emoji;
        switch (rec.level) {
        case echo::Level::Trace:
            emoji = "🔍";
            break;
        case echo::Level::Debug:
            emoji = "🐛";
            break;
        case echo::Level::Info:
            emoji = "ℹ️";
            break;
        case echo::Level::Warn:
            emoji = "⚠️";
            break;
        case echo::Level::Error:
            emoji = "❌";
            break;
        case echo::Level::Critical:
            emoji = "💀";
            break;
        default:
            emoji = "📝";
        }

        return emoji + " " + rec.message;
    });
    console_sink7->set_formatter(emoji_formatter);
    echo::add_sink(console_sink7);

    echo::trace("Trace with emoji");
    echo::debug("Debug with emoji");
    echo::info("Info with emoji");
    echo::warn("Warning with emoji");
    echo::error("Error with emoji");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 8. Multiple Sinks with Different Formatters
    // ========================================
    std::cout << "8. Multiple sinks with different formatters:\n";

    // Sink 1: Simple pattern
    auto sink_a = std::make_shared<echo::ConsoleSink>();
    auto formatter_a = std::make_shared<echo::PatternFormatter>("[A] {level}: {msg}");
    sink_a->set_formatter(formatter_a);

    // Sink 2: JSON format
    auto sink_b = std::make_shared<echo::ConsoleSink>();
    auto formatter_b = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        return "[B] {\"level\":\"" + std::string(echo::detail::level_name(rec.level)) + "\",\"msg\":\"" + rec.message +
               "\"}";
    });
    sink_b->set_formatter(formatter_b);

    echo::add_sink(sink_a);
    echo::add_sink(sink_b);

    echo::info("Message sent to both sinks");
    echo::warn("Warning to both sinks");

    echo::clear_sinks();
    std::cout << "\n";

    // ========================================
    // 9. Sink with Level Filtering
    // ========================================
    std::cout << "9. Sink with level filtering (only warnings and above):\n";
    auto filtered_sink = std::make_shared<echo::ConsoleSink>();
    filtered_sink->set_level(echo::Level::Warn);
    auto filtered_formatter = std::make_shared<echo::PatternFormatter>("[FILTERED] {level}: {msg}");
    filtered_sink->set_formatter(filtered_formatter);
    echo::add_sink(filtered_sink);

    echo::trace("This trace won't show");
    echo::debug("This debug won't show");
    echo::info("This info won't show");
    echo::warn("This warning WILL show");
    echo::error("This error WILL show");

    echo::clear_sinks();
    std::cout << "\n";

    // Restore default console sink
    echo::add_sink(std::make_shared<echo::ConsoleSink>());

    std::cout << "=== Demo Complete ===\n";
    return 0;
}
