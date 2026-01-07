/**
 * @file bench_vs_spdlog.cpp
 * @brief Comparison benchmarks between echo and spdlog
 *
 * Direct performance comparison:
 * - Basic logging operations
 * - Formatting overhead
 * - Level filtering
 * - Threading performance
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/sinks/null_sink.hpp>
#include <echo/sinks/registry.hpp>

#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono;

struct BenchResult {
    std::string name;
    double avg_ns;
    double min_ns;
    double max_ns;
    double p50_ns;
    double p95_ns;
    double p99_ns;
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

    // Sort for percentiles
    std::vector<double> sorted_times = times;
    std::sort(sorted_times.begin(), sorted_times.end());

    // Calculate statistics
    double sum = 0;
    double min_val = sorted_times[0];
    double max_val = sorted_times[iterations - 1];

    for (double t : times) {
        sum += t;
    }

    double avg = sum / iterations;
    size_t p50_idx = iterations * 50 / 100;
    size_t p95_idx = iterations * 95 / 100;
    size_t p99_idx = iterations * 99 / 100;

    return {name,       avg,      min_val, max_val, sorted_times[p50_idx], sorted_times[p95_idx], sorted_times[p99_idx],
            iterations, 1e9 / avg};
}

void print_result(const BenchResult &r) {
    std::cout << std::left << std::setw(40) << r.name << " | " << std::right << std::setw(8) << std::fixed
              << std::setprecision(1) << r.avg_ns << " | " << std::setw(8) << r.p50_ns << " | " << std::setw(8)
              << r.p95_ns << " | " << std::setw(8) << r.p99_ns << " | " << std::setw(10) << std::setprecision(0)
              << r.ops_per_sec << "\n";
}

void print_comparison(const BenchResult &echo_result, const BenchResult &spdlog_result) {
    double speedup = spdlog_result.avg_ns / echo_result.avg_ns;
    std::string winner = speedup > 1.0 ? "echo" : "spdlog";
    double percent = std::abs(speedup - 1.0) * 100.0;

    std::cout << std::left << std::setw(40) << ("  → " + echo_result.name) << " | " << std::right << std::setw(8)
              << std::fixed << std::setprecision(2) << speedup << "x | " << std::setw(8) << std::setprecision(1)
              << percent << "% | " << winner << "\n";
}

int main() {
    std::cout << "\n=== ECHO vs SPDLOG PERFORMANCE COMPARISON ===\n\n";

    // Setup echo with null sink
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    // Setup spdlog with null sink
    auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    auto spdlog_logger = std::make_shared<spdlog::logger>("bench", null_sink);
    spdlog::set_default_logger(spdlog_logger);
    spdlog::set_level(spdlog::level::trace);

    std::vector<BenchResult> echo_results;
    std::vector<BenchResult> spdlog_results;

    std::cout << "Running benchmarks (this may take a minute)...\n\n";

    // ========== Basic Logging ==========
    std::cout << "=== Basic Logging ===\n";

    echo_results.push_back(benchmark("echo: Simple string", []() { echo::info("Hello World"); }));
    spdlog_results.push_back(benchmark("spdlog: Simple string", []() { spdlog::info("Hello World"); }));

    echo_results.push_back(benchmark("echo: Integer", []() { echo::info(42); }));
    spdlog_results.push_back(benchmark("spdlog: Integer", []() { spdlog::info("{}", 42); }));

    echo_results.push_back(benchmark("echo: Float", []() { echo::info(3.14159); }));
    spdlog_results.push_back(benchmark("spdlog: Float", []() { spdlog::info("{}", 3.14159); }));

    echo_results.push_back(benchmark("echo: Multiple args", []() { echo::info("Value:", 42, "Pi:", 3.14); }));
    spdlog_results.push_back(benchmark("spdlog: Multiple args", []() { spdlog::info("Value: {} Pi: {}", 42, 3.14); }));

    // ========== String Sizes ==========
    std::cout << "\n=== String Sizes ===\n";

    std::string small(10, 'x');
    std::string medium(100, 'x');
    std::string large(1000, 'x');

    echo_results.push_back(benchmark("echo: Small string (10)", [&]() { echo::info(small); }));
    spdlog_results.push_back(benchmark("spdlog: Small string (10)", [&]() { spdlog::info(small); }));

    echo_results.push_back(benchmark("echo: Medium string (100)", [&]() { echo::info(medium); }));
    spdlog_results.push_back(benchmark("spdlog: Medium string (100)", [&]() { spdlog::info(medium); }));

    echo_results.push_back(benchmark("echo: Large string (1000)", [&]() { echo::info(large); }));
    spdlog_results.push_back(benchmark("spdlog: Large string (1000)", [&]() { spdlog::info(large); }));

    // ========== Level Filtering ==========
    std::cout << "\n=== Level Filtering ===\n";

    echo::set_level(echo::Level::Error);
    spdlog::set_level(spdlog::level::err);

    echo_results.push_back(benchmark("echo: Filtered trace", []() { echo::trace("test"); }));
    spdlog_results.push_back(benchmark("spdlog: Filtered trace", []() { spdlog::trace("test"); }));

    echo_results.push_back(benchmark("echo: Filtered info", []() { echo::info("test"); }));
    spdlog_results.push_back(benchmark("spdlog: Filtered info", []() { spdlog::info("test"); }));

    echo::set_level(echo::Level::Trace);
    spdlog::set_level(spdlog::level::trace);

    echo_results.push_back(benchmark("echo: Passed error", []() { echo::error("test"); }));
    spdlog_results.push_back(benchmark("spdlog: Passed error", []() { spdlog::error("test"); }));

    // ========== Different Log Levels ==========
    std::cout << "\n=== Different Log Levels ===\n";

    echo_results.push_back(benchmark("echo: trace", []() { echo::trace("test"); }));
    spdlog_results.push_back(benchmark("spdlog: trace", []() { spdlog::trace("test"); }));

    echo_results.push_back(benchmark("echo: debug", []() { echo::debug("test"); }));
    spdlog_results.push_back(benchmark("spdlog: debug", []() { spdlog::debug("test"); }));

    echo_results.push_back(benchmark("echo: info", []() { echo::info("test"); }));
    spdlog_results.push_back(benchmark("spdlog: info", []() { spdlog::info("test"); }));

    echo_results.push_back(benchmark("echo: warn", []() { echo::warn("test"); }));
    spdlog_results.push_back(benchmark("spdlog: warn", []() { spdlog::warn("test"); }));

    echo_results.push_back(benchmark("echo: error", []() { echo::error("test"); }));
    spdlog_results.push_back(benchmark("spdlog: error", []() { spdlog::error("test"); }));

    // ========== Print Results ==========
    std::cout << "\n=== DETAILED RESULTS ===\n\n";
    std::cout << std::left << std::setw(40) << "Benchmark" << " | " << std::setw(8) << "Avg"
              << " | " << std::setw(8) << "P50"
              << " | " << std::setw(8) << "P95"
              << " | " << std::setw(8) << "P99"
              << " | " << std::setw(10) << "Ops/sec\n";
    std::cout << std::string(95, '-') << "\n";

    for (size_t i = 0; i < echo_results.size(); ++i) {
        print_result(echo_results[i]);
        print_result(spdlog_results[i]);
        print_comparison(echo_results[i], spdlog_results[i]);
        std::cout << "\n";
    }

    // ========== Summary ==========
    std::cout << "\n=== SUMMARY ===\n\n";

    double echo_total = 0, spdlog_total = 0;
    int echo_wins = 0, spdlog_wins = 0;

    for (size_t i = 0; i < echo_results.size(); ++i) {
        echo_total += echo_results[i].avg_ns;
        spdlog_total += spdlog_results[i].avg_ns;

        if (echo_results[i].avg_ns < spdlog_results[i].avg_ns) {
            echo_wins++;
        } else {
            spdlog_wins++;
        }
    }

    double echo_avg = echo_total / echo_results.size();
    double spdlog_avg = spdlog_total / spdlog_results.size();

    std::cout << "Total benchmarks: " << echo_results.size() << "\n";
    std::cout << "Echo wins: " << echo_wins << "\n";
    std::cout << "Spdlog wins: " << spdlog_wins << "\n\n";

    std::cout << "Average latency across all tests:\n";
    std::cout << "  Echo:   " << std::fixed << std::setprecision(2) << echo_avg << " ns\n";
    std::cout << "  Spdlog: " << spdlog_avg << " ns\n\n";

    if (echo_avg < spdlog_avg) {
        double speedup = spdlog_avg / echo_avg;
        std::cout << "Echo is " << speedup << "x faster on average (" << ((speedup - 1.0) * 100.0) << "% faster)\n";
    } else {
        double speedup = echo_avg / spdlog_avg;
        std::cout << "Spdlog is " << speedup << "x faster on average (" << ((speedup - 1.0) * 100.0) << "% faster)\n";
    }

    std::cout << "\nNote: All benchmarks use null sinks to isolate logging overhead\n";
    std::cout << "Speedup = spdlog_time / echo_time (>1.0 means echo is faster)\n";

    return 0;
}
