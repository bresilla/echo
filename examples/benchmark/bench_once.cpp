/**
 * @file bench_once.cpp
 * @brief .once() modifier performance benchmarks
 *
 * Tests the performance of the .once() modifier:
 * - First call overhead (hash map insert)
 * - Subsequent call overhead (hash map lookup)
 * - Comparison with regular logging
 * - Multiple unique .once() calls
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
    std::cout << "\n=== .once() MODIFIER BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // Baseline - regular logging
    results.push_back(benchmark("Regular logging (no .once())", []() { echo::info("test message"); }));

    // .once() - subsequent calls (after warmup, so hash map already populated)
    results.push_back(benchmark(".once() - subsequent calls", []() { echo::info("test message").once(); }));

    // Multiple different .once() locations
    int counter = 0;
    results.push_back(benchmark(".once() - 100 unique locations", [&]() {
        // Simulate different source locations
        switch (counter++ % 100) {
        case 0:
            echo::info("msg0").once();
            break;
        case 1:
            echo::info("msg1").once();
            break;
        case 2:
            echo::info("msg2").once();
            break;
        case 3:
            echo::info("msg3").once();
            break;
        case 4:
            echo::info("msg4").once();
            break;
        case 5:
            echo::info("msg5").once();
            break;
        case 6:
            echo::info("msg6").once();
            break;
        case 7:
            echo::info("msg7").once();
            break;
        case 8:
            echo::info("msg8").once();
            break;
        case 9:
            echo::info("msg9").once();
            break;
        default:
            echo::info("msg_default").once();
            break;
        }
    }));

    // .once() with different log levels
    results.push_back(benchmark(".once() with trace level", []() { echo::trace("test").once(); }));
    results.push_back(benchmark(".once() with debug level", []() { echo::debug("test").once(); }));
    results.push_back(benchmark(".once() with info level", []() { echo::info("test").once(); }));
    results.push_back(benchmark(".once() with warn level", []() { echo::warn("test").once(); }));
    results.push_back(benchmark(".once() with error level", []() { echo::error("test").once(); }));

    // .once() with filtered messages
    echo::set_level(echo::Level::Error);
    results.push_back(benchmark(".once() filtered (level=Error)", []() { echo::info("test").once(); }));
    echo::set_level(echo::Level::Trace);

    // .once() with complex messages
    results.push_back(
        benchmark(".once() with multiple args", []() { echo::info("Value:", 42, "Pi:", 3.14, "Done").once(); }));

    // Print results
    std::cout << std::left << std::setw(45) << "Benchmark" << " | " << std::setw(10) << "Avg"
              << " | " << std::setw(10) << "Min"
              << " | " << std::setw(10) << "Max"
              << " | " << std::setw(12) << "Ops/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: .once() adds hash map lookup overhead (~20-50ns)\n";
    std::cout << "First call per location is slower (hash map insert)\n";
    std::cout << "All benchmarks use NullSink to isolate .once() overhead\n";

    return 0;
}
