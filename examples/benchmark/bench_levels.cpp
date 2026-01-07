/**
 * @file bench_levels.cpp
 * @brief Log level filtering performance benchmarks
 *
 * Tests runtime level filtering:
 * - Messages that pass the filter
 * - Messages that are filtered out
 * - Different log levels
 * - Runtime level changes
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/sinks/null_sink.hpp>
#include <echo/sinks/registry.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std::chrono;

struct BenchResult {
    std::string name;
    double avg_ns;
    double min_ns;
    double max_ns;
    size_t iterations;
    double ops_per_sec;
};

template <typename Func> BenchResult benchmark(const std::string &name, Func func, size_t iterations = 100000) {
    std::vector<double> times;
    times.reserve(iterations);

    // Warmup
    for (size_t i = 0; i < 1000; ++i) {
        func();
    }

    // Actual benchmark
    for (size_t i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        times.push_back(duration_cast<nanoseconds>(end - start).count());
    }

    // Calculate statistics
    double sum = 0;
    double min_val = times[0];
    double max_val = times[0];

    for (double t : times) {
        sum += t;
        if (t < min_val)
            min_val = t;
        if (t > max_val)
            max_val = t;
    }

    double avg = sum / iterations;

    return {name, avg, min_val, max_val, iterations, 1e9 / avg};
}

void print_result(const BenchResult &r) {
    std::cout << std::left << std::setw(45) << r.name << " | " << std::right << std::setw(10) << std::fixed
              << std::setprecision(2) << r.avg_ns << " ns | " << std::setw(10) << r.min_ns << " ns | " << std::setw(10)
              << r.max_ns << " ns | " << std::setw(12) << std::setprecision(0) << r.ops_per_sec << " ops/s\n";
}

int main() {
    std::cout << "\n=== LOG LEVEL FILTERING BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // Test messages that pass the filter
    echo::set_level(echo::Level::Trace);
    results.push_back(benchmark("trace (level=Trace, passes)", []() { echo::trace("test message"); }));
    results.push_back(benchmark("debug (level=Trace, passes)", []() { echo::debug("test message"); }));
    results.push_back(benchmark("info (level=Trace, passes)", []() { echo::info("test message"); }));
    results.push_back(benchmark("warn (level=Trace, passes)", []() { echo::warn("test message"); }));
    results.push_back(benchmark("error (level=Trace, passes)", []() { echo::error("test message"); }));
    results.push_back(benchmark("critical (level=Trace, passes)", []() { echo::critical("test message"); }));

    // Test messages that are filtered out
    echo::set_level(echo::Level::Error);
    results.push_back(benchmark("trace (level=Error, filtered)", []() { echo::trace("test message"); }));
    results.push_back(benchmark("debug (level=Error, filtered)", []() { echo::debug("test message"); }));
    results.push_back(benchmark("info (level=Error, filtered)", []() { echo::info("test message"); }));
    results.push_back(benchmark("warn (level=Error, filtered)", []() { echo::warn("test message"); }));
    results.push_back(benchmark("error (level=Error, passes)", []() { echo::error("test message"); }));
    results.push_back(benchmark("critical (level=Error, passes)", []() { echo::critical("test message"); }));

    // Test with Info level (most common)
    echo::set_level(echo::Level::Info);
    results.push_back(benchmark("trace (level=Info, filtered)", []() { echo::trace("test message"); }));
    results.push_back(benchmark("debug (level=Info, filtered)", []() { echo::debug("test message"); }));
    results.push_back(benchmark("info (level=Info, passes)", []() { echo::info("test message"); }));
    results.push_back(benchmark("warn (level=Info, passes)", []() { echo::warn("test message"); }));

    // Test with Off level (all filtered)
    echo::set_level(echo::Level::Off);
    results.push_back(benchmark("info (level=Off, filtered)", []() { echo::info("test message"); }));
    results.push_back(benchmark("error (level=Off, filtered)", []() { echo::error("test message"); }));

    // Reset to default
    echo::set_level(echo::Level::Info);

    // Print results
    std::cout << std::left << std::setw(45) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: Filtered messages should be significantly faster (early exit)\n";
    std::cout << "All benchmarks use NullSink to isolate level filtering overhead\n";

    return 0;
}
