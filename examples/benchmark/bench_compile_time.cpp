/**
 * @file bench_compile_time.cpp
 * @brief Compile-time vs runtime filtering benchmarks
 *
 * Tests the performance difference between:
 * - Compile-time log level filtering (LOGLEVEL macro)
 * - Runtime log level filtering
 * - No filtering
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
    std::cout << std::left << std::setw(50) << r.name << " | " << std::right << std::setw(10) << std::fixed
              << std::setprecision(2) << r.avg_ns << " ns | " << std::setw(10) << r.min_ns << " ns | " << std::setw(10)
              << r.max_ns << " ns | " << std::setw(12) << std::setprecision(0) << r.ops_per_sec << " ops/s\n";
}

int main() {
    std::cout << "\n=== COMPILE-TIME vs RUNTIME FILTERING BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

#ifdef LOGLEVEL
    std::cout << "Compile-time log level: " << LOGLEVEL << "\n\n";

    // With compile-time filtering, filtered messages should have near-zero overhead
    results.push_back(benchmark("trace (compile-time filtered)", []() { echo::trace("test"); }));
    results.push_back(benchmark("debug (compile-time filtered)", []() { echo::debug("test"); }));
    results.push_back(benchmark("info (compile-time filtered)", []() { echo::info("test"); }));
    results.push_back(benchmark("warn (compile-time filtered)", []() { echo::warn("test"); }));
    results.push_back(benchmark("error (compile-time filtered)", []() { echo::error("test"); }));
#else
    std::cout << "No compile-time log level set (runtime filtering only)\n\n";

    // Runtime filtering - all levels
    echo::set_level(echo::Level::Trace);
    results.push_back(benchmark("trace (runtime, level=Trace)", []() { echo::trace("test"); }));
    results.push_back(benchmark("debug (runtime, level=Trace)", []() { echo::debug("test"); }));
    results.push_back(benchmark("info (runtime, level=Trace)", []() { echo::info("test"); }));
    results.push_back(benchmark("warn (runtime, level=Trace)", []() { echo::warn("test"); }));
    results.push_back(benchmark("error (runtime, level=Trace)", []() { echo::error("test"); }));

    // Runtime filtering - filtered out
    echo::set_level(echo::Level::Error);
    results.push_back(benchmark("trace (runtime filtered, level=Error)", []() { echo::trace("test"); }));
    results.push_back(benchmark("debug (runtime filtered, level=Error)", []() { echo::debug("test"); }));
    results.push_back(benchmark("info (runtime filtered, level=Error)", []() { echo::info("test"); }));
    results.push_back(benchmark("warn (runtime filtered, level=Error)", []() { echo::warn("test"); }));
    results.push_back(benchmark("error (runtime passes, level=Error)", []() { echo::error("test"); }));

    // Reset
    echo::set_level(echo::Level::Info);
#endif

    // Baseline - no filtering at all (level=Trace)
    echo::set_level(echo::Level::Trace);
    results.push_back(benchmark("Baseline: no filtering (level=Trace)", []() { echo::info("test"); }));

    // Print results
    std::cout << std::left << std::setw(50) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(105, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

#ifdef LOGLEVEL
    std::cout << "\nNote: Compile-time filtering removes code entirely (zero overhead)\n";
    std::cout << "Rebuild without -DLOGLEVEL to test runtime filtering\n";
#else
    std::cout << "\nNote: Runtime filtering has small overhead (~5-10ns per check)\n";
    std::cout << "Rebuild with -DLOGLEVEL=Error to test compile-time filtering\n";
#endif

    return 0;
}
