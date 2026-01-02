#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#define LOGLEVEL Trace
#include <echo/banner.hpp>

#include <sstream>
#include <string>

// Helper to capture output
struct OutputCapture {
    std::streambuf *old_cout;
    std::ostringstream cout_stream;

    OutputCapture() { old_cout = std::cout.rdbuf(cout_stream.rdbuf()); }

    ~OutputCapture() { std::cout.rdbuf(old_cout); }

    std::string get() { return cout_stream.str(); }
};

TEST_CASE("separator() with no arguments creates full line") {
    OutputCapture capture;
    echo::separator();
    std::string output = capture.get();

    // Should be a line of dashes ending with newline
    CHECK(output.length() > 0);
    CHECK(output.back() == '\n');

    // Remove newline and check all chars are dashes
    std::string line = output.substr(0, output.length() - 1);
    CHECK(line.length() > 0);
    for (char c : line) {
        CHECK(c == '-');
    }
}

TEST_CASE("separator() with text creates centered output") {
    OutputCapture capture;
    echo::separator("TEST");
    std::string output = capture.get();

    // Should contain the text with brackets
    CHECK(output.find("[ TEST ]") != std::string::npos);

    // Should have separators on both sides
    CHECK(output.find("-") != std::string::npos);

    // Should end with newline
    CHECK(output.back() == '\n');
}

TEST_CASE("separator() with custom character") {
    SUBCASE("Equals sign") {
        OutputCapture capture;
        echo::separator("SECTION", '=');
        std::string output = capture.get();

        CHECK(output.find("[ SECTION ]") != std::string::npos);
        CHECK(output.find("=") != std::string::npos);
        CHECK(output.find("-") == std::string::npos); // No dashes
    }

    SUBCASE("Asterisk") {
        OutputCapture capture;
        echo::separator("TITLE", '*');
        std::string output = capture.get();

        CHECK(output.find("[ TITLE ]") != std::string::npos);
        CHECK(output.find("*") != std::string::npos);
    }

    SUBCASE("Hash") {
        OutputCapture capture;
        echo::separator("HEADER", '#');
        std::string output = capture.get();

        CHECK(output.find("[ HEADER ]") != std::string::npos);
        CHECK(output.find("#") != std::string::npos);
    }
}

TEST_CASE("separator() with various text lengths") {
    SUBCASE("Short text") {
        OutputCapture capture;
        echo::separator("Hi");
        std::string output = capture.get();

        CHECK(output.find("[ Hi ]") != std::string::npos);
        // Should have plenty of separators
        size_t dash_count = 0;
        for (char c : output) {
            if (c == '-')
                dash_count++;
        }
        CHECK(dash_count > 10); // Lots of dashes for short text
    }

    SUBCASE("Medium text") {
        OutputCapture capture;
        echo::separator("This is a medium length text");
        std::string output = capture.get();

        CHECK(output.find("[ This is a medium length text ]") != std::string::npos);
    }

    SUBCASE("Long text") {
        OutputCapture capture;
        std::string long_text(60, 'x');
        echo::separator(long_text);
        std::string output = capture.get();

        CHECK(output.find("[ " + long_text + " ]") != std::string::npos);
    }
}

TEST_CASE("separator() centering is correct") {
    OutputCapture capture;
    echo::separator("CENTER");
    std::string output = capture.get();

    // Remove newline
    std::string line = output.substr(0, output.length() - 1);

    // Find the text position
    size_t text_pos = line.find("[ CENTER ]");
    REQUIRE(text_pos != std::string::npos);

    // Count separators on left and right
    size_t left_sep = text_pos;
    size_t right_sep = line.length() - (text_pos + 10); // "[ CENTER ]" is 10 chars

    // Should be roughly equal (difference at most 1 for odd widths)
    CHECK(std::abs(static_cast<int>(left_sep) - static_cast<int>(right_sep)) <= 1);
}

TEST_CASE("separator() with empty string behaves like no arguments") {
    OutputCapture capture1;
    echo::separator();
    std::string output1 = capture1.get();

    OutputCapture capture2;
    echo::separator("");
    std::string output2 = capture2.get();

    // Both should produce the same output
    CHECK(output1 == output2);
}

TEST_CASE("separator() output format is consistent") {
    OutputCapture capture;
    echo::separator("TEST", '-');
    std::string output = capture.get();

    // Format should be: ---[ TEST ]---\n
    // Check structure
    CHECK(output.find("[ TEST ]") != std::string::npos);
    CHECK(output.back() == '\n');

    // Should not have extra spaces
    CHECK(output.find("  ") == std::string::npos); // No double spaces
}

TEST_CASE("separator() with different characters produces different output") {
    OutputCapture capture1;
    echo::separator("TEXT", '-');
    std::string output1 = capture1.get();

    OutputCapture capture2;
    echo::separator("TEXT", '=');
    std::string output2 = capture2.get();

    // Outputs should be different
    CHECK(output1 != output2);

    // But both should contain the text
    CHECK(output1.find("[ TEXT ]") != std::string::npos);
    CHECK(output2.find("[ TEXT ]") != std::string::npos);
}
