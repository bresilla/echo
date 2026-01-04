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

TEST_CASE("kv() with different types") {
    SUBCASE("Integer values") {
        std::string result = echo::kv("count", 42, "total", 100);
        CHECK(result == "count=42 total=100");
    }

    SUBCASE("String values") {
        std::string result = echo::kv("name", "john", "status", "active");
        CHECK(result == "name=john status=active");
    }

    SUBCASE("Float values") {
        std::string result = echo::kv("pi", 3.14, "e", 2.71);
        // Float formatting may vary, so just check structure
        CHECK(result.find("pi=") != std::string::npos);
        CHECK(result.find("e=") != std::string::npos);
        CHECK(result.find("3.14") != std::string::npos);
        CHECK(result.find("2.71") != std::string::npos);
    }

    SUBCASE("Mixed types") {
        std::string result = echo::kv("user", "alice", "age", 30, "score", 95.5);
        CHECK(result.find("user=alice") != std::string::npos);
        CHECK(result.find("age=30") != std::string::npos);
        CHECK(result.find("score=") != std::string::npos);
    }
}

TEST_CASE("kv() with multiple pairs") {
    SUBCASE("Single pair") {
        std::string result = echo::kv("key", "value");
        CHECK(result == "key=value");
    }

    SUBCASE("Two pairs") {
        std::string result = echo::kv("a", 1, "b", 2);
        CHECK(result == "a=1 b=2");
    }

    SUBCASE("Three pairs") {
        std::string result = echo::kv("x", 10, "y", 20, "z", 30);
        CHECK(result == "x=10 y=20 z=30");
    }

    SUBCASE("Many pairs") {
        std::string result = echo::kv("a", 1, "b", 2, "c", 3, "d", 4, "e", 5);
        CHECK(result == "a=1 b=2 c=3 d=4 e=5");
    }
}

TEST_CASE("kv() integrates with log functions") {
    SUBCASE("With info()") {
        OutputCapture capture;
        echo::info("Login: ", echo::kv("user", "bob", "session", 123));

        std::string output = capture.get_all();
        CHECK(output.find("[info]") != std::string::npos);
        CHECK(output.find("Login:") != std::string::npos);
        CHECK(output.find("user=bob") != std::string::npos);
        CHECK(output.find("session=123") != std::string::npos);
    }

    SUBCASE("With warn()") {
        OutputCapture capture;
        echo::warn("Alert: ", echo::kv("level", "high", "code", 500));

        std::string output = capture.get_all();
        CHECK(output.find("[warning]") != std::string::npos);
        CHECK(output.find("Alert:") != std::string::npos);
        CHECK(output.find("level=high") != std::string::npos);
        CHECK(output.find("code=500") != std::string::npos);
    }

    SUBCASE("With error()") {
        OutputCapture capture;
        echo::error("Failed: ", echo::kv("reason", "timeout", "duration", 30));

        std::string output = capture.get_all();
        CHECK(output.find("[error]") != std::string::npos);
        CHECK(output.find("Failed:") != std::string::npos);
        CHECK(output.find("reason=timeout") != std::string::npos);
        CHECK(output.find("duration=30") != std::string::npos);
    }
}

TEST_CASE("kv() output format is correct") {
    SUBCASE("Format is key=value") {
        std::string result = echo::kv("name", "test");
        CHECK(result.find("=") != std::string::npos);
        CHECK(result.find("name=test") != std::string::npos);
    }

    SUBCASE("Multiple pairs separated by space") {
        std::string result = echo::kv("a", 1, "b", 2, "c", 3);
        CHECK(result.find("a=1") != std::string::npos);
        CHECK(result.find("b=2") != std::string::npos);
        CHECK(result.find("c=3") != std::string::npos);
        CHECK(result.find(" ") != std::string::npos); // Has spaces
    }

    SUBCASE("No trailing space") {
        std::string result = echo::kv("x", 1, "y", 2);
        CHECK(result == "x=1 y=2"); // Exact match, no trailing space
    }
}

TEST_CASE("kv() with custom types") {
    struct Point {
        int x, y;
        std::string print() const { return "(" + std::to_string(x) + "," + std::to_string(y) + ")"; }
    };

    Point p{10, 20};
    std::string result = echo::kv("position", p, "count", 5);

    CHECK(result.find("position=(10,20)") != std::string::npos);
    CHECK(result.find("count=5") != std::string::npos);
}

TEST_CASE("kv() can be used multiple times in one log") {
    OutputCapture capture;
    echo::info("Event: ", echo::kv("type", "login"), " ", echo::kv("user", "alice", "time", 1234));

    std::string output = capture.get_all();
    CHECK(output.find("type=login") != std::string::npos);
    CHECK(output.find("user=alice") != std::string::npos);
    CHECK(output.find("time=1234") != std::string::npos);
}

TEST_CASE("kv() alone in log") {
    OutputCapture capture;
    echo::debug(echo::kv("debug", 1, "verbose", "yes"));

    std::string output = capture.get_all();
    CHECK(output.find("[debug]") != std::string::npos);
    CHECK(output.find("debug=1") != std::string::npos);
    CHECK(output.find("verbose=yes") != std::string::npos);
}
