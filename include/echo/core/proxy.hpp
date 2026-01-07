#pragma once

/**
 * @file core/proxy.hpp
 * @brief Proxy classes for fluent logging interface
 */

#include <echo/core/formatter.hpp>
#include <echo/core/level.hpp>
#include <echo/core/mutex.hpp>
#include <echo/core/once.hpp>
#include <echo/core/timestamp.hpp>

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace echo {

#ifdef ECHO_LOG_ENABLED
    // Forward declarations for file logging (defined in log.hpp)
    namespace log::detail {
        template <Level L> struct file_writer;
        struct file_writer_print;
    } // namespace log::detail
#endif

    // =================================================================================================
    // Fluent logging interface with color support
    // =================================================================================================

    /**
     * @brief Proxy object for fluent logging with color methods
     *
     * Allows chaining like:
     *   echo::info("message").red()
     *   echo::info("message").hex("#FF5733")
     *   echo::info("message").rgb(255, 87, 51)
     *   echo::info("message").cyan().bold().italic()
     */
    template <Level L> class log_proxy {
      private:
        std::string message_;
        std::string color_code_;
        bool skip_print_ = false;
        bool inplace_ = false;

#ifdef ECHO_LOG_ENABLED
        // Friend declaration for log.hpp to access private members
        template <Level> friend struct log::detail::file_writer;
#endif

      public:
        template <typename... Args> log_proxy(const Args &...args) {
            // Only build message if it will be printed (compile-time check)
            if constexpr (static_cast<int>(L) >= static_cast<int>(detail::ACTIVE_LEVEL)) {
                std::ostringstream oss;
                detail::append_args(oss, args...);
                message_ = oss.str();
            }
        }

        // Move semantics - allow moving but prevent copying
        log_proxy(log_proxy &&other) noexcept
            : message_(std::move(other.message_)), color_code_(std::move(other.color_code_)),
              skip_print_(other.skip_print_), inplace_(other.inplace_) {
            other.skip_print_ = true; // Prevent moved-from object from printing
        }

        log_proxy &operator=(log_proxy &&other) noexcept {
            if (this != &other) {
                message_ = std::move(other.message_);
                color_code_ = std::move(other.color_code_);
                skip_print_ = other.skip_print_;
                inplace_ = other.inplace_;
                other.skip_print_ = true;
            }
            return *this;
        }

        // Prevent copying
        log_proxy(const log_proxy &) = delete;
        log_proxy &operator=(const log_proxy &) = delete;

        // Color methods (24-bit RGB)
        log_proxy &red() {
            color_code_ = "\033[38;2;255;0;0m";
            return *this;
        }
        log_proxy &green() {
            color_code_ = "\033[38;2;0;255;0m";
            return *this;
        }
        log_proxy &yellow() {
            color_code_ = "\033[38;2;255;255;0m";
            return *this;
        }
        log_proxy &blue() {
            color_code_ = "\033[38;2;0;0;255m";
            return *this;
        }
        log_proxy &magenta() {
            color_code_ = "\033[38;2;255;0;255m";
            return *this;
        }
        log_proxy &cyan() {
            color_code_ = "\033[38;2;0;255;255m";
            return *this;
        }
        log_proxy &white() {
            color_code_ = "\033[38;2;255;255;255m";
            return *this;
        }
        log_proxy &gray() {
            color_code_ = "\033[38;2;128;128;128m";
            return *this;
        }
        log_proxy &bold() {
            color_code_ += "\033[1m";
            return *this;
        }
        log_proxy &italic() {
            color_code_ += "\033[3m";
            return *this;
        }
        log_proxy &underline() {
            color_code_ += "\033[4m";
            return *this;
        }

        // Custom hex color
        log_proxy &hex(const std::string &hex_color) {
            // Fallback: parse hex manually with exception safety
            try {
                if (hex_color.length() >= 6) {
                    std::string h = hex_color;
                    if (h[0] == '#')
                        h = h.substr(1);
                    if (h.length() == 6) {
                        int r = std::stoi(h.substr(0, 2), nullptr, 16);
                        int g = std::stoi(h.substr(2, 2), nullptr, 16);
                        int b = std::stoi(h.substr(4, 2), nullptr, 16);
                        color_code_ =
                            "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
                    }
                }
            } catch (const std::exception &) {
                // Invalid hex color, leave color_code_ unchanged
            }
            return *this;
        }

        // Custom RGB color
        log_proxy &rgb(int r, int g, int b) {
            color_code_ = "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
            return *this;
        }

        // Print only once (internal - use ONCE macro instead)
        log_proxy &once_impl(const char *file, int line) {
            if (!detail::check_and_mark_once(file, line)) {
                skip_print_ = true;
            }
            return *this;
        }

        // Print at most every N milliseconds (internal - use EVERY macro instead)
        log_proxy &every_impl(const char *file, int line, int64_t interval_ms) {
            if (!detail::check_every(file, line, interval_ms)) {
                skip_print_ = true;
            }
            return *this;
        }

        // Conditional print - only prints if condition is true
        log_proxy &when(bool condition) {
            if (!condition) {
                skip_print_ = true;
            }
            return *this;
        }

        // Print in place (clear line and print on same line)
        log_proxy &inplace() {
            inplace_ = true;
            return *this;
        }

#ifdef ECHO_LOG_ENABLED
        // Log to file (only available when log.hpp is included)
        log_proxy &log();
#endif

        // Destructor performs the actual logging
        ~log_proxy() {
            // Check if we should skip printing (e.g., from .once())
            if (skip_print_) {
                return;
            }

            if constexpr (static_cast<int>(L) >= static_cast<int>(detail::ACTIVE_LEVEL)) {
                // Runtime level check
                if (static_cast<int>(L) < static_cast<int>(detail::get_effective_level())) {
                    return;
                }

                std::lock_guard<std::mutex> lock(detail::get_log_mutex());
                std::ostream &out = (L >= Level::Error) ? std::cerr : std::cout;

                // Clear line if inplace
                if (inplace_) {
                    out << "\r\033[K"; // \r = carriage return, \033[K = clear to end of line
                }

#ifdef ECHO_ENABLE_TIMESTAMP
                out << "[" << detail::get_timestamp() << "]";
#endif
                out << detail::level_color(L) << "[" << detail::level_name(L) << "]" << detail::RESET << " ";

                if (!color_code_.empty()) {
                    out << color_code_ << message_ << detail::RESET;
                } else {
                    out << message_;
                }

                // Only add newline if not inplace
                if (!inplace_) {
                    out << "\n";
                }
                out << std::flush; // Always flush for inplace to work
            }
        }
    };

    // =================================================================================================
    // Simple print proxy (no log level, always prints)
    // =================================================================================================

    /**
     * @brief Proxy object for simple printing without log levels
     *
     * Allows: echo("message").red()
     */
    class print_proxy {
      private:
        std::string message_;
        std::string color_code_;
        bool skip_print_ = false;
        bool inplace_ = false;

#ifdef ECHO_LOG_ENABLED
        // Friend declaration for log.hpp to access private members
        friend struct log::detail::file_writer_print;
#endif

      public:
        template <typename... Args> print_proxy(const Args &...args) {
            std::ostringstream oss;
            detail::append_args(oss, args...);
            message_ = oss.str();
        }

        // Move semantics - allow moving but prevent copying
        print_proxy(print_proxy &&other) noexcept
            : message_(std::move(other.message_)), color_code_(std::move(other.color_code_)),
              skip_print_(other.skip_print_), inplace_(other.inplace_) {
            other.skip_print_ = true; // Prevent moved-from object from printing
        }

        print_proxy &operator=(print_proxy &&other) noexcept {
            if (this != &other) {
                message_ = std::move(other.message_);
                color_code_ = std::move(other.color_code_);
                skip_print_ = other.skip_print_;
                inplace_ = other.inplace_;
                other.skip_print_ = true;
            }
            return *this;
        }

        // Prevent copying
        print_proxy(const print_proxy &) = delete;
        print_proxy &operator=(const print_proxy &) = delete;

        // Color methods (24-bit RGB)
        print_proxy &red() {
            color_code_ = "\033[38;2;255;0;0m";
            return *this;
        }
        print_proxy &green() {
            color_code_ = "\033[38;2;0;255;0m";
            return *this;
        }
        print_proxy &yellow() {
            color_code_ = "\033[38;2;255;255;0m";
            return *this;
        }
        print_proxy &blue() {
            color_code_ = "\033[38;2;0;0;255m";
            return *this;
        }
        print_proxy &magenta() {
            color_code_ = "\033[38;2;255;0;255m";
            return *this;
        }
        print_proxy &cyan() {
            color_code_ = "\033[38;2;0;255;255m";
            return *this;
        }
        print_proxy &white() {
            color_code_ = "\033[38;2;255;255;255m";
            return *this;
        }
        print_proxy &gray() {
            color_code_ = "\033[38;2;128;128;128m";
            return *this;
        }
        print_proxy &bold() {
            color_code_ += "\033[1m";
            return *this;
        }
        print_proxy &italic() {
            color_code_ += "\033[3m";
            return *this;
        }
        print_proxy &underline() {
            color_code_ += "\033[4m";
            return *this;
        }

        // Custom hex color
        print_proxy &hex(const std::string &hex_color) {
            try {
                if (hex_color.length() >= 6) {
                    std::string h = hex_color;
                    if (h[0] == '#')
                        h = h.substr(1);
                    if (h.length() == 6) {
                        int r = std::stoi(h.substr(0, 2), nullptr, 16);
                        int g = std::stoi(h.substr(2, 2), nullptr, 16);
                        int b = std::stoi(h.substr(4, 2), nullptr, 16);
                        color_code_ =
                            "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
                    }
                }
            } catch (const std::exception &) {
                // Invalid hex color, leave color_code_ unchanged
            }
            return *this;
        }

        // Custom RGB color
        print_proxy &rgb(int r, int g, int b) {
            color_code_ = "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
            return *this;
        }

        // Print only once (internal - use ONCE macro instead)
        print_proxy &once_impl(const char *file, int line) {
            if (!detail::check_and_mark_once(file, line)) {
                skip_print_ = true;
            }
            return *this;
        }

        // Print at most every N milliseconds (internal - use EVERY macro instead)
        print_proxy &every_impl(const char *file, int line, int64_t interval_ms) {
            if (!detail::check_every(file, line, interval_ms)) {
                skip_print_ = true;
            }
            return *this;
        }

        // Conditional print - only prints if condition is true
        print_proxy &when(bool condition) {
            if (!condition) {
                skip_print_ = true;
            }
            return *this;
        }

        // Print in place (clear line and print on same line)
        print_proxy &inplace() {
            inplace_ = true;
            return *this;
        }

#ifdef ECHO_LOG_ENABLED
        // Log to file (only available when log.hpp is included)
        print_proxy &log();
#endif

        // Destructor performs the actual printing
        ~print_proxy() {
            // Check if we should skip printing (e.g., from .once())
            if (skip_print_) {
                return;
            }

            std::lock_guard<std::mutex> lock(detail::get_log_mutex());

            // Clear line if inplace
            if (inplace_) {
                std::cout << "\r\033[K"; // \r = carriage return, \033[K = clear to end of line
            }

            if (!color_code_.empty()) {
                std::cout << color_code_ << message_ << detail::RESET;
            } else {
                std::cout << message_;
            }

            // Only add newline if not inplace
            if (!inplace_) {
                std::cout << "\n";
            }
            std::cout << std::flush; // Always flush for inplace to work
        }
    };

    // =================================================================================================
    // Public logging functions
    // =================================================================================================

    template <typename... Args> inline log_proxy<Level::Trace> trace(const Args &...args) {
        return log_proxy<Level::Trace>(args...);
    }

    template <typename... Args> inline log_proxy<Level::Debug> debug(const Args &...args) {
        return log_proxy<Level::Debug>(args...);
    }

    template <typename... Args> inline log_proxy<Level::Info> info(const Args &...args) {
        return log_proxy<Level::Info>(args...);
    }

    template <typename... Args> inline log_proxy<Level::Warn> warn(const Args &...args) {
        return log_proxy<Level::Warn>(args...);
    }

    template <typename... Args> inline log_proxy<Level::Error> error(const Args &...args) {
        return log_proxy<Level::Error>(args...);
    }

    template <typename... Args> inline log_proxy<Level::Critical> critical(const Args &...args) {
        return log_proxy<Level::Critical>(args...);
    }

    // =================================================================================================
    // Simple echo function (inside namespace)
    // =================================================================================================

    /**
     * @brief Simple print function without log levels
     *
     * Usage (inside namespace): echo("message").red()
     * Usage (global): ::echo("message").red()
     */
    template <typename... Args> inline print_proxy print(const Args &...args) { return print_proxy(args...); }

    // =================================================================================================
    // File logging support (only when log.hpp is included)
    // =================================================================================================

#ifdef ECHO_LOG_ENABLED
    namespace log::detail {
        // Forward declarations for file writing functions
        template <Level L> void write_to_file(log_proxy<L> &proxy, Level level);
        void write_to_file(print_proxy &proxy);
    } // namespace log::detail

    // Implementation of log_proxy::log()
    template <Level L> inline log_proxy<L> &log_proxy<L>::log() {
        log::detail::write_to_file(*this, L);
        return *this;
    }

    // Implementation of print_proxy::log()
    inline print_proxy &print_proxy::log() {
        log::detail::write_to_file(*this);
        return *this;
    }
#endif

} // namespace echo

// =================================================================================================
// Global echo() macro (can be used without namespace prefix)
// =================================================================================================

/**
 * @brief Simple print function without log levels (global scope)
 *
 * Usage: echo("message").red()
 *
 * This allows using echo() without the echo:: prefix while still having
 * access to echo::info(), echo::debug(), etc.
 */
#define echo(...) echo::print_proxy(__VA_ARGS__)

// =================================================================================================
// Compile-out debug macros for release builds
// These macros completely eliminate debug/trace logging overhead in release builds
// =================================================================================================

#ifdef NDEBUG
/**
 * @brief Debug logging macro that compiles to nothing in release builds
 * Usage: ECHO_DEBUG("Debug message: ", value);
 */
#define ECHO_DEBUG(...) ((void)0)

/**
 * @brief Trace logging macro that compiles to nothing in release builds
 * Usage: ECHO_TRACE("Trace message: ", value);
 */
#define ECHO_TRACE(...) ((void)0)
#else
#define ECHO_DEBUG(...) echo::debug(__VA_ARGS__)
#define ECHO_TRACE(...) echo::trace(__VA_ARGS__)
#endif

// =================================================================================================
// C++20 source_location support (optional)
// =================================================================================================

#if __cplusplus >= 202002L && __has_include(<source_location>)
#include <source_location>

namespace echo {
    /**
     * @brief Log with automatic source location (C++20 only)
     * @param msg Message to log
     * @param loc Source location (auto-captured)
     */
    template <Level L>
    inline void log_with_location(const std::string &msg,
                                  const std::source_location &loc = std::source_location::current()) {
        if constexpr (static_cast<int>(L) >= static_cast<int>(detail::ACTIVE_LEVEL)) {
            if (static_cast<int>(L) >= static_cast<int>(detail::get_effective_level())) {
                std::lock_guard<std::mutex> lock(detail::get_log_mutex());
                std::ostream &out = (L >= Level::Error) ? std::cerr : std::cout;
                out << detail::level_color(L) << "[" << detail::level_name(L) << "]" << detail::RESET << " " << msg
                    << " [" << loc.file_name() << ":" << loc.line() << " " << loc.function_name() << "]\n";
            }
        }
    }
} // namespace echo
#endif
