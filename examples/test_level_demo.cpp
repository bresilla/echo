// Comprehensive test of log level control
// Compile without any -DLOGLEVEL to test environment variable support
#include <echo/echo.hpp>
#include <iostream>

void print_all_levels() {
    echo::trace("TRACE message");
    echo::debug("DEBUG message");
    echo::info("INFO message");
    echo::warn("WARN message");
    echo::error("ERROR message");
    echo::critical("CRITICAL message");
}

int main() {
    std::cout << "\n=== Testing Environment Variable Support ===\n";
    std::cout << "Current effective level: " << static_cast<int>(echo::get_level()) << "\n\n";

    print_all_levels();

    std::cout << "\n=== Testing Runtime API ===\n";
    std::cout << "Setting level to Warn via API...\n\n";
    echo::set_level(echo::Level::Warn);

    print_all_levels();

    return 0;
}
