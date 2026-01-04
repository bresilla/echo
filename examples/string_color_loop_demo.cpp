/**
 * @file string_color_loop_demo.cpp
 * @brief Demonstrates format::String color animation with inplace logging
 */

#define LOGLEVEL Trace
#include <echo/echo.hpp>
#include <echo/format.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using echo::format::String;

int main() {
    echo::info("╔════════════════════════════════════════╗");
    echo::info("║   String Color Animation Demo         ║");
    echo::info("╚════════════════════════════════════════╝\n");

    echo::info("Watch the color transition from green to red background:\n");

    // Animate color transition from green to red background
    for (int i = 0; i <= 100; i += 2) {
        // Calculate color transition (green to red)
        int green = 255 - (i * 255 / 100); // 255 -> 0
        int red = (i * 255 / 100);         // 0 -> 255

        // Create formatted string with changing background color
        String status = String("Processing... " + std::to_string(i) + "%").bg(red, green, 0).white().bold();

        // Use inplace to update the same line
        echo::info(status).inplace();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n\n";
    echo::info("Color transition complete!\n");

    // Another example: Pulsing effect
    echo::info("Pulsing color effect:\n");

    for (int cycle = 0; cycle < 3; ++cycle) {
        for (int i = 0; i <= 100; i += 5) {
            // Pulse between cyan and magenta
            int cyan_amount = 255 - (i * 255 / 100);
            int magenta_amount = (i * 255 / 100);

            String pulse = String("● PULSE ●").fg(magenta_amount, 0, cyan_amount).bold().center(20);

            echo::info(pulse).inplace();
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        for (int i = 100; i >= 0; i -= 5) {
            int cyan_amount = 255 - (i * 255 / 100);
            int magenta_amount = (i * 255 / 100);

            String pulse = String("● PULSE ●").fg(magenta_amount, 0, cyan_amount).bold().center(20);

            echo::info(pulse).inplace();
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    std::cout << "\n\n";

    // Example: Rainbow text
    echo::info("Rainbow text effect:\n");

    const std::string text = "RAINBOW COLORS";
    for (int offset = 0; offset < 360; offset += 10) {
        std::string colored_text;
        for (size_t i = 0; i < text.length(); ++i) {
            // Calculate hue for each character
            int hue = (offset + i * 20) % 360;

            // Simple HSV to RGB conversion (simplified)
            int r, g, b;
            if (hue < 60) {
                r = 255;
                g = (hue * 255) / 60;
                b = 0;
            } else if (hue < 120) {
                r = 255 - ((hue - 60) * 255) / 60;
                g = 255;
                b = 0;
            } else if (hue < 180) {
                r = 0;
                g = 255;
                b = ((hue - 120) * 255) / 60;
            } else if (hue < 240) {
                r = 0;
                g = 255 - ((hue - 180) * 255) / 60;
                b = 255;
            } else if (hue < 300) {
                r = ((hue - 240) * 255) / 60;
                g = 0;
                b = 255;
            } else {
                r = 255;
                g = 0;
                b = 255 - ((hue - 300) * 255) / 60;
            }

            String char_str = String(std::string(1, text[i])).fg(r, g, b).bold();
            colored_text += char_str.to_string();
        }

        echo::info(colored_text).inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n\n";

    // Example: Status indicators with different colors
    echo::info("Status indicator simulation:\n");

    std::vector<std::string> statuses = {"IDLE", "STARTING", "RUNNING", "WARNING", "ERROR", "CRITICAL", "STOPPED"};

    for (const auto &status : statuses) {
        String indicator;

        if (status == "IDLE") {
            indicator = String("[" + status + "]").fg("#888888").dim();
        } else if (status == "STARTING") {
            indicator = String("[" + status + "]").fg("#00FFFF").bold();
        } else if (status == "RUNNING") {
            indicator = String("[" + status + "]").fg("#00FF00").bold();
        } else if (status == "WARNING") {
            indicator = String("[" + status + "]").fg("#FFFF00").bold();
        } else if (status == "ERROR") {
            indicator = String("[" + status + "]").fg("#FF8800").bold();
        } else if (status == "CRITICAL") {
            indicator = String("[" + status + "]").bg("#FF0000").white().bold().blink();
        } else if (status == "STOPPED") {
            indicator = String("[" + status + "]").fg("#FF0000").dim();
        }

        echo::info("System Status: ", indicator);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }

    std::cout << "\n\n";

    // Example: Progress bar with color gradient
    echo::info("Progress bar with color gradient:\n");

    for (int progress = 0; progress <= 100; progress += 2) {
        // Color gradient from red -> yellow -> green
        int r, g;
        if (progress < 50) {
            r = 255;
            g = (progress * 2 * 255) / 100;
        } else {
            r = 255 - ((progress - 50) * 2 * 255) / 100;
            g = 255;
        }

        String bar = String("").progress(progress).fg(r, g, 0).bold();

        echo::info("Loading: ", bar).inplace();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n\n";
    echo::info("Demo complete!");

    return 0;
}
