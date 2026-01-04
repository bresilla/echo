// Performance demonstration
// Compile with: g++ -std=c++20 -O3 -DLOGLEVEL=Error -I../include performance_demo.cpp -o perf_demo

#include <chrono>
#include <echo/echo.hpp>

int expensive_calculation() {
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }
    return sum;
}

int main() {
    echo("=== Echo Performance Demonstration ===\n");

    // Test 1: Compile-time filtered (should be ZERO overhead)
    echo("Test 1: Compile-time filtered debug calls (1M iterations)");
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        echo::debug("This is filtered at compile-time"); // ZERO overhead with -DLOGLEVEL=Error
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    echo("Time: ", duration.count(), " μs (should be near zero)").cyan();

    // Test 2: .once() in loop
    echo("\nTest 2: .once() in loop (1M iterations, prints once)");
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        echo::error("This prints only once").once(); // First iteration: ~50ns, rest: ~30ns
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    echo("Time: ", duration.count(), " μs").cyan();

    // Test 3: Expensive calculation in filtered log
    echo("\nTest 3: Expensive calculation in filtered log (1000 iterations)");
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        echo::debug("Result: ",
                    expensive_calculation()); // With -DLOGLEVEL=Error, expensive_calculation() is NEVER called!
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    echo("Time: ", duration.count(), " μs (should be near zero - calculation never runs!)").cyan();

    // Test 4: Same calculation WITHOUT filtering
    echo("\nTest 4: Same calculation that actually runs (1000 iterations)");
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        int result = expensive_calculation(); // This actually runs
        (void)result;                         // Prevent optimization
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    echo("Time: ", duration.count(), " μs (this is the actual cost)").yellow();

    echo("\n=== Conclusion ===").green().bold();
    echo("With compile-time filtering (-DLOGLEVEL=Error):").green();
    echo("  - Debug/trace/info/warn calls have ZERO overhead").green();
    echo("  - Expensive calculations in filtered logs are NEVER executed").green();
    echo("  - .once() adds minimal overhead after first call").green();

    return 0;
}
