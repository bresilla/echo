// Test .inplace() functionality
#include <chrono>
#include <echo/echo.hpp>
#include <thread>

int main() {
    echo("=== Testing .inplace() ===\n");

    // Test 1: Simple counter
    echo("\n--- Test 1: Counter (updates in place) ---");
    for (int i = 0; i <= 10; i++) {
        echo::info("Counter: ", i).inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    echo(""); // New line after inplace

    // Test 2: Progress indicator
    echo("\n--- Test 2: Progress indicator ---");
    for (int i = 0; i <= 100; i += 5) {
        echo::info("Progress: ", i, "%").cyan().inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    echo("");

    // Test 3: Status updates with colors
    echo("\n--- Test 3: Status updates with colors ---");
    echo::info("Initializing...").yellow().inplace();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    echo::info("Loading data...").cyan().inplace();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    echo::info("Processing...").magenta().inplace();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    echo::info("Complete!").green().bold();

    // Test 4: Simple echo with inplace
    echo("\n--- Test 4: Simple echo() with .inplace() ---");
    for (int i = 0; i < 5; i++) {
        echo("Loading", std::string(i + 1, '.'), " ", i + 1, "/5").hex("#FF1493").inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    echo("");

    // Test 5: Countdown
    echo("\n--- Test 5: Countdown ---");
    for (int i = 10; i >= 0; i--) {
        if (i > 3) {
            echo::warn("Starting in ", i, " seconds...").inplace();
        } else if (i > 0) {
            echo::error("Starting in ", i, " seconds...").red().bold().inplace();
        } else {
            echo::info("GO!").green().bold();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Test 6: Spinner-like effect
    echo("\n--- Test 6: Spinner effect ---");
    const char *spinner[] = {"|", "/", "-", "\\"};
    for (int i = 0; i < 20; i++) {
        echo("Processing ", spinner[i % 4]).cyan().inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    echo("Processing complete!").green();

    echo("\n=== All tests complete ===");

    return 0;
}
