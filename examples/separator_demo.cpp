// Demonstration of separator functionality
#define LOGLEVEL Trace
#include <echo/banner.hpp>

int main() {
    echo::info("Echo Separator Demo");
    echo::info("This example demonstrates the separator function");

    // Basic separator - full width line
    echo::separator();
    echo::info("Full-width separator above");

    // Separator with text - centered
    echo::separator("Application Startup");
    echo::info("Initializing components...");
    echo::debug("Loading configuration");
    echo::debug("Connecting to database");
    echo::info("Startup complete");

    // Separator with custom character
    echo::separator("IMPORTANT SECTION", '=');
    echo::warn("This section contains critical information");
    echo::error("Error handling is enabled");
    echo::critical("System monitoring active");

    // Different separator styles
    echo::separator("Configuration", '*');
    echo::info("Host: localhost");
    echo::info("Port: 8080");
    echo::info("Debug mode: enabled");

    echo::separator("User Activity", '#');
    echo::info("User 'alice' logged in");
    echo::info("User 'bob' logged in");
    echo::info("User 'charlie' logged in");

    // Combining with structured logging
    echo::separator("System Metrics", '~');
    echo::info("CPU: ", echo::kv("usage", "45%", "cores", 8));
    echo::info("Memory: ", echo::kv("used", "4.2GB", "total", "16GB"));
    echo::info("Disk: ", echo::kv("free", "250GB", "total", "500GB"));

    // Short text
    echo::separator("End");

    // Very short text
    echo::separator("!");

    // Final separator
    echo::separator();
    echo::info("Demo complete");

    return 0;
}
