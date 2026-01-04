// Demonstration of banner and separator functionality
#define LOGLEVEL Trace
#include <echo/banner.hpp>

int main() {
    // Large banner for application start
    echo::banner("ECHO BANNER DEMO");
    echo::info("This example demonstrates all banner and separator functions");

    // ========================================
    // SEPARATORS
    // ========================================
    echo::separator("SEPARATORS", '=');

    // Basic separator - full width line
    echo::separator();
    echo::info("Full-width separator above");

    // Separator with text - centered
    echo::separator("Application Startup");
    echo::info("Initializing components...");
    echo::debug("Loading configuration");
    echo::info("Startup complete");

    // Separator with custom character
    echo::separator("IMPORTANT SECTION", '=');
    echo::warn("This section contains critical information");

    // Different separator styles
    echo::separator("Configuration", '*');
    echo::info("Host: localhost");
    echo::info("Port: 8080");

    // ========================================
    // BOXES
    // ========================================
    echo::separator("BOXES", '=');

    // All box styles
    echo::box("Single Line Box", echo::BoxStyle::Single);
    echo::box("Double Line Box", echo::BoxStyle::Double);
    echo::box("Rounded Corner Box", echo::BoxStyle::Rounded);
    echo::box("Heavy Line Box", echo::BoxStyle::Heavy);
    echo::box("Dashed Line Box", echo::BoxStyle::Dashed);
    echo::box("ASCII Box (Compatible)", echo::BoxStyle::ASCII);

    // ========================================
    // HEADERS
    // ========================================
    echo::separator("HEADERS", '=');

    echo::header("Application Configuration");
    echo::info("Host: localhost");
    echo::info("Port: 8080");
    echo::info("Debug mode: enabled");

    echo::header("User Activity");
    echo::info("User 'alice' logged in");
    echo::info("User 'bob' logged in");

    // ========================================
    // TITLES
    // ========================================
    echo::separator("TITLES", '=');

    echo::title("My Application");
    echo::info("Default title with '=' borders");

    echo::title("Section Header", '-');
    echo::info("Title with '-' borders");

    echo::title("Important Notice", '*');
    echo::info("Title with '*' borders");

    // ========================================
    // BANNERS
    // ========================================
    echo::separator("BANNERS", '=');

    echo::banner("WELCOME", echo::BoxStyle::Heavy);
    echo::info("Heavy style banner (default)");

    echo::banner("SUCCESS", echo::BoxStyle::Double);
    echo::info("Double line banner");

    echo::banner("INFO", echo::BoxStyle::Rounded);
    echo::info("Rounded corner banner");

    echo::banner("WARNING", echo::BoxStyle::Dashed);
    echo::info("Dashed line banner");

    echo::banner("COMPATIBLE", echo::BoxStyle::ASCII);
    echo::info("ASCII banner for compatibility");

    // ========================================
    // COMBINING WITH STRUCTURED LOGGING
    // ========================================
    echo::separator("STRUCTURED LOGGING", '=');
    echo::header("System Metrics");
    echo::info("CPU: ", echo::kv("usage", "45%", "cores", 8));
    echo::info("Memory: ", echo::kv("used", "4.2GB", "total", "16GB"));
    echo::info("Disk: ", echo::kv("free", "250GB", "total", "500GB"));

    // Final banner
    echo::separator();
    echo::banner("DEMO COMPLETE");

    return 0;
}
