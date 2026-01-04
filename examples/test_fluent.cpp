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

    // Custom hex colors
    echo::info("Custom hex color #FF5733").hex("#FF5733");
    echo::info("Custom hex color FF6B9D").hex("FF6B9D");
    echo::info("Custom hex color #00D9FF").hex("#00D9FF");

    // Custom RGB colors
    echo::info("Custom RGB (255, 87, 51)").rgb(255, 87, 51);
    echo::info("Custom RGB (107, 255, 157)").rgb(107, 255, 157);
    echo::info("Custom RGB (255, 215, 0)").rgb(255, 215, 0);

    // Combining with bold
    echo::warn("Bold custom color").hex("#9D00FF").bold();

    // Text modifiers
    echo::info("Bold text").bold();
    echo::info("Italic text").italic();
    echo::info("Underlined text").underline();
    echo::info("Bold + Italic").bold().italic();
    echo::info("Red + Bold + Italic").red().bold().italic();
    echo::info("Custom color + Bold + Italic + Underline").hex("#FF1493").bold().italic().underline();

    return 0;
}
