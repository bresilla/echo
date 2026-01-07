/**
 * @file global_formatter_demo.cpp
 * @brief Demonstrates global formatter API (set_pattern and set_formatter)
 */

#include <echo/echo.hpp>

#include <iostream>

int main() {
    std::cout << "=== Echo Global Formatter API Demo ===\n\n";

    // ========================================
    // 1. Default behavior (no custom formatter)
    // ========================================
    std::cout << "1. Default formatter:\n";
    echo::info("Default format message");
    echo::warn("Default warning");
    std::cout << "\n";

    // ========================================
    // 2. Global set_pattern() - Simple
    // ========================================
    std::cout << "2. Global set_pattern() - Simple:\n";
    echo::set_pattern("{level}: {msg}");

    echo::info("Simple pattern");
    echo::warn("Warning with simple pattern");
    echo::error("Error with simple pattern");
    std::cout << "\n";

    // ========================================
    // 3. Global set_pattern() - Detailed
    // ========================================
    std::cout << "3. Global set_pattern() - Detailed:\n";
    echo::set_pattern("[{time}] {level} | {msg}");

    echo::info("Detailed pattern with timestamp");
    echo::warn("Warning with timestamp");
    echo::error("Error with timestamp");
    std::cout << "\n";

    // ========================================
    // 4. Global set_pattern() - Custom brackets
    // ========================================
    std::cout << "4. Global set_pattern() - Custom brackets:\n";
    echo::set_pattern(">>> {level} <<< {msg}");

    echo::info("Custom bracket pattern");
    echo::warn("Warning with custom brackets");
    std::cout << "\n";

    // ========================================
    // 5. Global set_formatter() - Uppercase
    // ========================================
    std::cout << "5. Global set_formatter() - Uppercase:\n";
    echo::set_formatter(std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
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
    }));

    echo::info("This will be uppercase");
    echo::warn("Warning in uppercase");
    echo::error("Error in uppercase");
    std::cout << "\n";

    // ========================================
    // 6. Global set_formatter() - JSON
    // ========================================
    std::cout << "6. Global set_formatter() - JSON:\n";
    echo::set_formatter(std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
        std::ostringstream oss;
        oss << "{"
            << "\"level\":\"" << echo::detail::level_name(rec.level) << "\","
            << "\"message\":\"" << rec.message << "\"";

        if (!rec.timestamp.empty()) {
            oss << ",\"timestamp\":\"" << rec.timestamp << "\"";
        }

        oss << "}";
        return oss.str();
    }));

    echo::info("JSON formatted message");
    echo::warn("JSON warning");
    echo::error("JSON error");
    std::cout << "\n";

    // ========================================
    // 7. Global set_formatter() - Emoji
    // ========================================
    std::cout << "7. Global set_formatter() - Emoji:\n";
    echo::set_formatter(std::make_shared<echo::CustomFormatter>([](const echo::LogRecord &rec) {
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
    }));

    echo::trace("Trace with emoji");
    echo::debug("Debug with emoji");
    echo::info("Info with emoji");
    echo::warn("Warning with emoji");
    echo::error("Error with emoji");
    std::cout << "\n";

    // ========================================
    // 8. Multiple sinks with global formatter
    // ========================================
    std::cout << "8. Multiple sinks with global formatter:\n";

    // Clear and add multiple sinks
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::ConsoleSink>());
    echo::add_sink(std::make_shared<echo::ConsoleSink>());

    // Set global pattern - applies to all sinks
    echo::set_pattern("[GLOBAL] {level}: {msg}");

    echo::info("Message to all sinks");
    echo::warn("Warning to all sinks");
    std::cout << "\n";

    // ========================================
    // 9. Per-sink override after global
    // ========================================
    std::cout << "9. Per-sink override after global formatter:\n";

    // Clear and add two sinks
    echo::clear_sinks();
    auto sink1 = std::make_shared<echo::ConsoleSink>();
    auto sink2 = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(sink1);
    echo::add_sink(sink2);

    // Set global pattern
    echo::set_pattern("[SINK1] {level}: {msg}");

    // Override sink2 with custom formatter
    sink2->set_formatter(std::make_shared<echo::CustomFormatter>(
        [](const echo::LogRecord &rec) { return "[SINK2-OVERRIDE] " + rec.message; }));

    echo::info("Message with mixed formatters");
    echo::warn("Warning with mixed formatters");
    std::cout << "\n";

    // ========================================
    // 10. Reset to default
    // ========================================
    std::cout << "10. Reset to default formatter:\n";

    // Clear and add default console sink
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::ConsoleSink>());

    echo::info("Back to default format");
    echo::warn("Default warning");
    std::cout << "\n";

    std::cout << "=== Demo Complete ===\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "- echo::set_pattern() sets a pattern formatter for ALL sinks\n";
    std::cout << "- echo::set_formatter() sets a custom formatter for ALL sinks\n";
    std::cout << "- Individual sinks can override the global formatter\n";
    std::cout << "- Formatters can be changed at runtime\n";

    return 0;
}
