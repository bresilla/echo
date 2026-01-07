/**
 * @file bench_basic.cpp
 * @brief Basic logging performance benchmarks
 *
 * Tests fundamental logging operations:
 * - Simple string logging
 * - Integer logging
 * - Float logging
 * - Multiple arguments
 * - Different log levels
 */

#include <echo/echo.hpp>

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
    std::cout << std::left << std::setw(40) << r.name << " | " << std::right << std::setw(10) << std::fixed
              << std::setprecision(2) << r.avg_ns << " ns | " << std::setw(10) << r.min_ns << " ns | " << std::setw(10)
              << r.max_ns << " ns | " << std::setw(12) << std::setprecision(0) << r.ops_per_sec << " ops/s\n";
}

int main() {
    std::cout << "\n=== BASIC LOGGING BENCHMARKS ===\n\n";

    // Disable output for fair benchmarking
    echo::clear_sinks();

    std::vector<BenchResult> results;

    // Simple string logging
    results.push_back(benchmark("Simple string (literal)", []() { echo::info("Hello World"); }));

    std::string msg = "Hello World";
    results.push_back(benchmark("Simple string (variable)", [&]() { echo::info(msg); }));

    // Integer logging
    results.push_back(benchmark("Single integer", []() { echo::info(42); }));

    results.push_back(benchmark("Multiple integers", []() { echo::info(1, 2, 3, 4, 5); }));

    // Float logging
    results.push_back(benchmark("Single float", []() { echo::info(3.14159); }));

    results.push_back(benchmark("Multiple floats", []() { echo::info(1.1, 2.2, 3.3, 4.4, 5.5); }));

    // Mixed types
    results.push_back(benchmark("Mixed types", []() { echo::info("Value:", 42, "Pi:", 3.14, "Done"); }));

    // Different log levels
    results.push_back(benchmark("trace level", []() { echo::trace("trace message"); }));
    results.push_back(benchmark("debug level", []() { echo::debug("debug message"); }));
    results.push_back(benchmark("info level", []() { echo::info("info message"); }));
    results.push_back(benchmark("warn level", []() { echo::warn("warn message"); }));
    results.push_back(benchmark("error level", []() { echo::error("error message"); }));
    results.push_back(benchmark("critical level", []() { echo::critical("critical message"); }));

    // Simple echo function
    results.push_back(benchmark("echo() function", []() { echo("simple echo"); }));

    // Long strings
    std::string long_msg(100, 'x');
    results.push_back(benchmark("Long string (100 chars)", [&]() { echo::info(long_msg); }));

    std::string very_long_msg(1000, 'x');
    results.push_back(benchmark("Very long string (1000 chars)", [&]() { echo::info(very_long_msg); }));

    // Print results
    std::cout << std::left << std::setw(40) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(90, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: All benchmarks run with sinks disabled (null output)\n";

    return 0;
}
