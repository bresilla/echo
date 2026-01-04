// Test .once() functionality
#include <echo/echo.hpp>

int main() {
    echo("=== Testing .once() in loops ===");

    // Test 1: Simple loop - should print only once
    echo("\n--- Test 1: Simple loop (prints once) ---");
    for (int i = 0; i < 5; i++) {
        echo::info("Loop iteration: ", i).once();
    }

    // Test 2: Without .once() - prints every time
    echo("\n--- Test 2: Without .once() (prints 5 times) ---");
    for (int i = 0; i < 5; i++) {
        echo::info("Loop iteration: ", i);
    }

    // Test 3: Multiple .once() calls at different locations
    echo("\n--- Test 3: Multiple .once() at different locations ---");
    for (int i = 0; i < 3; i++) {
        echo::info("First once call").once();
        echo::warn("Second once call").once();
        echo::error("Third once call").once();
    }

    // Test 4: .once() with colors and modifiers
    echo("\n--- Test 4: .once() with colors ---");
    for (int i = 0; i < 5; i++) {
        echo::info("Red and bold, once!").red().bold().once();
        echo::info("Cyan and italic, once!").cyan().italic().once();
    }

    // Test 5: Simple echo() with .once()
    echo("\n--- Test 5: Simple echo() with .once() ---");
    for (int i = 0; i < 5; i++) {
        echo("Simple echo, prints once").hex("#FF1493").once();
    }

    // Test 6: Nested loops - each location prints once
    echo("\n--- Test 6: Nested loops ---");
    for (int i = 0; i < 2; i++) {
        echo::info("Outer loop: ", i).once();
        for (int j = 0; j < 3; j++) {
            echo::debug("  Inner loop: ", j).once();
        }
    }

    echo("\n=== All tests complete ===");

    return 0;
}
