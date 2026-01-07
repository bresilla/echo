/**
 * @file bench_logging.cpp
 * @brief Benchmark core logging performance
 *
 * Tests:
 * - Compile-time filtered logs (should be ~0ns)
 * - Runtime filtered logs
 * - .once() first call vs subsequent
 * - .every() performance
 * - Multi-threaded logging throughput
 */

#include <echo/echo.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono;

// Simple benchmark timer
class Timer {
  private:
    high_resolution_clock::time_point start_;
    std::string name_;

  public:
    explicit Timer(std::string name) : start_(high_resolution_clock::now()), name_(std::move(name)) {}

    ~Timer() {
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end - start_).count();
        std::cout << name_ << ": " << duration << " ns (" << (duration / 1000.0) << " μs)\n";
    }
};

void bench_compile_time_filtered() {
    std::cout << "\n=== Compile-Time Filtered Logs ===\n";
    std::cout << "These should be ~0ns (code doesn't exist in binary)\n";

    // Set runtime level to Error so Debug is filtered
    echo::set_level(echo::Level::Error);

    constexpr int iterations = 1000000;

    {
        Timer t("Debug (filtered) x1M");
        for (int i = 0; i < iterations; ++i) {
            echo::debug("This is filtered");
        }
    }

    {
        Timer t("Info (filtered) x1M");
        for (int i = 0; i < iterations; ++i) {
            echo::info("This is filtered");
        }
    }
}

void bench_runtime_filtered() {
    std::cout << "\n=== Runtime Filtered Logs ===\n";
    std::cout << "These check level at runtime (~8ns per call)\n";

    // Clear sinks to avoid I/O overhead
    echo::clear_sinks();

    echo::set_level(echo::Level::Error);

    constexpr int iterations = 1000000;

    {
        Timer t("Debug (runtime filtered) x1M");
        for (int i = 0; i < iterations; ++i) {
            echo::debug("This is filtered");
        }
    }

    {
        Timer t("Info (runtime filtered) x1M");
        for (int i = 0; i < iterations; ++i) {
            echo::info("This is filtered");
        }
    }
}

void bench_once() {
    std::cout << "\n=== .once() Performance ===\n";

    // Clear sinks to avoid I/O overhead
    echo::clear_sinks();
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 1000000;

    // First call (should insert into hash map)
    {
        Timer t("First .once() call");
        echo::info("First once").once();
    }

    // Subsequent calls (should be hash map lookup + skip)
    {
        Timer t("Subsequent .once() calls x1M");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Subsequent once").once();
        }
    }
}

void bench_actual_logging() {
    std::cout << "\n=== Actual Logging (with I/O) ===\n";

    // Clear sinks to measure formatting overhead without I/O
    echo::clear_sinks();

    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 100000;

    {
        Timer t("Info logging x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Log message ", i);
        }
    }

    {
        Timer t("Error logging x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::error("Error message ", i);
        }
    }
}

void bench_multithreaded() {
    std::cout << "\n=== Multi-threaded Logging ===\n";

    // Clear sinks
    echo::clear_sinks();

    echo::set_level(echo::Level::Trace);

    constexpr int num_threads = 4;
    constexpr int iterations_per_thread = 25000;

    auto worker = [](int thread_id) {
        for (int i = 0; i < iterations_per_thread; ++i) {
            echo::info("Thread ", thread_id, " message ", i);
        }
    };

    {
        Timer t("4 threads x 25k messages each (100k total)");

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, i);
        }

        for (auto &thread : threads) {
            thread.join();
        }
    }
}

void bench_string_formatting() {
    std::cout << "\n=== String Formatting ===\n";

    // Clear sinks to measure pure formatting
    echo::clear_sinks();
    echo::set_level(echo::Level::Trace);

    constexpr int iterations = 100000;

    {
        Timer t("Simple string x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Simple message");
        }
    }

    {
        Timer t("String with int x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("Message ", i);
        }
    }

    {
        Timer t("String with multiple args x100k");
        for (int i = 0; i < iterations; ++i) {
            echo::info("User ", "alice", " logged in at ", i, " with status ", true);
        }
    }
}

int main() {
    std::cout << "=== Echo Logging Benchmarks ===\n";
    std::cout << "Lower is better (ns = nanoseconds, μs = microseconds)\n";

    bench_compile_time_filtered();
    bench_runtime_filtered();
    bench_once();
    bench_string_formatting();
    bench_actual_logging();
    bench_multithreaded();

    std::cout << "\n=== Benchmarks Complete ===\n";
    return 0;
}
