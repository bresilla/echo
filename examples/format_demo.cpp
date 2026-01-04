/**
 * @file format_demo.cpp
 * @brief Demonstrates echo::format::String color and style methods
 */

#define LOGLEVEL Trace
#include <echo/echo.hpp>
#include <echo/format.hpp>

#include <iostream>

using echo::format::String;

void demo_foreground_colors() {
    echo::info("=== FOREGROUND COLORS ===\n");

    std::cout << String("Black text").black() << "\n";
    std::cout << String("Red text").red() << "\n";
    std::cout << String("Green text").green() << "\n";
    std::cout << String("Yellow text").yellow() << "\n";
    std::cout << String("Blue text").blue() << "\n";
    std::cout << String("Magenta text").magenta() << "\n";
    std::cout << String("Cyan text").cyan() << "\n";
    std::cout << String("White text").white() << "\n";
    std::cout << "\n";
}

void demo_bright_foreground_colors() {
    echo::info("=== BRIGHT FOREGROUND COLORS ===\n");

    std::cout << String("Bright black text").bright_black() << "\n";
    std::cout << String("Bright red text").bright_red() << "\n";
    std::cout << String("Bright green text").bright_green() << "\n";
    std::cout << String("Bright yellow text").bright_yellow() << "\n";
    std::cout << String("Bright blue text").bright_blue() << "\n";
    std::cout << String("Bright magenta text").bright_magenta() << "\n";
    std::cout << String("Bright cyan text").bright_cyan() << "\n";
    std::cout << String("Bright white text").bright_white() << "\n";
    std::cout << "\n";
}

void demo_background_colors() {
    echo::info("=== BACKGROUND COLORS ===\n");

    std::cout << String("Black background").on_black().white() << "\n";
    std::cout << String("Red background").on_red().white() << "\n";
    std::cout << String("Green background").on_green().white() << "\n";
    std::cout << String("Yellow background").on_yellow().black() << "\n";
    std::cout << String("Blue background").on_blue().white() << "\n";
    std::cout << String("Magenta background").on_magenta().white() << "\n";
    std::cout << String("Cyan background").on_cyan().black() << "\n";
    std::cout << String("White background").on_white().black() << "\n";
    std::cout << "\n";
}

void demo_bright_background_colors() {
    echo::info("=== BRIGHT BACKGROUND COLORS ===\n");

    std::cout << String("Bright black background").on_bright_black().white() << "\n";
    std::cout << String("Bright red background").on_bright_red().white() << "\n";
    std::cout << String("Bright green background").on_bright_green().black() << "\n";
    std::cout << String("Bright yellow background").on_bright_yellow().black() << "\n";
    std::cout << String("Bright blue background").on_bright_blue().white() << "\n";
    std::cout << String("Bright magenta background").on_bright_magenta().black() << "\n";
    std::cout << String("Bright cyan background").on_bright_cyan().black() << "\n";
    std::cout << String("Bright white background").on_bright_white().black() << "\n";
    std::cout << "\n";
}

void demo_rgb_hex_colors() {
    echo::info("=== RGB & HEX COLORS ===\n");

    std::cout << String("RGB foreground (255, 100, 50)").fg(255, 100, 50) << "\n";
    std::cout << String("RGB background (50, 100, 255)").bg(50, 100, 255).white() << "\n";
    std::cout << String("HEX foreground #FF5733").fg("#FF5733") << "\n";
    std::cout << String("HEX background #3498DB").bg("#3498DB").white() << "\n";
    std::cout << String("Combined RGB fg + bg").fg(255, 215, 0).bg(139, 0, 139) << "\n";
    std::cout << "\n";
}

void demo_text_styles() {
    echo::info("=== TEXT STYLES ===\n");

    std::cout << String("Bold text").bold() << "\n";
    std::cout << String("Dim text").dim() << "\n";
    std::cout << String("Italic text").italic() << "\n";
    std::cout << String("Underline text").underline() << "\n";
    std::cout << String("Blink text").blink() << "\n";
    std::cout << String("Reverse text").reverse() << "\n";
    std::cout << String("Hidden text").hidden() << " (hidden)\n";
    std::cout << String("Strikethrough text").strikethrough() << "\n";
    std::cout << "\n";
}

void demo_chaining() {
    echo::info("=== METHOD CHAINING ===\n");

    std::cout << String("Red bold text").red().bold() << "\n";
    std::cout << String("Blue italic underline").blue().italic().underline() << "\n";
    std::cout << String("Green on yellow bold").green().on_yellow().bold() << "\n";
    std::cout << String("Magenta dim strikethrough").magenta().dim().strikethrough() << "\n";
    std::cout << String("Cyan on red bold italic").cyan().on_red().bold().italic() << "\n";
    std::cout << String("Custom RGB combo").fg(255, 0, 128).bg(0, 255, 128).bold().underline() << "\n";
    std::cout << "\n";
}

void demo_clear_normal() {
    echo::info("=== CLEAR & NORMAL ===\n");

    String text("Styled text");
    std::cout << "Original: " << text.red().bold() << "\n";
    std::cout << "After clear: " << text.clear() << "\n";

    String text2("Another styled text");
    std::cout << "Original: " << text2.blue().italic().on_yellow() << "\n";
    std::cout << "After normal: " << text2.normal() << "\n";
    std::cout << "\n";
}

void demo_conversion() {
    echo::info("=== CONVERSION METHODS ===\n");

    String styled("Hello World");
    styled.red().bold();

    std::cout << "to_string(): " << styled.to_string() << "\n";
    std::cout << "str() (raw): " << styled.str() << "\n";
    std::cout << "implicit conversion: " << styled << "\n";

    // Can use in string concatenation
    std::string result = "Prefix " + styled.to_string() + " Suffix";
    std::cout << "Concatenated: " << result << "\n";
    std::cout << "\n";
}

int main() {
    echo::info("╔════════════════════════════════════════╗");
    echo::info("║   Echo Format::String Color Demo      ║");
    echo::info("╚════════════════════════════════════════╝\n");

    demo_foreground_colors();
    demo_bright_foreground_colors();
    demo_background_colors();
    demo_bright_background_colors();
    demo_rgb_hex_colors();
    demo_text_styles();
    demo_chaining();
    demo_clear_normal();
    demo_conversion();

    echo::info("Demo complete!");
    return 0;
}
