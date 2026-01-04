#pragma once

/**
 * @file log.hpp
 * @brief File logging extension for Echo library
 *
 * This header adds file logging capabilities to the Echo library.
 * When included, it enables the .log() method on all echo logging functions.
 *
 * Usage:
 *   IMPORTANT: Include log.hpp BEFORE echo.hpp or ONLY include log.hpp
 *
 *   #include <echo/log.hpp>  // This automatically includes echo.hpp with .log() enabled
 *
 *   // Configure file logging
 *   echo::log::set_file("app.log");
 *   echo::log::enable_rotation(5 * 1024 * 1024, 3);  // 5MB, keep 3 files
 *
 *   // Use .log() to write to file
 *   echo::info("This goes to console and file").log();
 *   echo::error("Error message").red().log();
 *
 * Configuration:
 *   1. Compile-time (CMake):
 *      -DECHO_LOG_LEVEL=Info
 *      -DECHO_LOG_FILE="app.log"
 *
 *   2. Environment variable:
 *      export ECHO_LOG_LEVEL=Debug
 *      export ECHO_LOG_FILE=/var/log/app.log
 *
 *   3. Runtime:
 *      echo::log::set_level(echo::Level::Debug);
 *      echo::log::set_file("app.log");
 *
 * Features:
 *   - Automatic timestamps with millisecond precision (always enabled for files)
 *   - Automatic ANSI color code stripping for files
 *   - Thread-safe file writing
 *   - Optional log rotation (size-based)
 *   - Independent log level control for files
 *   - Works with all echo logging functions
 *
 * Log Format:
 *   [YYYY-MM-DD HH:MM:SS.mmm][level] message
 *   Example: [2026-01-04 13:52:41.625][info] Application started
 */

#ifndef ECHO_LOG_HPP
#define ECHO_LOG_HPP

// Check if echo.hpp was already included without ECHO_LOG_ENABLED
#ifdef ECHO_HPP
#ifndef ECHO_LOG_ENABLED
#error "echo/log.hpp must be included BEFORE echo/echo.hpp, or include ONLY echo/log.hpp"
#endif
#endif

// Enable logging methods in echo.hpp
#define ECHO_LOG_ENABLED

// Include echo.hpp (will have .log() methods enabled)
#include <echo/echo.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>

namespace echo::log {

    // =================================================================================================
    // Configuration and state management
    // =================================================================================================

    namespace detail {
        // Compile-time log level for file logging
#ifdef ECHO_LOG_LEVEL
        constexpr Level ACTIVE_LOG_LEVEL = Level::ECHO_LOG_LEVEL;
#else
        constexpr Level ACTIVE_LOG_LEVEL = Level::Trace; // Log everything by default
#endif

        // Runtime level storage
        inline Level &get_runtime_log_level() {
            static Level level = []() {
                // Check environment variable
                const char *env = std::getenv("ECHO_LOG_LEVEL");
                if (env) {
                    std::string level_str(env);
                    if (level_str == "Trace")
                        return Level::Trace;
                    if (level_str == "Debug")
                        return Level::Debug;
                    if (level_str == "Info")
                        return Level::Info;
                    if (level_str == "Warn")
                        return Level::Warn;
                    if (level_str == "Error")
                        return Level::Error;
                    if (level_str == "Critical")
                        return Level::Critical;
                    if (level_str == "Off")
                        return Level::Off;
                }
                return ACTIVE_LOG_LEVEL;
            }();
            return level;
        }

        inline Level get_effective_log_level() { return get_runtime_log_level(); }

        // File state management
        struct FileState {
            std::string filename;
            std::ofstream file;
            std::mutex mutex;
            bool enabled = false;
            size_t current_size = 0;
            size_t max_size = 5 * 1024 * 1024; // 5MB default
            int max_files = 3;
            bool rotation_enabled = false;

            // Delete copy constructor and assignment (non-copyable due to ofstream and mutex)
            FileState(const FileState &) = delete;
            FileState &operator=(const FileState &) = delete;

            // Default constructor
            FileState() = default;
        };

        inline FileState &get_file_state() {
            static FileState state;
            static bool initialized = false;

            if (!initialized) {
                // Check environment variable for file path
                const char *env_file = std::getenv("ECHO_LOG_FILE");
                if (env_file) {
                    state.filename = env_file;
                    state.file.open(env_file, std::ios::app);
                    state.enabled = state.file.is_open();
                }
                initialized = true;
            }

            return state;
        }

        // =================================================================================================
        // ANSI code stripping
        // =================================================================================================

        inline std::string strip_ansi(const std::string &str) {
            std::string result;
            result.reserve(str.size());

            bool in_escape = false;
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
                    in_escape = true;
                    ++i; // Skip '['
                    continue;
                }
                if (in_escape) {
                    if (str[i] == 'm') {
                        in_escape = false;
                    }
                    continue;
                }
                result += str[i];
            }
            return result;
        }

        // =================================================================================================
        // File rotation
        // =================================================================================================

        inline void rotate_if_needed() {
            auto &state = get_file_state();
            if (!state.rotation_enabled || state.current_size < state.max_size) {
                return;
            }

            state.file.close();

            // Rotate files: app.2.log -> app.3.log, app.1.log -> app.2.log
            for (int i = state.max_files - 1; i > 0; --i) {
                std::string old_name = state.filename + "." + std::to_string(i);
                std::string new_name = state.filename + "." + std::to_string(i + 1);
                std::rename(old_name.c_str(), new_name.c_str());
            }

            // Rename current: app.log -> app.1.log
            std::string backup = state.filename + ".1";
            std::rename(state.filename.c_str(), backup.c_str());

            // Open new file
            state.file.open(state.filename, std::ios::app);
            state.current_size = 0;
        }

        // =================================================================================================
        // Timestamp generation (always enabled for file logging)
        // =================================================================================================

        inline std::string get_file_timestamp() {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            std::tm tm_buf;
#ifdef _WIN32
            localtime_s(&tm_buf, &time_t_now);
#else
            localtime_r(&time_t_now, &tm_buf);
#endif

            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_buf);

            // Add milliseconds
            std::string result = buffer;
            result += ".";
            if (ms.count() < 100)
                result += "0";
            if (ms.count() < 10)
                result += "0";
            result += std::to_string(ms.count());

            return result;
        }

        // =================================================================================================
        // File writing
        // =================================================================================================

        inline void write_line(const std::string &line, Level level) {
            auto &state = get_file_state();

            if (!state.enabled)
                return;

            // Check log level
            if (static_cast<int>(level) < static_cast<int>(get_effective_log_level())) {
                return;
            }

            std::lock_guard<std::mutex> lock(state.mutex);

            rotate_if_needed();

            std::string clean_line = strip_ansi(line);
            state.file << clean_line << std::endl;
            state.current_size += clean_line.size() + 1;
        }

        // =================================================================================================
        // File writer for log_proxy
        // =================================================================================================

        template <Level L> struct file_writer {
            static void write(log_proxy<L> &proxy, Level level) {
                // Access private members via friendship
                std::string full_message;

                // Always add timestamp for file logging
                full_message += "[" + get_file_timestamp() + "]";
                full_message += "[" + std::string(echo::detail::level_name(level)) + "] ";
                full_message += proxy.message_;

                write_line(full_message, level);
            }
        };

        // File writer for print_proxy
        struct file_writer_print {
            static void write(print_proxy &proxy) {
                auto &state = get_file_state();
                if (!state.enabled)
                    return;

                std::lock_guard<std::mutex> lock(state.mutex);
                rotate_if_needed();

                // Always add timestamp for file logging
                std::string full_message = "[" + get_file_timestamp() + "] ";
                full_message += strip_ansi(proxy.message_);

                state.file << full_message << std::endl;
                state.current_size += full_message.size() + 1;
            }
        };

        // =================================================================================================
        // Implementation of write_to_file functions
        // =================================================================================================

        template <Level L> void write_to_file(log_proxy<L> &proxy, Level level) { file_writer<L>::write(proxy, level); }

        inline void write_to_file(print_proxy &proxy) { file_writer_print::write(proxy); }

    } // namespace detail

    // =================================================================================================
    // Public API
    // =================================================================================================

    /**
     * @brief Set the log file path
     * @param filename Path to the log file
     */
    inline void set_file(const std::string &filename) {
        auto &state = detail::get_file_state();
        std::lock_guard<std::mutex> lock(state.mutex);

        if (state.file.is_open()) {
            state.file.close();
        }

        state.filename = filename;
        state.file.open(filename, std::ios::app);
        state.enabled = state.file.is_open();
        state.current_size = 0;
    }

    /**
     * @brief Enable log file rotation
     * @param max_size Maximum file size in bytes before rotation
     * @param max_files Maximum number of rotated files to keep
     */
    inline void enable_rotation(size_t max_size = 5 * 1024 * 1024, int max_files = 3) {
        auto &state = detail::get_file_state();
        state.rotation_enabled = true;
        state.max_size = max_size;
        state.max_files = max_files;
    }

    /**
     * @brief Disable log file rotation
     */
    inline void disable_rotation() {
        auto &state = detail::get_file_state();
        state.rotation_enabled = false;
    }

    /**
     * @brief Set minimum log level for file logging
     * @param level Minimum level to log to file
     */
    inline void set_level(Level level) { detail::get_runtime_log_level() = level; }

    /**
     * @brief Get current file log level
     * @return Current minimum log level
     */
    inline Level get_level() { return detail::get_runtime_log_level(); }

    /**
     * @brief Close the log file
     */
    inline void close() {
        auto &state = detail::get_file_state();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.file.is_open()) {
            state.file.close();
        }
        state.enabled = false;
    }

    /**
     * @brief Check if file logging is enabled
     * @return true if file logging is active
     */
    inline bool is_enabled() { return detail::get_file_state().enabled; }

    /**
     * @brief Get current log file path
     * @return Path to the log file
     */
    inline std::string get_file() { return detail::get_file_state().filename; }

    /**
     * @brief Flush the log file
     */
    inline void flush() {
        auto &state = detail::get_file_state();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.file.is_open()) {
            state.file.flush();
        }
    }

} // namespace echo::log

#endif // ECHO_LOG_HPP
