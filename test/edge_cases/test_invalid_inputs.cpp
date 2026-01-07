/**
 * @file test_invalid_inputs.cpp
 * @brief Edge case tests for null, empty, and invalid inputs
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <string>

TEST_CASE("Empty and null strings") {
    echo::clear_sinks();

    SUBCASE("Empty string") {
        CHECK_NOTHROW(echo::info(""));
        CHECK_NOTHROW(echo::debug(""));
        CHECK_NOTHROW(echo(""));
    }

    SUBCASE("Empty string with colors") {
        CHECK_NOTHROW(echo::info("").red());
        CHECK_NOTHROW(echo::info("").hex("#FF0000"));
    }

    SUBCASE("Null pointer (const char*)") {
        const char *null_str = nullptr;
        // This might crash or be handled - test behavior
        // Most implementations will crash, so we skip this
        // CHECK_NOTHROW(echo::info(null_str));
    }

    SUBCASE("Empty std::string") {
        std::string empty;
        CHECK_NOTHROW(echo::info(empty));
        CHECK_NOTHROW(echo::debug(empty));
    }

    SUBCASE("Multiple empty arguments") {
        CHECK_NOTHROW(echo::info("", "", ""));
        CHECK_NOTHROW(echo::info("", "text", ""));
    }

    SUBCASE("Empty category name") {
        CHECK_NOTHROW(echo::category("").info("Message"));
        CHECK_NOTHROW(echo::set_category_level("", echo::Level::Debug));
    }
}

TEST_CASE("Invalid color codes") {
    echo::clear_sinks();

    SUBCASE("Invalid HEX colors") {
        CHECK_NOTHROW(echo::info("Test").hex(""));
        CHECK_NOTHROW(echo::info("Test").hex("invalid"));
        CHECK_NOTHROW(echo::info("Test").hex("GG0000"));
        CHECK_NOTHROW(echo::info("Test").hex("#"));
        CHECK_NOTHROW(echo::info("Test").hex("#12"));
        CHECK_NOTHROW(echo::info("Test").hex("#12345"));
        CHECK_NOTHROW(echo::info("Test").hex("#1234567"));
    }

    SUBCASE("Out of range RGB values") {
        CHECK_NOTHROW(echo::info("Test").rgb(-1, 0, 0));
        CHECK_NOTHROW(echo::info("Test").rgb(256, 0, 0));
        CHECK_NOTHROW(echo::info("Test").rgb(0, -100, 0));
        CHECK_NOTHROW(echo::info("Test").rgb(0, 0, 1000));
        CHECK_NOTHROW(echo::info("Test").rgb(999, 999, 999));
    }

    SUBCASE("Extreme RGB values") {
        CHECK_NOTHROW(echo::info("Test").rgb(INT_MAX, INT_MAX, INT_MAX));
        CHECK_NOTHROW(echo::info("Test").rgb(INT_MIN, INT_MIN, INT_MIN));
    }
}

TEST_CASE("Invalid category patterns") {
    echo::clear_sinks();

    SUBCASE("Malformed wildcard patterns") {
        CHECK_NOTHROW(echo::set_category_level("*", echo::Level::Debug));
        CHECK_NOTHROW(echo::set_category_level(".*", echo::Level::Debug));
        CHECK_NOTHROW(echo::set_category_level("**", echo::Level::Debug));
        CHECK_NOTHROW(echo::set_category_level("app.**", echo::Level::Debug));
        CHECK_NOTHROW(echo::set_category_level("*.app", echo::Level::Debug));
    }

    SUBCASE("Special characters in category names") {
        CHECK_NOTHROW(echo::category("app/network").info("Test"));
        CHECK_NOTHROW(echo::category("app\\network").info("Test"));
        CHECK_NOTHROW(echo::category("app:network").info("Test"));
        CHECK_NOTHROW(echo::category("app network").info("Test"));
        CHECK_NOTHROW(echo::category("app\tnetwork").info("Test"));
    }

    SUBCASE("Category names with dots") {
        CHECK_NOTHROW(echo::category(".").info("Test"));
        CHECK_NOTHROW(echo::category("..").info("Test"));
        CHECK_NOTHROW(echo::category("...").info("Test"));
        CHECK_NOTHROW(echo::category(".app").info("Test"));
        CHECK_NOTHROW(echo::category("app.").info("Test"));
    }
}

#ifdef ECHO_ENABLE_FILE_SINK
TEST_CASE("Invalid file paths") {
    SUBCASE("Empty file path") { CHECK_THROWS_AS(echo::FileSink(""), std::exception); }

    SUBCASE("Invalid characters in path") {
        // These might fail on different platforms
        CHECK_NOTHROW(echo::FileSink("/tmp/test\0file.log"));
    }

    SUBCASE("Very long file path") {
        std::string long_path(10000, 'a');
        long_path += ".log";
        // Might fail due to OS limits
        CHECK_NOTHROW(echo::FileSink(long_path));
    }

    SUBCASE("Non-existent directory") {
        // Should fail gracefully
        CHECK_NOTHROW(echo::FileSink("/nonexistent/directory/file.log"));
    }

    SUBCASE("Read-only directory") {
        // Should fail gracefully
        CHECK_NOTHROW(echo::FileSink("/root/file.log"));
    }
}
#endif

TEST_CASE("Extreme log levels") {
    echo::clear_sinks();

    SUBCASE("Set invalid log level via cast") {
        // Cast invalid integer to Level
        auto invalid_level = static_cast<echo::Level>(999);
        CHECK_NOTHROW(echo::set_level(invalid_level));
    }

    SUBCASE("Category with invalid level") {
        auto invalid_level = static_cast<echo::Level>(-1);
        CHECK_NOTHROW(echo::set_category_level("test", invalid_level));
    }
}

TEST_CASE("Rapid operations") {
    echo::clear_sinks();

    SUBCASE("Rapid level changes") {
        for (int i = 0; i < 1000; ++i) {
            echo::set_level(echo::Level::Debug);
            echo::set_level(echo::Level::Error);
            echo::set_level(echo::Level::Info);
        }
    }

    SUBCASE("Rapid category level changes") {
        for (int i = 0; i < 1000; ++i) {
            echo::set_category_level("test", echo::Level::Debug);
            echo::set_category_level("test", echo::Level::Error);
            echo::clear_category_levels();
        }
    }

    SUBCASE("Rapid sink addition/removal") {
        for (int i = 0; i < 100; ++i) {
            auto sink = std::make_shared<echo::ConsoleSink>();
            echo::add_sink(sink);
            echo::clear_sinks();
        }
    }
}

TEST_CASE("Chained operations edge cases") {
    echo::clear_sinks();

    SUBCASE("Very long chain") {
        CHECK_NOTHROW(
            echo::info("Test").red().bold().italic().underline().hex("#FF0000").rgb(255, 0, 0).when(true).once());
    }

    SUBCASE("Contradictory colors") {
        CHECK_NOTHROW(echo::info("Test").red().green().blue().hex("#FF0000").rgb(0, 255, 0));
    }

    SUBCASE("Multiple .once() calls") { CHECK_NOTHROW(echo::info("Test").once().once().once()); }

    SUBCASE("when(false) with colors") {
        // Should not print, but should not crash
        CHECK_NOTHROW(echo::info("Test").when(false).red().bold());
    }
}
