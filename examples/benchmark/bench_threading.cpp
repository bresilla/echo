/**
 * @file bench_threading.cpp
 * @brief Multi-threading performance benchmarks
 *
 * Tests concurrent logging performance:
 * - Single thread baseline
 * - Multiple threads logging simultaneously
 * - Thread contention scenarios
 * - Scalability with thread count
 */

#include <echo/core/level.hpp>
#include <echo/core/proxy.hpp>
#include <echo/sinks/null_sink.hpp>
#include <echo/sinks/registry.hpp>

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono;

struct BenchResult {
    std::string name;
    size_t num_threads;
    size_t total_ops;
    double duration_ms;
    double ops_per_sec;
    double ops_per_thread_per_sec;
};

void print_result(const BenchResult &r) {
    std::cout << std::left << std::setw(30) << r.name << " | " << std::right << std::setw(8) << r.num_threads << " | "
              << std::setw(12) << r.total_ops << " | " << std::setw(10) << std::fixed << std::setprecision(2)
              << r.duration_ms << " ms | " << std::setw(12) << std::setprecision(0) << r.ops_per_sec << " | "
              << std::setw(12) << r.ops_per_thread_per_sec << "\n";
}

BenchResult benchmark_threads(const std::string &name, size_t num_threads, size_t ops_per_thread) {
    std::atomic<size_t> ready_count{0};
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    auto start_time = high_resolution_clock::now();

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            ready_count++;
            while (!start.load())
                ; // Spin wait for synchronized start

            for (size_t j = 0; j < ops_per_thread; ++j) {
                echo::info("Thread ", i, " message ", j);
            }
        });
    }

    // Wait for all threads to be ready
    while (ready_count.load() < num_threads) {
        std::this_thread::yield();
    }

    // Start all threads simultaneously
    start.store(true);

    // Wait for completion
    for (auto &t : threads) {
        t.join();
    }

    auto end_time = high_resolution_clock::now();
    double duration_ms = duration_cast<microseconds>(end_time - start_time).count() / 1000.0;

    size_t total_ops = num_threads * ops_per_thread;
    double ops_per_sec = (total_ops / duration_ms) * 1000.0;
    double ops_per_thread_per_sec = ops_per_sec / num_threads;

    return {name, num_threads, total_ops, duration_ms, ops_per_sec, ops_per_thread_per_sec};
}

int main() {
    std::cout << "\n=== THREADING PERFORMANCE BENCHMARKS ===\n\n";

    // Use null sink for fair benchmarking
    echo::clear_sinks();
    echo::add_sink(std::make_shared<echo::NullSink>());

    std::vector<BenchResult> results;

    // Test different thread counts
    const size_t ops_per_thread = 10000;

    results.push_back(benchmark_threads("Single thread", 1, ops_per_thread));
    results.push_back(benchmark_threads("2 threads", 2, ops_per_thread));
    results.push_back(benchmark_threads("4 threads", 4, ops_per_thread));
    results.push_back(benchmark_threads("8 threads", 8, ops_per_thread));
    results.push_back(benchmark_threads("16 threads", 16, ops_per_thread));

    // High contention test
    results.push_back(benchmark_threads("32 threads (high contention)", 32, ops_per_thread / 2));

    // Print results
    std::cout << std::left << std::setw(30) << "Benchmark" << " | " << std::setw(8) << "Threads"
              << " | " << std::setw(12) << "Total Ops"
              << " | " << std::setw(10) << "Duration"
              << " | " << std::setw(12) << "Ops/sec"
              << " | " << std::setw(12) << "Ops/t/sec\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto &r : results) {
        print_result(r);
    }

    std::cout << "\nNote: All benchmarks use NullSink to isolate threading overhead\n";
    std::cout << "Ops/t/sec = Operations per thread per second\n";

    return 0;
}
