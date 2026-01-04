// Test simple echo() function without log levels
#include <echo/echo.hpp>

int main() {
    // Simple echo - always prints, no log level
    echo("Simple message");
    echo("Red text").red();
    echo("Green text").green();
    echo("Blue text").blue();

    // With modifiers
    echo("Bold text").bold();
    echo("Italic text").italic();
    echo("Bold + Italic").bold().italic();

    // Custom colors
    echo("Custom hex color").hex("#FF1493");
    echo("Custom RGB color").rgb(255, 165, 0);

    // Complex chaining
    echo("Everything combined!").hex("#00FFFF").bold().italic().underline();

    // String concatenation
    std::string hello = "Hello";
    std::string world = "World";
    echo(hello + " " + world).cyan();

    // Compare with log levels (these might not show depending on level)
    echo::debug("This is a debug message");
    echo::info("This is an info message");

    // But echo() always shows
    echo("This ALWAYS shows, regardless of log level!").magenta().bold();

    return 0;
}
