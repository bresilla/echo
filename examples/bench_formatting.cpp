/**
 * @file bench_formatting.cpp
 * @brief Benchmark string formatting performance
 *
 * Compares:
 * - std::format vs ostringstream (if C++20 available)
 * - Different argument types
 * - Custom type formatting
 */

#include <echo/echo.hpp>

#include <chrono>
#include <iostream>
#include <sstream>

#ifdef ECHO_HAS_STD_FORMAT
#include <format>
#endif

using namespace std::chrono;

class Timer {
  private:
    high_resolution_clock::time_point start_;
    std::string name_;

  public:
    explicit Timer(std::string name) : start_(high_resolution_clock::now()), name_(std::move(name)) {}

    ~Timer() {
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start_).count();
        std::cout << name_ << ": " << duration << " ns (" << (duration / 1000.0) << " μs)\n";
    }
};

// Custom type with pretty() method
struct Point {
    int x, y;
    std::string pretty() const { return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }
};

void bench_echo_formatting() {
    std::cout << "\n=== Echo Formatting (current implementation) ===\n";

    echo::clear_sinks();
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 100000;

    {
        Timer t("Simple string x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Simple message");
        }
    }

    {
        Timer t("String + int x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Value: ", i);
        }
    }

    {
        Timer t("Multiple args x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("User ", "alice", " age ", 30, " active ", true);
        }
    }

    {
        Timer t("Custom type x100k");
        Point p{10, 20};
        for (int i = 0; i < iterations; ++i) {
            echo::info("Position: ", p);
        }
    }
}

void bench_raw_ostringstream() {
    std::cout << "\n=== Raw ostringstream (baseline) ===\n";

    constexpr int iterations = 100000;

    {
        Timer t("Simple string x100k");
        for (int i = 0; i < iterations; ++i) {
            std::ostringstream oss;
            oss << "Simple message";
            [[maybe_unused]] auto str = oss.str();
        }
    }

    {
        Timer t("String + int x100k");
        for (int i = 0; i < iterations; ++i) {
            std::ostringstream oss;
            oss << "Value: " << i;
            [[maybe_unused]] auto str = oss.str();
        }
    }

    {
        Timer t("Multiple args x100k");
        for (int i = 0; i < iterations; ++i) {
            std::ostringstream oss;
            oss << "User " << "alice" << " age " << 30 << " active " << true;
            [[maybe_unused]] auto str = oss.str();
        }
    }
}

#ifdef ECHO_HAS_STD_FORMAT
void bench_raw_std_format() {
    std::cout << "\n=== Raw std::format (C++20) ===\n";

    constexpr int iterations = 100000;

    {
        Timer t("Simple string x100k");
        for (int i = 0; i < iterations; ++i) {
            [[maybe_unused]] auto str = std::format("Simple message");
        }
    }

    {
        Timer t("String + int x100k");
        for (int i = 0; i < iterations; ++i) {
            [[maybe_unused]] auto str = std::format("Value: {}", i);
        }
    }

    {
        Timer t("Multiple args x100k");
        for (int i = 0; i < iterations; ++i) {
            [[maybe_unused]] auto str = std::format("User {} age {} active {}", "alice", 30, true);
        }
    }
}
#endif

int main() {
    std::cout << "=== Echo Formatting Benchmarks ===\n";
    std::cout << "Lower is better (ns = nanoseconds, μs = microseconds)\n";

#ifdef ECHO_HAS_STD_FORMAT
    std::cout << "std::format: AVAILABLE (C++20)\n";
#else
    std::cout << "std::format: NOT AVAILABLE (using ostringstream)\n";
#endif

    bench_echo_formatting();
    bench_raw_ostringstream();

#ifdef ECHO_HAS_STD_FORMAT
    bench_raw_std_format();
#endif

    std::cout << "\n=== Benchmarks Complete ===\n";
    return 0;
}
