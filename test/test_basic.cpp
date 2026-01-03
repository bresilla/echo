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
};

TEST_CASE("Log level filtering") {
    SUBCASE("Trace level logs everything") {
        OutputCapture capture;
        echo::trace("trace message");
        echo::debug("debug message");
        echo::info("info message");
        echo::warn("warn message");
        echo::error("error message");
        echo::critical("critical message");

        std::string cout_output = capture.get_cout();
        std::string cerr_output = capture.get_cerr();

        // Trace, Debug, Info, Warn go to stdout
        CHECK(cout_output.find("trace message") != std::string::npos);
        CHECK(cout_output.find("debug message") != std::string::npos);
        CHECK(cout_output.find("info message") != std::string::npos);
        CHECK(cout_output.find("warn message") != std::string::npos);

        // Error and Critical go to stderr
        CHECK(cerr_output.find("error message") != std::string::npos);
        CHECK(cerr_output.find("critical message") != std::string::npos);
    }
}

TEST_CASE("Different types can be logged") {
    SUBCASE("Basic types") {
        OutputCapture capture;
        echo::info("Integer: ", 42);
        echo::info("Float: ", 3.14);
        echo::info("String: ", "hello");

        std::string output = capture.get_cout();
        CHECK(output.find("42") != std::string::npos);
        CHECK(output.find("3.14") != std::string::npos);
        CHECK(output.find("hello") != std::string::npos);
    }

    SUBCASE("Custom types with print()") {
        struct Point {
            int x, y;
            std::string print() const { return "Point(" + std::to_string(x) + "," + std::to_string(y) + ")"; }
        };

        OutputCapture capture;
        Point p{10, 20};
        echo::info("Point: ", p);

        std::string output = capture.get_cout();
        CHECK(output.find("Point(10,20)") != std::string::npos);
    }

    SUBCASE("Custom types with pretty()") {
        struct Vector {
            float x, y;
            std::string print() const { return "Vec"; }
            std::string pretty() const { return "Vector(" + std::to_string(x) + "," + std::to_string(y) + ")"; }
        };

        OutputCapture capture;
        Vector v{1.5f, 2.5f};
        echo::info("Vector: ", v);

        std::string output = capture.get_cout();
        // pretty() should be preferred over print()
        CHECK(output.find("Vector(") != std::string::npos);
    }

    SUBCASE("Custom types with to_string()") {
        struct Color {
            int r, g, b;
            std::string to_string() const {
                return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
            }
        };

        OutputCapture capture;
        Color c{255, 128, 0};
        echo::info("Color: ", c);

        std::string output = capture.get_cout();
        CHECK(output.find("rgb(255,128,0)") != std::string::npos);
    }
}

TEST_CASE("Multiple arguments concatenate") {
    OutputCapture capture;
    echo::info("Value: ", 42, " and ", 3.14, " and ", "text");

    std::string output = capture.get_cout();
    CHECK(output.find("42") != std::string::npos);
    CHECK(output.find("3.14") != std::string::npos);
    CHECK(output.find("text") != std::string::npos);
}

TEST_CASE("Color codes are present") {
    SUBCASE("Each level has color") {
        OutputCapture capture;
        echo::trace("trace");
        echo::debug("debug");
        echo::info("info");
        echo::warn("warn");
        echo::error("error");
        echo::critical("critical");

        std::string cout_output = capture.get_cout();
        std::string cerr_output = capture.get_cerr();

        // Check for ANSI escape codes (color codes start with \033[)
        CHECK(cout_output.find("\033[") != std::string::npos);
        CHECK(cerr_output.find("\033[") != std::string::npos);
    }
}

TEST_CASE("Level names are correct") {
    OutputCapture capture;
    echo::trace("msg");
    echo::debug("msg");
    echo::info("msg");
    echo::warn("msg");
    echo::error("msg");
    echo::critical("msg");

    std::string cout_output = capture.get_cout();
    std::string cerr_output = capture.get_cerr();

    CHECK(cout_output.find("[trace]") != std::string::npos);
    CHECK(cout_output.find("[debug]") != std::string::npos);
    CHECK(cout_output.find("[info]") != std::string::npos);
    CHECK(cout_output.find("[warning]") != std::string::npos);
    CHECK(cerr_output.find("[error]") != std::string::npos);
    CHECK(cerr_output.find("[critical]") != std::string::npos);
}

TEST_CASE("Utility functions") {
    SUBCASE("current_level returns active level") {
        auto level = echo::current_level();
        CHECK(level == echo::Level::Trace);
    }

    SUBCASE("is_enabled checks level") {
        CHECK(echo::is_enabled<echo::Level::Trace>() == true);
        CHECK(echo::is_enabled<echo::Level::Debug>() == true);
        CHECK(echo::is_enabled<echo::Level::Info>() == true);
    }
}
