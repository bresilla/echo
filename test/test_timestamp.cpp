#include <doctest/doctest.h>

#include <regex>
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
};

// Test WITH timestamp enabled
#define LOGLEVEL Trace
#define ECHO_ENABLE_TIMESTAMP
#include <echo/echo.hpp>

TEST_CASE("Timestamp appears when ECHO_ENABLE_TIMESTAMP defined") {
    OutputCapture capture;
    echo::info("test message");

    std::string output = capture.get_cout();

    // Check that timestamp pattern exists: [HH:MM:SS]
    std::regex timestamp_pattern(R"(\[\d{2}:\d{2}:\d{2}\])");
    CHECK(std::regex_search(output, timestamp_pattern));
}

TEST_CASE("Timestamp format is HH:MM:SS") {
    OutputCapture capture;
    echo::info("test");

    std::string output = capture.get_cout();

    // Verify exact format: [HH:MM:SS] with digits
    std::regex timestamp_pattern(R"(\[(\d{2}):(\d{2}):(\d{2})\])");
    std::smatch match;

    REQUIRE(std::regex_search(output, match, timestamp_pattern));

    // Extract hours, minutes, seconds
    int hours = std::stoi(match[1].str());
    int minutes = std::stoi(match[2].str());
    int seconds = std::stoi(match[3].str());

    // Validate ranges
    CHECK(hours >= 0);
    CHECK(hours <= 23);
    CHECK(minutes >= 0);
    CHECK(minutes <= 59);
    CHECK(seconds >= 0);
    CHECK(seconds <= 59);
}

TEST_CASE("Timestamp works with all log levels") {
    OutputCapture capture;

    echo::trace("trace");
    echo::debug("debug");
    echo::info("info");
    echo::warn("warn");
    echo::error("error");
    echo::critical("critical");

    std::string cout_output = capture.get_cout();
    std::string cerr_output = capture.get_cerr();

    std::regex timestamp_pattern(R"(\[\d{2}:\d{2}:\d{2}\])");

    // Check stdout levels have timestamps
    CHECK(std::regex_search(cout_output, timestamp_pattern));

    // Check stderr levels have timestamps
    CHECK(std::regex_search(cerr_output, timestamp_pattern));
}

TEST_CASE("Timestamp appears before log level") {
    OutputCapture capture;
    echo::info("test");

    std::string output = capture.get_cout();

    // Pattern: [HH:MM:SS][level]
    std::regex pattern(R"(\[\d{2}:\d{2}:\d{2}\].*\[info\])");
    CHECK(std::regex_search(output, pattern));
}

TEST_CASE("Timestamp with multiple arguments") {
    OutputCapture capture;
    echo::info("Value: ", 42, " and ", 3.14);

    std::string output = capture.get_cout();

    std::regex timestamp_pattern(R"(\[\d{2}:\d{2}:\d{2}\])");
    CHECK(std::regex_search(output, timestamp_pattern));
    CHECK(output.find("42") != std::string::npos);
    CHECK(output.find("3.14") != std::string::npos);
}
