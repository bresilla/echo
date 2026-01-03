// Test that ECHOLEVEL works
#define ECHOLEVEL Debug
#include <echo/echo.hpp>

int main() {
    echo::trace("This should NOT appear (level is Debug)");
    echo::debug("This SHOULD appear (level is Debug)");
    echo::info("This SHOULD appear");
    echo::warn("This SHOULD appear");
    return 0;
}
