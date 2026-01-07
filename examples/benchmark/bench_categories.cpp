/**
 * @file bench_categories.cpp
 * @brief Category filtering performance benchmarks
 *
 * Tests category-based filtering:
 * - Messages that match category filter
 * - Messages that are filtered out
 * - Multiple categories
 * - Category enable/disable
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/filters/category.hpp>
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
    std::cout << "\n=== CATEGORY FILTERING BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // Baseline - no category filtering
    results.push_back(benchmark("No category filtering", []() { echo::info("test message"); }));

    // Enable category filtering by setting level to Info
    echo::set_category_level("network", echo::Level::Info);
    results.push_back(
        benchmark("Category enabled (matches)", []() { echo::category("network").info("test message"); }));

    results.push_back(
        benchmark("Category enabled (no match)", []() { echo::category("database").info("test message"); }));

    // Multiple categories enabled
    echo::set_category_level("database", echo::Level::Info);
    echo::set_category_level("ui", echo::Level::Info);
    results.push_back(
        benchmark("Multiple categories (matches network)", []() { echo::category("network").info("test message"); }));

    results.push_back(
        benchmark("Multiple categories (matches database)", []() { echo::category("database").info("test message"); }));

    results.push_back(
        benchmark("Multiple categories (matches ui)", []() { echo::category("ui").info("test message"); }));

    results.push_back(
        benchmark("Multiple categories (no match)", []() { echo::category("audio").info("test message"); }));

    // Disable all categories by setting level to Critical (higher than Info)
    echo::set_category_level("network", echo::Level::Critical);
    echo::set_category_level("database", echo::Level::Critical);
    echo::set_category_level("ui", echo::Level::Critical);
    results.push_back(benchmark("All categories disabled", []() { echo::category("network").info("test message"); }));

    // Print results
    std::cout << std::left << std::setw(45) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: Category filtering adds overhead for hash lookup\n";
    std::cout << "All benchmarks use NullSink to isolate category filtering overhead\n";

    return 0;
}
