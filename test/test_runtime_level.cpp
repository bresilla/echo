#include <doctest/doctest.h>

#define LOGLEVEL Trace
#include <echo/echo.hpp>

#include <sstream>
#include <string>

// Helper to capture output
struct OutputCapture {
    std::streambuf *old_cout;
    std::streambuf *old_cerr;
    std::ostringstream cout_stream;
    std::ostringstream cerr_stream;

    OutputCapture() {
        old_cout = std::cout.rdbuf(cout_stream.rdbuf());
        old_cerr = std::cerr.rdbuf(cerr_stream.rdbuf());
    }

    ~OutputCapture() {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
    }

    std::string get_cout() { return cout_stream.str(); }
    std::string get_cerr() { return cerr_stream.str(); }
    std::string get_all() { return get_cout() + get_cerr(); }
};

TEST_CASE("Default level is compile-time level") {
    // Reset to default
    echo::set_level(echo::Level::Off);

    auto level = echo::get_level();
    CHECK(level == echo::Level::Trace); // Compile-time level is Trace
}

TEST_CASE("set_level changes active level") {
    echo::set_level(echo::Level::Info);
    CHECK(echo::get_level() == echo::Level::Info);

    echo::set_level(echo::Level::Error);
    CHECK(echo::get_level() == echo::Level::Error);

    echo::set_level(echo::Level::Debug);
    CHECK(echo::get_level() == echo::Level::Debug);

    // Reset
    echo::set_level(echo::Level::Off);
}

TEST_CASE("Logs filter correctly after level change") {
    SUBCASE("Set to Info level") {
        echo::set_level(echo::Level::Info);

        OutputCapture capture;
        echo::trace("trace");
        echo::debug("debug");
        echo::info("info");
        echo::warn("warn");

        std::string output = capture.get_all();

        // Trace and Debug should be filtered
        CHECK(output.find("trace") == std::string::npos);
        CHECK(output.find("debug") == std::string::npos);

        // Info and Warn should appear
        CHECK(output.find("info") != std::string::npos);
        CHECK(output.find("warn") != std::string::npos);

        echo::set_level(echo::Level::Off); // Reset
    }

    SUBCASE("Set to Error level") {
        echo::set_level(echo::Level::Error);

        OutputCapture capture;
        echo::info("info");
        echo::warn("warn");
        echo::error("error");
        echo::critical("critical");

        std::string output = capture.get_all();

        // Info and Warn should be filtered
        CHECK(output.find("info") == std::string::npos);
        CHECK(output.find("warn") == std::string::npos);

        // Error and Critical should appear
        CHECK(output.find("error") != std::string::npos);
        CHECK(output.find("critical") != std::string::npos);

        echo::set_level(echo::Level::Off); // Reset
    }

    SUBCASE("Set to Trace level") {
        echo::set_level(echo::Level::Trace);

        OutputCapture capture;
        echo::trace("trace");
        echo::debug("debug");
        echo::info("info");

        std::string output = capture.get_all();

        // All should appear
        CHECK(output.find("trace") != std::string::npos);
        CHECK(output.find("debug") != std::string::npos);
        CHECK(output.find("info") != std::string::npos);

        echo::set_level(echo::Level::Off); // Reset
    }
}

TEST_CASE("Level::Off resets to compile-time level") {
    // Change to a different level
    echo::set_level(echo::Level::Error);
    CHECK(echo::get_level() == echo::Level::Error);

    // Reset to compile-time level
    echo::set_level(echo::Level::Off);
    CHECK(echo::get_level() == echo::Level::Trace); // Compile-time level

    OutputCapture capture;
    echo::trace("trace should appear");
    echo::debug("debug should appear");

    std::string output = capture.get_all();
    CHECK(output.find("trace should appear") != std::string::npos);
    CHECK(output.find("debug should appear") != std::string::npos);
}

TEST_CASE("Runtime level changes are immediate") {
    echo::set_level(echo::Level::Error);

    {
        OutputCapture capture;
        echo::info("should not appear");
        std::string output = capture.get_all();
        CHECK(output.find("should not appear") == std::string::npos);
    }

    // Change level
    echo::set_level(echo::Level::Info);

    {
        OutputCapture capture;
        echo::info("should appear now");
        std::string output = capture.get_all();
        CHECK(output.find("should appear now") != std::string::npos);
    }

    echo::set_level(echo::Level::Off); // Reset
}

TEST_CASE("Multiple level changes work correctly") {
    echo::set_level(echo::Level::Trace);
    CHECK(echo::get_level() == echo::Level::Trace);

    echo::set_level(echo::Level::Debug);
    CHECK(echo::get_level() == echo::Level::Debug);

    echo::set_level(echo::Level::Info);
    CHECK(echo::get_level() == echo::Level::Info);

    echo::set_level(echo::Level::Warn);
    CHECK(echo::get_level() == echo::Level::Warn);

    echo::set_level(echo::Level::Error);
    CHECK(echo::get_level() == echo::Level::Error);

    echo::set_level(echo::Level::Critical);
    CHECK(echo::get_level() == echo::Level::Critical);

    echo::set_level(echo::Level::Off);
    CHECK(echo::get_level() == echo::Level::Trace); // Back to compile-time
}

TEST_CASE("Runtime level persists across calls") {
    echo::set_level(echo::Level::Warn);

    {
        OutputCapture capture;
        echo::info("info1");
        echo::warn("warn1");
        std::string output = capture.get_all();
        CHECK(output.find("info1") == std::string::npos);
        CHECK(output.find("warn1") != std::string::npos);
    }

    // Level should still be Warn
    {
        OutputCapture capture;
        echo::debug("debug2");
        echo::error("error2");
        std::string output = capture.get_all();
        CHECK(output.find("debug2") == std::string::npos);
        CHECK(output.find("error2") != std::string::npos);
    }

    echo::set_level(echo::Level::Off); // Reset
}
