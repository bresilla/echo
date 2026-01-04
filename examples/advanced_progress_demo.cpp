/**
 * @file advanced_progress_demo.cpp
 * @brief Demonstrates advanced progress bar features including styles, themes, and formatting
 */

#include <echo/widget.hpp>

#include <chrono>
#include <thread>

void demo_bar_styles() {
    echo::separator("Bar Styles Demo", '=');
    std::cout << "\n";

    // Classic style
    echo::progress_bar bar1(50);
    bar1.set_bar_style(echo::BarStyle::Classic);
    bar1.set_prefix("Classic");
    for (int i = 0; i <= 50; ++i) {
        bar1.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar1.finish();

    // Blocks style
    echo::progress_bar bar2(50);
    bar2.set_bar_style(echo::BarStyle::Blocks);
    bar2.set_prefix("Blocks");
    for (int i = 0; i <= 50; ++i) {
        bar2.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar2.finish();

    // Smooth style (with sub-block precision)
    echo::progress_bar bar3(50);
    bar3.set_bar_style(echo::BarStyle::Smooth);
    bar3.set_prefix("Smooth");
    for (int i = 0; i <= 50; ++i) {
        bar3.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar3.finish();

    // Arrows style
    echo::progress_bar bar4(50);
    bar4.set_bar_style(echo::BarStyle::Arrows);
    bar4.set_prefix("Arrows");
    for (int i = 0; i <= 50; ++i) {
        bar4.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar4.finish();

    // Dots style
    echo::progress_bar bar5(50);
    bar5.set_bar_style(echo::BarStyle::Dots);
    bar5.set_prefix("Dots");
    for (int i = 0; i <= 50; ++i) {
        bar5.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar5.finish();

    // ASCII style
    echo::progress_bar bar6(50);
    bar6.set_bar_style(echo::BarStyle::ASCII);
    bar6.set_prefix("ASCII");
    for (int i = 0; i <= 50; ++i) {
        bar6.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar6.finish();

    std::cout << "\n";
}

void demo_themes() {
    echo::separator("Pre-configured Themes", '=');
    std::cout << "\n";

    // Fire theme
    echo::progress_bar bar1(50);
    bar1.set_theme(echo::BarTheme::fire());
    bar1.set_prefix("Fire");
    for (int i = 0; i <= 50; ++i) {
        bar1.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar1.finish();

    // Ocean theme
    echo::progress_bar bar2(50);
    bar2.set_theme(echo::BarTheme::ocean());
    bar2.set_prefix("Ocean");
    for (int i = 0; i <= 50; ++i) {
        bar2.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar2.finish();

    // Forest theme
    echo::progress_bar bar3(50);
    bar3.set_theme(echo::BarTheme::forest());
    bar3.set_prefix("Forest");
    for (int i = 0; i <= 50; ++i) {
        bar3.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar3.finish();

    // Sunset theme
    echo::progress_bar bar4(50);
    bar4.set_theme(echo::BarTheme::sunset());
    bar4.set_prefix("Sunset");
    for (int i = 0; i <= 50; ++i) {
        bar4.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar4.finish();

    // Neon theme
    echo::progress_bar bar5(50);
    bar5.set_theme(echo::BarTheme::neon());
    bar5.set_prefix("Neon");
    for (int i = 0; i <= 50; ++i) {
        bar5.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar5.finish();

    std::cout << "\n";
}

void demo_byte_formatting() {
    echo::separator("Byte Formatting Demo", '=');
    std::cout << "\n";

    // Download simulation (10 MB file)
    const size_t total_bytes = 10 * 1024 * 1024; // 10 MB
    const size_t chunk_size = 256 * 1024;        // 256 KB chunks
    const size_t total_chunks = total_bytes / chunk_size;

    echo::progress_bar download(total_chunks);
    download.set_bar_style(echo::BarStyle::Smooth);
    download.set_show_bytes(true, chunk_size);
    download.set_show_speed(true);
    download.set_show_elapsed(true);
    download.set_prefix("Download");
    download.set_gradient({"#00FF00", "#FFFF00", "#FF0000"});

    for (size_t i = 0; i <= total_chunks; ++i) {
        download.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    download.finish();

    std::cout << "\n";
}

void demo_speed_display() {
    echo::separator("Speed Display Demo", '=');
    std::cout << "\n";

    // Processing items
    const size_t total_items = 1000;

    echo::progress_bar processor(total_items);
    processor.set_bar_style(echo::BarStyle::Blocks);
    processor.set_show_speed(true);
    processor.set_show_elapsed(true);
    processor.set_show_remaining(true);
    processor.set_prefix("Processing");
    processor.set_color("#00FFFF");

    for (size_t i = 0; i <= total_items; ++i) {
        processor.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    processor.finish();

    std::cout << "\n";
}

void demo_custom_width() {
    echo::separator("Custom Bar Width Demo", '=');
    std::cout << "\n";

    // Small bar
    echo::progress_bar small(50);
    small.set_bar_width(20);
    small.set_bar_style(echo::BarStyle::Blocks);
    small.set_prefix("Small (20)");
    small.set_color("#FF00FF");
    for (int i = 0; i <= 50; ++i) {
        small.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    small.finish();

    // Medium bar (default is 50)
    echo::progress_bar medium(50);
    medium.set_bar_style(echo::BarStyle::Smooth);
    medium.set_prefix("Medium (50)");
    medium.set_color("#00FF00");
    for (int i = 0; i <= 50; ++i) {
        medium.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    medium.finish();

    // Large bar
    echo::progress_bar large(50);
    large.set_bar_width(80);
    large.set_bar_style(echo::BarStyle::Arrows);
    large.set_prefix("Large (80)");
    large.set_gradient({"#FF0000", "#00FF00", "#0000FF"});
    for (int i = 0; i <= 50; ++i) {
        large.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    large.finish();

    std::cout << "\n";
}

void demo_combined_features() {
    echo::separator("Combined Features Demo", '=');
    std::cout << "\n";

    // Realistic file transfer simulation
    const size_t file_size = 50 * 1024 * 1024; // 50 MB
    const size_t chunk_size = 512 * 1024;      // 512 KB chunks
    const size_t total_chunks = file_size / chunk_size;

    echo::progress_bar transfer(total_chunks);
    transfer.set_theme(echo::BarTheme::ocean());
    transfer.set_bar_width(60);
    transfer.set_show_bytes(true, chunk_size);
    transfer.set_show_speed(true);
    transfer.set_show_elapsed(true);
    transfer.set_show_remaining(true);
    transfer.set_prefix("Transfer");

    for (size_t i = 0; i <= total_chunks; ++i) {
        transfer.set_progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    transfer.finish();

    std::cout << "\n";
}

int main() {
    echo::banner("Advanced Progress Bar Features", echo::BoxStyle::Double);
    std::cout << "\n";

    demo_bar_styles();
    demo_themes();
    demo_byte_formatting();
    demo_speed_display();
    demo_custom_width();
    demo_combined_features();

    echo::separator("Demo Complete!", '=');
    std::cout << "\n";

    return 0;
}
