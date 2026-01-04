// Test simple echo() function without log levels
#include <echo/echo.hpp>

int main() {
    // Simple echo - always prints, no log level
    echo::echo("Simple message");
    echo::echo("Red text").red();
    echo::echo("Green text").green();
    echo::echo("Blue text").blue();

    // With modifiers
    echo::echo("Bold text").bold();
    echo::echo("Italic text").italic();
    echo::echo("Bold + Italic").bold().italic();

    // Custom colors
    echo::echo("Custom hex color").hex("#FF1493");
    echo::echo("Custom RGB color").rgb(255, 165, 0);

    // Complex chaining
    echo::echo("Everything combined!").hex("#00FFFF").bold().italic().underline();

    // String concatenation
    std::string hello = "Hello";
    std::string world = "World";
    echo::echo(hello + " " + world).cyan();

    // Compare with log levels (these might not show depending on level)
    echo::debug("This is a debug message");
    echo::info("This is an info message");

    // But echo() always shows
    echo::echo("This ALWAYS shows, regardless of log level!").magenta().bold();

    return 0;
}
