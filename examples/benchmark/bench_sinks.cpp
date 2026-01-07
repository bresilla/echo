/**
 * @file bench_sinks.cpp
 * @brief Sink performance benchmarks
 *
 * Tests different sink configurations:
 * - Null sink (no output)
 * - Console sink
 * - File sink
 * - Multiple sinks
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/sinks/console_sink.hpp>
#include <echo/sinks/file_sink.hpp>
#include <echo/sinks/null_sink.hpp>
#include <echo/sinks/registry.hpp>

#include <chrono>
#include <fstream>
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

template <typename Func> BenchResult benchmark(const std::string &name, Func func, size_t iterations = 10000) {
    std::vector<double> times;
    times.reserve(iterations);

    // Warmup
    for (size_t i = 0; i < 100; ++i) {
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
    std::cout << "\n=== SINK PERFORMANCE BENCHMARKS ===\n\n";

    std::vector<BenchResult> results;

    // Null sink (baseline - no sinks)
    echo::clear_sinks();
    results.push_back(benchmark("No sinks (null output)", []() { echo::info("test message"); }));

    // Console sink
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::ConsoleSink>());
    results.push_back(benchmark("Console sink (stdout)", []() { echo::info("test message"); }, 1000));

    // File sink
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo.log"));
    results.push_back(benchmark("File sink", []() { echo::info("test message"); }, 5000));

    // Multiple file sinks
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo1.log"));
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo2.log"));
    results.push_back(benchmark("Two file sinks", []() { echo::info("test message"); }, 5000));

    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo1.log"));
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo2.log"));
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo3.log"));
    results.push_back(benchmark("Three file sinks", []() { echo::info("test message"); }, 5000));

    // Mixed sinks
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::ConsoleSink>());
    echo::add_sink(std::make_shared<echo::FileSink>("/tmp/bench_echo.log"));
    results.push_back(benchmark("Console + File sink", []() { echo::info("test message"); }, 1000));

    // Null sink explicit
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());
    results.push_back(benchmark("NullSink explicit", []() { echo::info("test message"); }));

    // Print results
    std::cout << std::left << std::setw(40) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(90, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    // Cleanup
    std::remove("/tmp/bench_echo.log");
    std::remove("/tmp/bench_echo1.log");
    std::remove("/tmp/bench_echo2.log");
    std::remove("/tmp/bench_echo3.log");

    return 0;
}
