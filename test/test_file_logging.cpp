// IMPORTANT: Include log.hpp FIRST to enable .log() method
#include <echo/log.hpp>

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// Helper function to read file contents
std::string read_file(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    return content;
}

// Helper function to count lines in file
int count_lines(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return 0;
    }
    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        count++;
    }
    return count;
}

// Helper function to check if string contains substring
bool contains(const std::string &str, const std::string &substr) { return str.find(substr) != std::string::npos; }

// Test 1: Basic file logging
void test_basic_file_logging() {
    std::cout << "Test 1: Basic file logging..." << std::endl;

    const std::string filename = "test_basic.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);
    assert(echo::log::is_enabled());

    echo::info("Test message").log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(contains(content, "Test message"));
    assert(contains(content, "[Info]"));

    std::remove(filename.c_str());
    std::cout << "  ✓ Basic file logging works" << std::endl;
}

// Test 2: ANSI code stripping
void test_ansi_stripping() {
    std::cout << "Test 2: ANSI code stripping..." << std::endl;

    const std::string filename = "test_ansi.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    echo::info("Red message").red().log();
    echo::info("Green message").green().log();
    echo::info("Custom color").hex("#FF5733").log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(contains(content, "Red message"));
    assert(contains(content, "Green message"));
    assert(contains(content, "Custom color"));

    // Ensure no ANSI codes in file
    assert(!contains(content, "\033["));
    assert(!contains(content, "38;2;"));

    std::remove(filename.c_str());
    std::cout << "  ✓ ANSI codes are stripped from file" << std::endl;
}

// Test 3: Multiple log levels
void test_log_levels() {
    std::cout << "Test 3: Log levels..." << std::endl;

    const std::string filename = "test_levels.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);
    echo::log::set_level(echo::Level::Trace);

    echo::trace("Trace message").log();
    echo::debug("Debug message").log();
    echo::info("Info message").log();
    echo::warn("Warn message").log();
    echo::error("Error message").log();
    echo::critical("Critical message").log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(contains(content, "Trace message"));
    assert(contains(content, "Debug message"));
    assert(contains(content, "Info message"));
    assert(contains(content, "Warn message"));
    assert(contains(content, "Error message"));
    assert(contains(content, "Critical message"));

    std::remove(filename.c_str());
    std::cout << "  ✓ All log levels work" << std::endl;
}

// Test 4: Log level filtering
void test_level_filtering() {
    std::cout << "Test 4: Log level filtering..." << std::endl;

    const std::string filename = "test_filter.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);
    echo::log::set_level(echo::Level::Warn); // Only Warn and above

    echo::trace("Trace message").log();
    echo::debug("Debug message").log();
    echo::info("Info message").log();
    echo::warn("Warn message").log();
    echo::error("Error message").log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(!contains(content, "Trace message"));
    assert(!contains(content, "Debug message"));
    assert(!contains(content, "Info message"));
    assert(contains(content, "Warn message"));
    assert(contains(content, "Error message"));

    std::remove(filename.c_str());
    std::cout << "  ✓ Log level filtering works" << std::endl;
}

// Test 5: Print proxy logging
void test_print_proxy() {
    std::cout << "Test 5: Print proxy logging..." << std::endl;

    const std::string filename = "test_print.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    echo("Simple message").log();
    echo("Colored message").blue().log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(contains(content, "Simple message"));
    assert(contains(content, "Colored message"));
    assert(!contains(content, "\033[")); // No ANSI codes

    std::remove(filename.c_str());
    std::cout << "  ✓ Print proxy logging works" << std::endl;
}

// Test 6: File rotation
void test_rotation() {
    std::cout << "Test 6: File rotation..." << std::endl;

    const std::string filename = "test_rotate.log";
    std::remove(filename.c_str());
    std::remove((filename + ".1").c_str());
    std::remove((filename + ".2").c_str());

    echo::log::set_file(filename);
    echo::log::enable_rotation(100, 2); // 100 bytes, keep 2 files

    // Write enough to trigger rotation
    for (int i = 0; i < 20; i++) {
        echo::info("This is a test message number ", i).log();
    }
    echo::log::close();

    // Check that rotation happened
    std::ifstream file1(filename + ".1");
    bool rotated = file1.good();
    file1.close();

    assert(rotated);

    std::remove(filename.c_str());
    std::remove((filename + ".1").c_str());
    std::remove((filename + ".2").c_str());
    std::remove((filename + ".3").c_str());
    std::cout << "  ✓ File rotation works" << std::endl;
}

// Test 7: Chaining with other methods
void test_chaining() {
    std::cout << "Test 7: Method chaining..." << std::endl;

    const std::string filename = "test_chain.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    echo::info("Chained message").red().bold().log();
    echo::warn("Another chain").hex("#00FF00").italic().log();
    echo::log::close();

    std::string content = read_file(filename);
    assert(contains(content, "Chained message"));
    assert(contains(content, "Another chain"));

    std::remove(filename.c_str());
    std::cout << "  ✓ Method chaining works" << std::endl;
}

// Test 8: Multiple messages
void test_multiple_messages() {
    std::cout << "Test 8: Multiple messages..." << std::endl;

    const std::string filename = "test_multiple.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    for (int i = 0; i < 10; i++) {
        echo::info("Message ", i).log();
    }
    echo::log::close();

    int lines = count_lines(filename);
    assert(lines == 10);

    std::remove(filename.c_str());
    std::cout << "  ✓ Multiple messages work" << std::endl;
}

// Test 9: File without .log() doesn't write
void test_no_log_method() {
    std::cout << "Test 9: Without .log() method..." << std::endl;

    const std::string filename = "test_nolog.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    // These should NOT write to file (no .log() call)
    echo::info("Console only 1");
    echo::warn("Console only 2");

    // This SHOULD write to file
    echo::info("File message").log();

    echo::log::close();

    std::string content = read_file(filename);
    assert(!contains(content, "Console only 1"));
    assert(!contains(content, "Console only 2"));
    assert(contains(content, "File message"));

    std::remove(filename.c_str());
    std::cout << "  ✓ Messages without .log() don't write to file" << std::endl;
}

// Test 10: Flush functionality
void test_flush() {
    std::cout << "Test 10: Flush functionality..." << std::endl;

    const std::string filename = "test_flush.log";
    std::remove(filename.c_str());

    echo::log::set_file(filename);

    echo::info("Message before flush").log();
    echo::log::flush();

    // File should contain the message even without closing
    std::string content = read_file(filename);
    assert(contains(content, "Message before flush"));

    echo::log::close();
    std::remove(filename.c_str());
    std::cout << "  ✓ Flush works" << std::endl;
}

int main() {
    std::cout << "\n=== Echo File Logging Tests ===\n" << std::endl;

    try {
        test_basic_file_logging();
        test_ansi_stripping();
        test_log_levels();
        test_level_filtering();
        test_print_proxy();
        test_rotation();
        test_chaining();
        test_multiple_messages();
        test_no_log_method();
        test_flush();

        std::cout << "\n✅ All tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
