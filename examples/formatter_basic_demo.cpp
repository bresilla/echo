/**
 * @file formatter_basic_demo.cpp
 * @brief Demonstrates formatter usage directly (without sinks)
 */

#include <echo/echo.hpp>

#include <iostream>

int main() {
    std::cout << "=== Echo Formatter Basic Demo ===\n\n";

    // Create a sample log record
    echo::LogRecord record;
    record.level = echo::Level::Info;
    record.message = "This is a test message";
    record.timestamp = "2026-01-07 12:34:56.789";
    record.file = "main.cpp";
    record.line = 42;
    record.function = "main";
    record.thread_id = 12345;

    // ========================================
    // 1. Default Formatter
    // ========================================
    std::cout << "1. DefaultFormatter (with timestamp and level):\n";
    auto default_formatter = std::make_shared<echo::DefaultFormatter>(true, true);
    std::cout << "   " << default_formatter->format(record) << "\n\n";

    std::cout << "2. DefaultFormatter (message only):\n";
    auto minimal_formatter = std::make_shared<echo::DefaultFormatter>(false, false);
    std::cout << "   " << minimal_formatter->format(record) << "\n\n";

    // ========================================
    // 3. Pattern Formatter
    // ========================================
    std::cout << "3. PatternFormatter - Simple:\n";
    auto pattern1 = std::make_shared<echo::PatternFormatter>("{level}: {msg}");
    std::cout << "   " << pattern1->format(record) << "\n\n";

    std::cout << "4. PatternFormatter - With timestamp:\n";
    auto pattern2 = std::make_shared<echo::PatternFormatter>("[{time}] {level} | {msg}");
    std::cout << "   " << pattern2->format(record) << "\n\n";

    std::cout << "5. PatternFormatter - With file/line:\n";
    auto pattern3 = std::make_shared<echo::PatternFormatter>("{file}:{line} - {level}: {msg}");
    std::cout << "   " << pattern3->format(record) << "\n\n";

    std::cout << "6. PatternFormatter - With function:\n";
    auto pattern4 = std::make_shared<echo::PatternFormatter>("[{func}] {msg}");
    std::cout << "   " << pattern4->format(record) << "\n\n";

    std::cout << "7. PatternFormatter - With thread:\n";
    auto pattern5 = std::make_shared<echo::PatternFormatter>("[Thread {thread}] {level}: {msg}");
    std::cout << "   " << pattern5->format(record) << "\n\n";

    std::cout << "8. PatternFormatter - Complex:\n";
    auto pattern6 =
        std::make_shared<echo::PatternFormatter>("[{time}][{level}][{file}:{line}][{func}][Thread:{thread}] {msg}");
    std::cout << "   " << pattern6->format(record) << "\n\n";

    // ========================================
    // 9. Custom Formatter - Uppercase
    // ========================================
    std::cout << "9. CustomFormatter - Uppercase:\n";
    auto uppercase_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::string msg = rec.message;
        std::string level = echo::detail::level_name(rec.level);
        for (char &c : msg)
            c = std::toupper(c);
        for (char &c : level)
            c = std::toupper(c);
        return "[" + level + "] " + msg;
    });
    std::cout << "   " << uppercase_formatter->format(record) << "\n\n";

    // ========================================
    // 10. Custom Formatter - JSON
    // ========================================
    std::cout << "10. CustomFormatter - JSON:\n";
    auto json_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::ostringstream oss;
        oss << "{"
            << "\"timestamp\":\"" << rec.timestamp << "\","
            << "\"level\":\"" << echo::detail::level_name(rec.level) << "\","
            << "\"message\":\"" << rec.message << "\","
            << "\"file\":\"" << rec.file << "\","
            << "\"line\":" << rec.line << ","
            << "\"function\":\"" << rec.function << "\","
            << "\"thread\":" << rec.thread_id << "}";
        return oss.str();
    });
    std::cout << "   " << json_formatter->format(record) << "\n\n";

    // ========================================
    // 11. Custom Formatter - Emoji
    // ========================================
    std::cout << "11. CustomFormatter - With emojis:\n";
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
    std::cout << "   " << emoji_formatter->format(record) << "\n\n";

    // ========================================
    // 12. Test different log levels
    // ========================================
    std::cout << "12. Different log levels with emoji formatter:\n";
    auto emoji_level_formatter = std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
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
        return emoji + " [" + std::string(echo::detail::level_name(rec.level)) + "] " + rec.message;
    });

    echo::LogRecord trace_rec = record;
    trace_rec.level = echo::Level::Trace;
    trace_rec.message = "Trace message";
    std::cout << "   " << emoji_level_formatter->format(trace_rec) << "\n";

    echo::LogRecord debug_rec = record;
    debug_rec.level = echo::Level::Debug;
    debug_rec.message = "Debug message";
    std::cout << "   " << emoji_level_formatter->format(debug_rec) << "\n";

    echo::LogRecord info_rec = record;
    info_rec.level = echo::Level::Info;
    info_rec.message = "Info message";
    std::cout << "   " << emoji_level_formatter->format(info_rec) << "\n";

    echo::LogRecord warn_rec = record;
    warn_rec.level = echo::Level::Warn;
    warn_rec.message = "Warning message";
    std::cout << "   " << emoji_level_formatter->format(warn_rec) << "\n";

    echo::LogRecord error_rec = record;
    error_rec.level = echo::Level::Error;
    error_rec.message = "Error message";
    std::cout << "   " << emoji_level_formatter->format(error_rec) << "\n";

    echo::LogRecord critical_rec = record;
    critical_rec.level = echo::Level::Critical;
    critical_rec.message = "Critical message";
    std::cout << "   " << emoji_level_formatter->format(critical_rec) << "\n\n";

    std::cout << "=== Demo Complete ===\n";
    std::cout << "\nNote: These formatters can be used with custom sinks that create LogRecord objects.\n";
    std::cout << "See test/test_formatters.cpp for examples of formatter integration with sinks.\n";

    return 0;
}
