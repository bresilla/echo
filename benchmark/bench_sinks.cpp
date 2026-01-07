/**
 * @file bench_sinks.cpp
 * @brief Benchmark sink overhead comparison
 *
 * Compares performance of different sink types:
 * - ConsoleSink
 * - FileSink
 * - Multiple sinks
 */

#include <echo/echo.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono;

class Timer {
  private:
    high_resolution_clock::time_point start_;
    std::string name_;

  public:
    explicit Timer(std::string name) : start_(high_resolution_clock::now()), name_(std::move(name)) {}

    ~Timer() {
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start_).count();
        std::cout << name_ << ": " << duration << " μs\n";
    }
};

void bench_no_sinks() {
    std::cout << "\n=== No Sinks (baseline) ===\n";

    echo::clear_sinks();
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 10000;

    {
        Timer t("10k messages with no sinks");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Message ", i);
        }
    }
}

void bench_console_sink() {
    std::cout << "\n=== ConsoleSink ===\n";

    echo::clear_sinks();
    auto console = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console);
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 1000;

    {
        Timer t("1k messages to console");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Message ", i);
        }
    }
}

#ifdef ECHO_ENABLE_FILE_SINK
void bench_file_sink() {
    std::cout << "\n=== FileSink ===\n";

    echo::clear_sinks();
    auto file = std::make_shared<echo::FileSink>("/tmp/bench_echo.log");
    echo::add_sink(file);
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 10000;

    {
        Timer t("10k messages to file");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Message ", i);
        }
    }

    // Clean up
    std::remove("/tmp/bench_echo.log");
}
#endif

void bench_multiple_sinks() {
    std::cout << "\n=== Multiple Sinks ===\n";

    echo::clear_sinks();
    auto console = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console);

#ifdef ECHO_ENABLE_FILE_SINK
    auto file = std::make_shared<echo::FileSink>("/tmp/bench_echo_multi.log");
    echo::add_sink(file);
#endif

    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 1000;

    {
        Timer t("1k messages to multiple sinks");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Message ", i);
        }
    }

#ifdef ECHO_ENABLE_FILE_SINK
    std::remove("/tmp/bench_echo_multi.log");
#endif
}

int main() {
    std::cout << "=== Echo Sink Benchmarks ===\n";
    std::cout << "Lower is better (μs = microseconds)\n";

    bench_no_sinks();
    bench_console_sink();

#ifdef ECHO_ENABLE_FILE_SINK
    bench_file_sink();
#else
    std::cout << "\n=== FileSink ===\n";
    std::cout << "FileSink not available (compile with -DECHO_ENABLE_FILE_SINK)\n";
#endif

    bench_multiple_sinks();

    std::cout << "\n=== Benchmarks Complete ===\n";
    return 0;
}
