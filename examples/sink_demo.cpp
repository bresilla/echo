/**
 * @file sink_demo.cpp
 * @brief Demonstration of Echo's sink system
 *
 * Shows how to:
 * - Use the default ConsoleSink
 * - Add multiple sinks
 * - Configure per-sink log levels
 * - Use FileSink, SyslogSink, and NetworkSink
 */

#include <echo/echo.hpp>

#include <iostream>
#include <thread>

int main() {
    std::cout << "=== Echo Sink System Demo ===\n\n";

    // By default, Echo uses ConsoleSink
    std::cout << "1. Default ConsoleSink:\n";
    echo::info("This goes to the console (default sink)");
    echo::warn("Warnings are also logged");
    echo::error("Errors go to stderr");
    std::cout << "\n";

    // Add a file sink (if enabled)
#ifdef ECHO_ENABLE_FILE_SINK
    std::cout << "2. Adding FileSink:\n";
    auto file_sink = std::make_shared<echo::FileSink>("demo.log");
    file_sink->set_level(echo::Level::Debug); // Log Debug and above to file
    echo::add_sink(file_sink);

    echo::debug("This goes to both console and file");
    echo::info("Multiple sinks receive the same message");
    std::cout << "   (Check demo.log for file output)\n\n";
#endif

    // Add a syslog sink (if enabled and on Unix)
#ifdef ECHO_ENABLE_SYSLOG_SINK
    std::cout << "3. Adding SyslogSink:\n";
    auto syslog_sink = std::make_shared<echo::SyslogSink>("EchoDemo");
    syslog_sink->set_level(echo::Level::Warn); // Only warnings and above to syslog
    echo::add_sink(syslog_sink);

    echo::info("This goes to console and file, but NOT syslog (below Warn level)");
    echo::warn("This goes to console, file, AND syslog");
    std::cout << "   (Check system logs with: journalctl -t EchoDemo)\n\n";
#endif

    // Add a network sink (if enabled)
#ifdef ECHO_ENABLE_NETWORK_SINK
    std::cout << "4. Adding NetworkSink (UDP):\n";
    // Send logs to localhost:5140 (you can run: nc -ul 5140 to receive)
    auto network_sink = std::make_shared<echo::NetworkSink>("127.0.0.1", 5140, echo::NetworkSink::Protocol::UDP);
    network_sink->set_level(echo::Level::Error); // Only errors to network
    echo::add_sink(network_sink);

    echo::info("This doesn't go to network (below Error level)");
    echo::error("This goes to ALL sinks including network");
    std::cout << "   (Run 'nc -ul 5140' in another terminal to see network output)\n\n";
#endif

    // Demonstrate sink count
    std::cout << "5. Sink Management:\n";
    std::cout << "   Total sinks registered: " << echo::sink_count() << "\n";

    // Clear all sinks and add only console
    std::cout << "   Clearing all sinks...\n";
    echo::clear_sinks();
    std::cout << "   Sinks after clear: " << echo::sink_count() << "\n";

    // Add console back
    auto console = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console);
    std::cout << "   Added ConsoleSink back: " << echo::sink_count() << " sink(s)\n";
    echo::info("Back to console-only logging");
    std::cout << "\n";

    // Demonstrate per-sink level filtering
    std::cout << "6. Per-Sink Level Filtering:\n";
    echo::clear_sinks();

    auto console_info = std::make_shared<echo::ConsoleSink>();
    console_info->set_level(echo::Level::Info);
    echo::add_sink(console_info);

#ifdef ECHO_ENABLE_FILE_SINK
    auto file_debug = std::make_shared<echo::FileSink>("debug.log");
    file_debug->set_level(echo::Level::Debug);
    echo::add_sink(file_debug);
#endif

    std::cout << "   Console: Info and above\n";
#ifdef ECHO_ENABLE_FILE_SINK
    std::cout << "   File: Debug and above\n";
#endif

    echo::debug("Debug message (file only, not console)");
    echo::info("Info message (both console and file)");
    echo::error("Error message (both console and file)");
    std::cout << "\n";

    // Flush all sinks before exit
    std::cout << "7. Flushing all sinks...\n";
    echo::flush();

    std::cout << "\n=== Demo Complete ===\n";
    std::cout << "Summary:\n";
    std::cout << "  - ConsoleSink: Always available, writes to stdout/stderr\n";
#ifdef ECHO_ENABLE_FILE_SINK
    std::cout << "  - FileSink: Writes to files with rotation support\n";
#endif
#ifdef ECHO_ENABLE_SYSLOG_SINK
    std::cout << "  - SyslogSink: Integrates with system logging (Unix)\n";
#endif
#ifdef ECHO_ENABLE_NETWORK_SINK
    std::cout << "  - NetworkSink: Sends logs over TCP/UDP\n";
#endif
    std::cout << "  - Each sink can have its own log level filter\n";
    std::cout << "  - Multiple sinks can be active simultaneously\n";

    return 0;
}
