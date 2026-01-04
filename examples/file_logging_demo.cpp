/**
 * @file file_logging_demo.cpp
 * @brief Demonstrates file logging capabilities of the Echo library
 */

// IMPORTANT: Include log.hpp to enable .log() method
#include <echo/log.hpp>

#include <chrono>
#include <thread>

void demo_basic_file_logging() {
    echo::info("=== Basic File Logging Demo ===").cyan();

    // Set up file logging
    echo::log::set_file("demo.log");
    echo::info("File logging enabled to: demo.log").green();
    echo::info("Timestamps are ALWAYS added to file logs!").yellow();

    // Messages with .log() go to both console and file
    echo::info("This message goes to console and file").log();
    echo::warn("Warning message").yellow().log();
    echo::error("Error message").red().log();

    // Messages without .log() only go to console
    echo::info("This only goes to console (no .log())").magenta();

    echo::info("✓ Check demo.log - all entries have timestamps!\n").green();
}

void demo_ansi_stripping() {
    echo::info("=== ANSI Code Stripping Demo ===").cyan();

    echo::log::set_file("demo_colors.log");

    // Colored messages - colors appear in console but not in file
    echo::info("Red text in console, plain in file").red().log();
    echo::info("Green text in console, plain in file").green().log();
    echo::info("Custom hex color").hex("#FF5733").log();
    echo::info("RGB color").rgb(100, 200, 255).log();
    echo::info("Bold and italic").bold().italic().log();

    echo::info("✓ Check demo_colors.log - no ANSI codes!\n").green();
}

void demo_log_levels() {
    echo::info("=== Log Level Filtering Demo ===").cyan();

    echo::log::set_file("demo_levels.log");

    // Set file log level to Warn (only Warn and above go to file)
    echo::log::set_level(echo::Level::Warn);
    echo::info("File log level set to Warn").yellow();

    // These won't go to file (below Warn level)
    echo::trace("Trace - console only").log();
    echo::debug("Debug - console only").log();
    echo::info("Info - console only").log();

    // These will go to file (Warn and above)
    echo::warn("Warning - console AND file").yellow().log();
    echo::error("Error - console AND file").red().log();
    echo::critical("Critical - console AND file").magenta().log();

    echo::info("✓ Check demo_levels.log - only Warn+ messages\n").green();

    // Reset to log everything
    echo::log::set_level(echo::Level::Trace);
}

void demo_rotation() {
    echo::info("=== File Rotation Demo ===").cyan();

    echo::log::set_file("demo_rotate.log");

    // Enable rotation: 1KB max size, keep 3 old files
    echo::log::enable_rotation(1024, 3);
    echo::info("Rotation enabled: 1KB max, keep 3 files").yellow();

    // Write many messages to trigger rotation
    for (int i = 0; i < 50; i++) {
        echo::info("Message number ", i, " - writing to file").log();
    }

    echo::info("✓ Check demo_rotate.log, demo_rotate.log.1, etc.\n").green();
}

void demo_print_proxy() {
    echo::info("=== Print Proxy (echo) Logging Demo ===").cyan();

    echo::log::set_file("demo_print.log");

    // The global echo() function also supports .log()
    echo("Simple message").log();
    echo("Colored message").blue().log();
    echo("Bold message").bold().log();

    echo::info("✓ Print proxy logging works too!\n").green();
}

void demo_chaining() {
    echo::info("=== Method Chaining Demo ===").cyan();

    echo::log::set_file("demo_chain.log");

    // All methods can be chained with .log()
    echo::info("Chained").red().bold().italic().log();
    echo::warn("Multiple").hex("#00FF00").underline().log();
    echo::error("Styles").rgb(255, 100, 50).bold().log();

    echo::info("✓ Method chaining works perfectly\n").green();
}

void demo_environment_variables() {
    echo::info("=== Environment Variable Demo ===").cyan();

    echo::info("You can configure file logging via environment variables:").yellow();
    echo::info("  export ECHO_LOG_FILE=/var/log/myapp.log");
    echo::info("  export ECHO_LOG_LEVEL=Warn");
    echo::info("");
    echo::info("Current settings:");
    echo::info("  File: ", echo::log::get_file());
    echo::info("  Level: ", static_cast<int>(echo::log::get_level()));
    echo::info("");
}

void demo_runtime_control() {
    echo::info("=== Runtime Control Demo ===").cyan();

    // Start with one file
    echo::log::set_file("demo_runtime1.log");
    echo::info("Logging to demo_runtime1.log").log();

    // Switch to another file
    echo::log::set_file("demo_runtime2.log");
    echo::info("Now logging to demo_runtime2.log").log();

    // Disable file logging
    echo::log::close();
    echo::info("File logging disabled - console only");

    // Re-enable
    echo::log::set_file("demo_runtime3.log");
    echo::info("Re-enabled to demo_runtime3.log").log();

    echo::info("✓ Runtime control works\n").green();
}

void demo_thread_safety() {
    echo::info("=== Thread Safety Demo ===").cyan();

    echo::log::set_file("demo_threads.log");
    echo::info("Starting 5 threads, each writing 10 messages...").yellow();

    auto worker = [](int thread_id) {
        for (int i = 0; i < 10; i++) {
            echo::info("Thread ", thread_id, " message ", i).log();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    echo::info("✓ Check demo_threads.log - all messages intact\n").green();
}

void demo_timestamps() {
    echo::info("=== Timestamp Demo ===").cyan();

    echo::log::set_file("demo_timestamps.log");
    echo::info("Timestamps are ALWAYS enabled for file logging").yellow();
    echo::info("Format: [YYYY-MM-DD HH:MM:SS.mmm][level] message").yellow();

    echo::info("Message 1").log();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    echo::info("Message 2 (100ms later)").log();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    echo::info("Message 3 (250ms later)").log();

    echo::info("✓ Check demo_timestamps.log - see millisecond precision!\n").green();
}

void demo_flush() {
    echo::info("=== Manual Flush Demo ===").cyan();

    echo::log::set_file("demo_flush.log");

    echo::info("Writing message...").log();
    echo::info("Flushing to ensure it's written to disk...").yellow();
    echo::log::flush();
    echo::info("✓ Message flushed to file\n").green();
}

int main() {
    echo::info("\n╔════════════════════════════════════════╗").bold().cyan();
    echo::info("║  Echo File Logging Demo                ║").bold().cyan();
    echo::info("╚════════════════════════════════════════╝\n").bold().cyan();

    demo_basic_file_logging();
    demo_timestamps();
    demo_ansi_stripping();
    demo_log_levels();
    demo_rotation();
    demo_print_proxy();
    demo_chaining();
    demo_environment_variables();
    demo_runtime_control();
    demo_thread_safety();
    demo_flush();

    // Clean up
    echo::log::close();

    echo::info("╔════════════════════════════════════════╗").bold().green();
    echo::info("║  All demos complete!                   ║").bold().green();
    echo::info("║  Check the demo_*.log files            ║").bold().green();
    echo::info("╚════════════════════════════════════════╝\n").bold().green();

    return 0;
}
