/**
 * @file test_long_messages.cpp
 * @brief Edge case tests for very long messages
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <sstream>
#include <string>

TEST_CASE("Very long messages") {
    echo::clear_sinks();

    SUBCASE("Message larger than memory pool (>4KB)") {
        // Create a message larger than the 4KB pool
        std::string large_msg(5000, 'A');

        // Should not crash, should fall back to heap
        CHECK_NOTHROW(echo::info(large_msg));
        CHECK_NOTHROW(echo::debug(large_msg));
        CHECK_NOTHROW(echo::print(large_msg));
    }

    SUBCASE("Very large message (>64KB)") {
        // Create a very large message
        std::string huge_msg(70000, 'B');

        // Should handle gracefully
        CHECK_NOTHROW(echo::info(huge_msg));
        CHECK_NOTHROW(echo::error(huge_msg));
    }

    SUBCASE("Multiple large arguments") {
        std::string arg1(2000, 'X');
        std::string arg2(2000, 'Y');
        std::string arg3(2000, 'Z');

        // Total > 6KB
        CHECK_NOTHROW(echo::info(arg1, arg2, arg3));
    }

    SUBCASE("Long message with colors") {
        std::string long_msg(10000, 'C');

        CHECK_NOTHROW(echo::info(long_msg).red());
        CHECK_NOTHROW(echo::info(long_msg).hex("#FF5733"));
        CHECK_NOTHROW(echo::info(long_msg).rgb(255, 87, 51));
    }

    SUBCASE("Long message with .once()") {
        std::string long_msg(8000, 'D');

        // First call should work
        CHECK_NOTHROW(echo::info(long_msg).once());

        // Subsequent calls should be skipped
        for (int i = 0; i < 100; ++i) {
            CHECK_NOTHROW(echo::info(long_msg).once());
        }
    }

    SUBCASE("Long category names") {
        std::string long_category(1000, 'a');

        CHECK_NOTHROW(echo::category(long_category).info("Message"));
        CHECK_NOTHROW(echo::set_category_level(long_category, echo::Level::Debug));
    }
}

TEST_CASE("Unicode and special characters") {
    echo::clear_sinks();

    SUBCASE("Unicode characters") {
        CHECK_NOTHROW(echo::info("Hello 世界"));
        CHECK_NOTHROW(echo::info("Привет мир"));
        CHECK_NOTHROW(echo::info("مرحبا بالعالم"));
        CHECK_NOTHROW(echo::info("こんにちは世界"));
    }

    SUBCASE("Emoji") {
        CHECK_NOTHROW(echo::info("🚀 Rocket"));
        CHECK_NOTHROW(echo::info("✅ Success"));
        CHECK_NOTHROW(echo::info("❌ Error"));
        CHECK_NOTHROW(echo::info("🎉 🎊 🎈"));
    }

    SUBCASE("Mixed Unicode and ASCII") {
        CHECK_NOTHROW(echo::info("User: ", "Alice", " Status: ✅"));
        CHECK_NOTHROW(echo::info("Temperature: ", 25, "°C"));
    }

    SUBCASE("Special characters") {
        CHECK_NOTHROW(echo::info("Newline\nTab\tCarriage\rReturn"));
        CHECK_NOTHROW(echo::info("Null\0Character"));
        CHECK_NOTHROW(echo::info("Backslash\\Quote\"Apostrophe'"));
    }

    SUBCASE("ANSI escape sequences in message") {
        // User might accidentally include ANSI codes
        CHECK_NOTHROW(echo::info("\033[31mRed\033[0m"));
        CHECK_NOTHROW(echo::info("\033[1;32mBold Green\033[0m"));
    }

    SUBCASE("Unicode in category names") {
        CHECK_NOTHROW(echo::category("网络").info("Message"));
        CHECK_NOTHROW(echo::category("データベース").warn("Warning"));
    }

    SUBCASE("Very long Unicode string") {
        std::string unicode_msg;
        for (int i = 0; i < 1000; ++i) {
            unicode_msg += "世界";
        }
        CHECK_NOTHROW(echo::info(unicode_msg));
    }
}

TEST_CASE("Message boundary conditions") {
    echo::clear_sinks();

    SUBCASE("Exactly 4KB message") {
        std::string msg_4kb(4096, 'X');
        CHECK_NOTHROW(echo::info(msg_4kb));
    }

    SUBCASE("Just under 4KB") {
        std::string msg(4095, 'Y');
        CHECK_NOTHROW(echo::info(msg));
    }

    SUBCASE("Just over 4KB") {
        std::string msg(4097, 'Z');
        CHECK_NOTHROW(echo::info(msg));
    }

    SUBCASE("Power of 2 sizes") {
        for (int power = 10; power <= 16; ++power) {
            size_t size = 1 << power; // 1KB, 2KB, 4KB, 8KB, 16KB, 32KB, 64KB
            std::string msg(size, 'A');
            CHECK_NOTHROW(echo::info(msg));
        }
    }
}
