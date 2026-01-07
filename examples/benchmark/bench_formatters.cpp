/**
 * @file bench_formatters.cpp
 * @brief Formatter performance benchmarks
 *
 * Tests different formatting scenarios:
 * - Pattern formatters
 * - Custom formatters
 * - Complex patterns
 * - Timestamp formatting
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/formatters/pattern.hpp>
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

template <typename Func> BenchResult benchmark(const std::string &name, Func func, size_t iterations = 50000) {
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
    std::cout << "\n=== FORMATTER PERFORMANCE BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // Simple pattern
    echo::set_pattern("[%l] %m");
    results.push_back(benchmark("Simple pattern [%l] %m", []() { echo::info("test message"); }));

    // Standard pattern with timestamp
    echo::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %m");
    results.push_back(benchmark("Standard pattern with timestamp", []() { echo::info("test message"); }));

    // Complex pattern
    echo::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %m");
    results.push_back(benchmark("Complex pattern (full info)", []() { echo::info("test message"); }));

    // Very complex pattern
    echo::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %m");
    results.push_back(benchmark("Very complex pattern (with source)", []() { echo::info("test message"); }));

    // Message only
    echo::set_pattern("%m");
    results.push_back(benchmark("Message only pattern", []() { echo::info("test message"); }));

    // Level only
    echo::set_pattern("%l");
    results.push_back(benchmark("Level only pattern", []() { echo::info("test message"); }));

    // Timestamp only
    echo::set_pattern("%Y-%m-%d %H:%M:%S.%e");
    results.push_back(benchmark("Timestamp only pattern", []() { echo::info("test message"); }));

    // Multiple timestamps
    echo::set_pattern("%Y-%m-%d %H:%M:%S | %H:%M:%S.%e | %m");
    results.push_back(benchmark("Multiple timestamps pattern", []() { echo::info("test message"); }));

    // Reset to default
    echo::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %m");

    // Print results
    std::cout << std::left << std::setw(45) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: All benchmarks use NullSink to isolate formatter overhead\n";

    return 0;
}
