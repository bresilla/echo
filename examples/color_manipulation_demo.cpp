/**
 * @file color_manipulation_demo.cpp
 * @brief Demonstrates RGB color manipulation functions
 */

#define LOGLEVEL Trace
#include <echo/color.hpp>
#include <echo/echo.hpp>

#include <iostream>

void demo_conversions() {
    echo::info("=== COLOR CONVERSIONS ===\n");

    auto red = echo::color::from_hex("#FF0000");
    echo::info("Red RGB: ", red.r, ", ", red.g, ", ", red.b);
    echo::info("Red HEX: ", echo::color::to_hex(red));
    echo::info("Red ANSI: ", echo::color::to_ansi(red), "COLORED TEXT", "\033[0m");
    std::cout << "\n";
}

void demo_mixing() {
    echo::info("=== COLOR MIXING ===\n");

    auto red = echo::color::from_hex("#FF0000");
    auto blue = echo::color::from_hex("#0000FF");

    auto mixed = echo::color::mix(red, blue);
    echo::info("Red + Blue (mix): ", echo::color::to_hex(mixed));

    auto added = echo::color::add(red, blue);
    echo::info("Red + Blue (add): ", echo::color::to_hex(added));

    auto multiplied = echo::color::multiply(red, blue);
    echo::info("Red * Blue (multiply): ", echo::color::to_hex(multiplied));

    auto screened = echo::color::screen(red, blue);
    echo::info("Red + Blue (screen): ", echo::color::to_hex(screened));
    std::cout << "\n";
}

void demo_adjustments() {
    echo::info("=== COLOR ADJUSTMENTS ===\n");

    auto orange = echo::color::from_hex("#FF8800");
    echo::info("Original: ", echo::color::to_hex(orange));

    auto lighter = echo::color::lighten(orange, 0.3f);
    echo::info("Lightened 30%: ", echo::color::to_hex(lighter));

    auto darker = echo::color::darken(orange, 0.3f);
    echo::info("Darkened 30%: ", echo::color::to_hex(darker));

    auto brightened = echo::color::brighten(orange, 50);
    echo::info("Brightened +50: ", echo::color::to_hex(brightened));

    auto dimmed = echo::color::dim(orange, 50);
    echo::info("Dimmed -50: ", echo::color::to_hex(dimmed));

    auto saturated = echo::color::saturate(orange, 0.5f);
    echo::info("Saturated: ", echo::color::to_hex(saturated));

    auto desaturated = echo::color::desaturate(orange, 0.5f);
    echo::info("Desaturated: ", echo::color::to_hex(desaturated));

    auto inverted = echo::color::invert(orange);
    echo::info("Inverted: ", echo::color::to_hex(inverted));

    auto gray = echo::color::grayscale(orange);
    echo::info("Grayscale: ", echo::color::to_hex(gray));
    std::cout << "\n";
}

void demo_analysis() {
    echo::info("=== COLOR ANALYSIS ===\n");

    auto red = echo::color::from_hex("#FF0000");
    auto blue = echo::color::from_hex("#0000FF");
    auto white = echo::color::from_hex("#FFFFFF");
    auto black = echo::color::from_hex("#000000");

    echo::info("Red luminance: ", echo::color::luminance(red));
    echo::info("Blue luminance: ", echo::color::luminance(blue));
    echo::info("Red is dark: ", echo::color::is_dark(red) ? "yes" : "no");
    echo::info("Red is light: ", echo::color::is_light(red) ? "yes" : "no");

    echo::info("Distance red-blue: ", echo::color::distance(red, blue));
    echo::info("Contrast white/black: ", echo::color::contrast_ratio(white, black));
    echo::info("Contrast red/blue: ", echo::color::contrast_ratio(red, blue));
    std::cout << "\n";
}

void demo_schemes() {
    echo::info("=== COLOR SCHEMES ===\n");

    auto base = echo::color::from_hex("#FF5733");
    echo::info("Base color: ", echo::color::to_hex(base));

    auto comp = echo::color::complementary(base);
    echo::info("Complementary: ", echo::color::to_hex(comp));

    auto triadic_colors = echo::color::triadic(base);
    echo::info("Triadic scheme:");
    for (const auto &c : triadic_colors) {
        echo::info("  - ", echo::color::to_hex(c));
    }

    auto tint_colors = echo::color::tints(base, 5);
    echo::info("Tints (lighter):");
    for (const auto &c : tint_colors) {
        echo::info("  - ", echo::color::to_hex(c));
    }

    auto shade_colors = echo::color::shades(base, 5);
    echo::info("Shades (darker):");
    for (const auto &c : shade_colors) {
        echo::info("  - ", echo::color::to_hex(c));
    }
    std::cout << "\n";
}

void demo_temperature() {
    echo::info("=== COLOR TEMPERATURE ===\n");

    auto cool = echo::color::from_temperature(-1.0f);
    echo::info("Cool (-1.0): ", echo::color::to_hex(cool));

    auto neutral = echo::color::from_temperature(0.0f);
    echo::info("Neutral (0.0): ", echo::color::to_hex(neutral));

    auto warm = echo::color::from_temperature(1.0f);
    echo::info("Warm (1.0): ", echo::color::to_hex(warm));
    std::cout << "\n";
}

void demo_random() {
    echo::info("=== RANDOM COLORS ===\n");

    for (int i = 0; i < 5; ++i) {
        auto random_color = echo::color::random();
        echo::info("Random ", i + 1, ": ", echo::color::to_hex(random_color));
    }
    std::cout << "\n";
}

int main() {
    echo::info("╔════════════════════════════════════════╗");
    echo::info("║   Echo Color Manipulation Demo        ║");
    echo::info("╚════════════════════════════════════════╝\n");

    demo_conversions();
    demo_mixing();
    demo_adjustments();
    demo_analysis();
    demo_schemes();
    demo_temperature();
    demo_random();

    echo::info("Demo complete!");
    return 0;
}
