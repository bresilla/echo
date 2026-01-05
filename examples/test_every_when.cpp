/**
 * @file test_every_when.cpp
 * @brief Test .every() and .when() functionality
 *
 * Demonstrates:
 *   - .every(ms) - rate-limited printing (at most once per interval)
 *   - .when(condition) - conditional printing
 *   - Combining both with other fluent methods
 */

#include <echo/echo.hpp>
#include <thread>

int main() {
    echo("=== Testing .every() and .when() ===\n");

    // -------------------------------------------------------------------------
    // Test 1: .when() with modulo condition
    // -------------------------------------------------------------------------
    echo("--- Test 1: .when() with i % 10 == 0 ---");
    for (int i = 0; i <= 50; i++) {
        echo::info("Iteration ", i, " (divisible by 10)").when(i % 10 == 0);
    }

    // -------------------------------------------------------------------------
    // Test 2: .when() with various conditions
    // -------------------------------------------------------------------------
    echo("\n--- Test 2: .when() with various conditions ---");
    for (int i = 0; i < 20; i++) {
        echo::debug("Even number: ", i).when(i % 2 == 0).cyan();
        echo::warn("Greater than 15: ", i).when(i > 15).yellow();
    }

    // -------------------------------------------------------------------------
    // Test 3: .every() rate limiting
    // -------------------------------------------------------------------------
    echo("\n--- Test 3: .every(500) - prints at most every 500ms ---");
    echo("Running tight loop for ~2 seconds...");

    auto start = std::chrono::steady_clock::now();
    int iteration = 0;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed > 2000)
            break;

        echo::info("Loop iteration ", iteration, " at ", elapsed, "ms").every(500);
        iteration++;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    echo("\nTotal iterations: ", iteration, " (but only ~4-5 prints due to rate limiting)");

    // -------------------------------------------------------------------------
    // Test 4: Combining .every() and .when()
    // -------------------------------------------------------------------------
    echo("\n--- Test 4: Combining .every(300) and .when(i % 5 == 0) ---");
    echo("Running for ~1.5 seconds...");

    start = std::chrono::steady_clock::now();
    iteration = 0;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed > 1500)
            break;

        // Only prints if: iteration is divisible by 5 AND at least 300ms since last print
        echo::info("Iteration ", iteration, " (div by 5, rate-limited)").when(iteration % 5 == 0).every(300).green();

        iteration++;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // -------------------------------------------------------------------------
    // Test 5: .when() with simple echo()
    // -------------------------------------------------------------------------
    echo("\n--- Test 5: Simple echo() with .when() ---");
    for (int i = 0; i < 10; i++) {
        echo("Value ", i, " is prime-ish").when(i == 2 || i == 3 || i == 5 || i == 7).hex("#FF69B4").bold();
    }

    // -------------------------------------------------------------------------
    // Test 6: Multiple .every() at different locations
    // -------------------------------------------------------------------------
    echo("\n--- Test 6: Multiple .every() with different intervals ---");
    echo("Running for ~2 seconds...");

    start = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed > 2000)
            break;

        echo::info("Fast update (every 200ms)").every(200).cyan();
        echo::warn("Slow update (every 700ms)").every(700).yellow();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // -------------------------------------------------------------------------
    // Test 7: .when() with false condition (should not print)
    // -------------------------------------------------------------------------
    echo("\n--- Test 7: .when(false) should not print ---");
    echo::error("This should NOT appear").when(false);
    echo::info("This SHOULD appear").when(true);

    echo("\n=== All tests complete ===");

    return 0;
}
