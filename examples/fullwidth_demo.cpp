/**
 * @file fullwidth_demo.cpp
 * @brief Demonstrates full-width progress bars (auto-sizing to terminal)
 */

#include <echo/banner.hpp>
#include <echo/wait.hpp>

#include <chrono>
#include <thread>

int main() {
    echo::banner("Full-Width Progress Bars", echo::BoxStyle::Double);
    std::cout << "\n";

    echo::separator("Default: Full Terminal Width");
    std::cout << "\n";

    // Default: full terminal width (no set_bar_width call)
    echo::progress_bar bar1(100);
    bar1.set_prefix("Download");
    bar1.set_bar_style(echo::BarStyle::Smooth);
    bar1.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});
    for (int i = 0; i <= 100; ++i) {
        bar1.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar1.finish();

    std::cout << "\n";
    echo::separator("With Percentage and Time");
    std::cout << "\n";

    // Full width with percentage and time
    echo::progress_bar bar2(50);
    bar2.set_prefix("Processing");
    bar2.set_show_percentage(true);
    bar2.set_show_elapsed(true);
    bar2.set_show_remaining(true);
    bar2.set_bar_style(echo::BarStyle::Blocks);
    bar2.set_color("#00FFFF");
    for (int i = 0; i <= 50; ++i) {
        bar2.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    bar2.finish();

    std::cout << "\n";
    echo::separator("With Byte Formatting and Speed");
    std::cout << "\n";

    // Full width with bytes and speed
    const size_t total_bytes = 50 * 1024 * 1024; // 50 MB
    const size_t chunk_size = 512 * 1024;        // 512 KB chunks
    const size_t total_chunks = total_bytes / chunk_size;

    echo::progress_bar bar3(total_chunks);
    bar3.set_prefix("Transfer");
    bar3.set_show_bytes(true, chunk_size);
    bar3.set_show_speed(true);
    bar3.set_show_elapsed(true);
    bar3.set_bar_style(echo::BarStyle::Arrows);
    bar3.set_gradient({"#FF00FF", "#00FFFF"});
    for (size_t i = 0; i <= total_chunks; ++i) {
        bar3.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    bar3.finish();

    std::cout << "\n";
    echo::separator("Custom Width (40 chars)");
    std::cout << "\n";

    // Custom width (overrides auto-sizing)
    echo::progress_bar bar4(50);
    bar4.set_bar_width(40);
    bar4.set_prefix("Custom");
    bar4.set_bar_style(echo::BarStyle::Dots);
    bar4.set_color("#FFFF00");
    for (int i = 0; i <= 50; ++i) {
        bar4.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar4.finish();

    std::cout << "\n";
    echo::separator("Demo Complete!");
    std::cout << "\n";

    return 0;
}
