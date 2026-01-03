// Test environment variable support (no compile-time level set)
#include <echo/echo.hpp>

int main() {
    echo::trace("TRACE message");
    echo::debug("DEBUG message");
    echo::info("INFO message");
    echo::warn("WARN message");
    echo::error("ERROR message");
    echo::critical("CRITICAL message");
    return 0;
}
