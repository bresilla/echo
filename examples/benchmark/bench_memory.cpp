/**
 * @file bench_memory.cpp
 * @brief Memory allocation and usage benchmarks
 *
 * Tests memory-related performance:
 * - Stack vs heap allocations
 * - String allocations
 * - Message buffer sizes
 * - Memory pooling effects
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
    std::cout << "\n=== MEMORY ALLOCATION BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // String literal (no allocation)
    results.push_back(benchmark("String literal", []() { echo::info("test"); }));

    // Small string (SSO - Small String Optimization)
    std::string small_str = "test";
    results.push_back(benchmark("Small string (SSO)", [&]() { echo::info(small_str); }));

    // Medium string (likely heap allocated)
    std::string medium_str(50, 'x');
    results.push_back(benchmark("Medium string (50 chars)", [&]() { echo::info(medium_str); }));

    // Large string (definitely heap allocated)
    std::string large_str(200, 'x');
    results.push_back(benchmark("Large string (200 chars)", [&]() { echo::info(large_str); }));

    // Very large string
    std::string very_large_str(1000, 'x');
    results.push_back(benchmark("Very large string (1000 chars)", [&]() { echo::info(very_large_str); }));

    // Multiple small arguments (multiple allocations)
    results.push_back(benchmark("5 small strings", []() { echo::info("a", "b", "c", "d", "e"); }));

    // Multiple integers (minimal allocation)
    results.push_back(benchmark("5 integers", []() { echo::info(1, 2, 3, 4, 5); }));

    // Mixed types (various allocations)
    results.push_back(benchmark("Mixed types (10 args)",
                                []() { echo::info("str", 42, 3.14, "another", 100, 2.71, "more", 999, 1.41, "end"); }));

    // String concatenation scenarios
    results.push_back(benchmark("String concat (2 args)", []() { echo::info("Hello", "World"); }));

    results.push_back(benchmark("String concat (5 args)", []() { echo::info("a", "b", "c", "d", "e"); }));

    results.push_back(
        benchmark("String concat (10 args)", []() { echo::info("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"); }));

    // Temporary string creation
    results.push_back(benchmark("Temporary string creation", []() { echo::info(std::string("temporary")); }));

    // String view (if supported)
    std::string_view sv = "string_view_test";
    results.push_back(benchmark("String view", [&]() { echo::info(sv); }));

    // Print results
    std::cout << std::left << std::setw(45) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: All benchmarks use NullSink to isolate memory allocation overhead\n";
    std::cout << "SSO = Small String Optimization (strings stored on stack, not heap)\n";

    return 0;
}
