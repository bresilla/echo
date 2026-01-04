#include <doctest/doctest.h>

#define LOGLEVEL Trace
#include <echo/echo.hpp>

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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

    std::vector<std::string> get_lines() {
        std::string output = get_cout() + get_cerr();
        std::vector<std::string> lines;
        std::istringstream iss(output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        return lines;
    }
};

TEST_CASE("Multiple threads can log concurrently") {
    OutputCapture capture;

    const int num_threads = 10;
    const int logs_per_thread = 100;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, logs_per_thread]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                echo::info("Thread ", i, " message ", j);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    auto lines = capture.get_lines();

    // Should have exactly num_threads * logs_per_thread lines
    CHECK(lines.size() == num_threads * logs_per_thread);
}

TEST_CASE("Log messages are not interleaved") {
    OutputCapture capture;

    const int num_threads = 5;
    const int logs_per_thread = 50;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, logs_per_thread]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                echo::info("T", i, "M", j);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    auto lines = capture.get_lines();

    // Each line should be complete and well-formed
    // Check that each line contains the expected parts (ignoring ANSI codes)
    for (const auto &line : lines) {
        // Each line should contain [info] and T<digit>M<digit> pattern
        CHECK(line.find("[info]") != std::string::npos);
        // Check for T followed by digit, M followed by digit
        bool has_pattern = false;
        for (size_t i = 0; i < line.length() - 2; ++i) {
            if (line[i] == 'T' && std::isdigit(line[i + 1])) {
                // Found T<digit>, now look for M<digit>
                for (size_t j = i + 2; j < line.length() - 1; ++j) {
                    if (line[j] == 'M' && std::isdigit(line[j + 1])) {
                        has_pattern = true;
                        break;
                    }
                }
                if (has_pattern)
                    break;
            }
        }
        CHECK(has_pattern);
    }
}

TEST_CASE("Each log line is complete and intact") {
    OutputCapture capture;

    const int num_threads = 8;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            // Log a longer message with multiple parts
            echo::info("Thread-", i, "-START-", i * 100, "-MIDDLE-", i * 200, "-END");
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    auto lines = capture.get_lines();

    CHECK(lines.size() == num_threads);

    // Each line should contain all parts
    for (const auto &line : lines) {
        CHECK(line.find("START") != std::string::npos);
        CHECK(line.find("MIDDLE") != std::string::npos);
        CHECK(line.find("END") != std::string::npos);
    }
}

TEST_CASE("Different log levels are thread-safe") {
    OutputCapture capture;

    const int num_threads = 6;

    std::vector<std::thread> threads;

    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::trace("trace", i);
    });
    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::debug("debug", i);
    });
    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::info("info", i);
    });
    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::warn("warn", i);
    });
    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::error("error", i);
    });
    threads.emplace_back([]() {
        for (int i = 0; i < 20; ++i)
            echo::critical("critical", i);
    });

    for (auto &t : threads) {
        t.join();
    }

    auto lines = capture.get_lines();

    // Should have 6 threads * 20 messages = 120 lines
    CHECK(lines.size() == 120);

    // Count each level
    int trace_count = 0, debug_count = 0, info_count = 0;
    int warn_count = 0, error_count = 0, critical_count = 0;

    for (const auto &line : lines) {
        if (line.find("[trace]") != std::string::npos)
            trace_count++;
        if (line.find("[debug]") != std::string::npos)
            debug_count++;
        if (line.find("[info]") != std::string::npos)
            info_count++;
        if (line.find("[warning]") != std::string::npos)
            warn_count++;
        if (line.find("[error]") != std::string::npos)
            error_count++;
        if (line.find("[critical]") != std::string::npos)
            critical_count++;
    }

    CHECK(trace_count == 20);
    CHECK(debug_count == 20);
    CHECK(info_count == 20);
    CHECK(warn_count == 20);
    CHECK(error_count == 20);
    CHECK(critical_count == 20);
}

TEST_CASE("High contention stress test") {
    OutputCapture capture;

    const int num_threads = 20;
    const int logs_per_thread = 50;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, logs_per_thread]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                echo::info("T", i, ":", j);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    auto lines = capture.get_lines();

    // All messages should be logged
    CHECK(lines.size() == num_threads * logs_per_thread);

    // All lines should be well-formed
    for (const auto &line : lines) {
        // Should contain [info] and the pattern T<num>:<num>
        CHECK(line.find("[info]") != std::string::npos);
        CHECK(line.find("T") != std::string::npos);
        CHECK(line.find(":") != std::string::npos);
    }
}
