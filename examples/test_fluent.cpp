// Test fluent interface with color methods
#include <echo/echo.hpp>

int main() {
    // Basic usage
    echo::info("Normal message");

    // With colors
    echo::info("This is red").red();
    echo::info("This is green").green();
    echo::info("This is yellow").yellow();
    echo::info("This is blue").blue();
    echo::info("This is magenta").magenta();
    echo::info("This is cyan").cyan();

    // Different log levels with colors
    echo::debug("Debug in gray").gray();
    echo::warn("Warning in bold red").red().bold();
    echo::error("Error in bold yellow").yellow().bold();

    // String concatenation
    std::string cool = "cool";
    std::string stuff = "stuff";
    echo::info(cool + " " + stuff).cyan();

    return 0;
}
