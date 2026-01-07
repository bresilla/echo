/**
 * @file bench_latency.cpp
 * @brief Latency and percentile benchmarks
 *
 * Tests latency characteristics:
 * - Minimum latency
 * - Average latency
 * - P50, P95, P99, P99.9 percentiles
 * - Maximum latency
 * - Latency distribution
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/sinks/null_sink.hpp>
#include <echo/sinks/registry.hpp>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std::chrono;

struct LatencyResult {
    std::string name;
    double min_ns;
    double p50_ns;
    double p95_ns;
    double p99_ns;
    double p999_ns;
    double max_ns;
    double avg_ns;
};

template <typename Func> LatencyResult measure_latency(const std::string &name, Func func, size_t iterations = 100000) {
    std::vector<double> times;
    times.reserve(iterations);

    // Warmup
    for (size_t i = 0; i < 1000; ++i) {
        func();
    }

    // Actual measurement
    for (size_t i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        times.push_back(duration_cast<nanoseconds>(end - start).count());
    }

    // Sort for percentile calculation
    std::sort(times.begin(), times.end());

    // Calculate statistics
    double sum = 0;
    for (double t : times) {
        sum += t;
    }
    double avg = sum / iterations;

    size_t p50_idx = iterations * 50 / 100;
    size_t p95_idx = iterations * 95 / 100;
    size_t p99_idx = iterations * 99 / 100;
    size_t p999_idx = iterations * 999 / 1000;

    return {
        name,
        times[0],              // min
        times[p50_idx],        // p50
        times[p95_idx],        // p95
        times[p99_idx],        // p99
        times[p999_idx],       // p99.9
        times[iterations - 1], // max
        avg                    // average
    };
}

void print_result(const LatencyResult &r) {
    std::cout << std::left << std::setw(35) << r.name << " | " << std::right << std::setw(8) << std::fixed
              << std::setprecision(1) << r.min_ns << " | " << std::setw(8) << r.p50_ns << " | " << std::setw(8)
              << r.p95_ns << " | " << std::setw(8) << r.p99_ns << " | " << std::setw(8) << r.p999_ns << " | "
              << std::setw(8) << r.max_ns << " | " << std::setw(8) << r.avg_ns << "\n";
}

int main() {
    std::cout << "\n=== LATENCY PERCENTILE BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<LatencyResult> results;

    // Basic operations
    results.push_back(measure_latency("Simple string", []() { echo::info("test"); }));
    results.push_back(measure_latency("Integer", []() { echo::info(42); }));
    results.push_back(measure_latency("Float", []() { echo::info(3.14159); }));
    results.push_back(measure_latency("Multiple args", []() { echo::info("Value:", 42, "Pi:", 3.14); }));

    // Different log levels
    results.push_back(measure_latency("trace level", []() { echo::trace("test"); }));
    results.push_back(measure_latency("debug level", []() { echo::debug("test"); }));
    results.push_back(measure_latency("info level", []() { echo::info("test"); }));
    results.push_back(measure_latency("warn level", []() { echo::warn("test"); }));
    results.push_back(measure_latency("error level", []() { echo::error("test"); }));

    // Filtered messages
    echo::set_level(echo::Level::Error);
    results.push_back(measure_latency("Filtered (level=Error)", []() { echo::info("test"); }));
    echo::set_level(echo::Level::Trace);

    // String sizes
    std::string small(10, 'x');
    std::string medium(100, 'x');
    std::string large(1000, 'x');
    results.push_back(measure_latency("Small string (10)", [&]() { echo::info(small); }));
    results.push_back(measure_latency("Medium string (100)", [&]() { echo::info(medium); }));
    results.push_back(measure_latency("Large string (1000)", [&]() { echo::info(large); }));

    // Modifiers
    results.push_back(measure_latency(".once() modifier", []() { echo::info("test").once(); }));
    results.push_back(measure_latency(".when(true) modifier", []() { echo::info("test").when(true); }));
    results.push_back(measure_latency(".when(false) modifier", []() { echo::info("test").when(false); }));

    // Print results
    std::cout << std::left << std::setw(35) << "Benchmark"
              << " | " << std::setw(8) << "Min"
              << " | " << std::setw(8) << "P50"
              << " | " << std::setw(8) << "P95"
              << " | " << std::setw(8) << "P99"
              << " | " << std::setw(8) << "P99.9"
              << " | " << std::setw(8) << "Max"
              << " | " << std::setw(8) << "Avg\n";
    std::cout << std::string(120, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: All values in nanoseconds (ns)\n";
    std::cout << "P50 = median, P95/P99/P99.9 = tail latencies\n";
    std::cout << "All benchmarks use NullSink to isolate logging overhead\n";

    return 0;
}
