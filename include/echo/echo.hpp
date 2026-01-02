#pragma once

/**
 * @file echo.hpp
 * @brief A minimal, header-only logging library for C++
 *
 * Usage:
 *   #include <echo/echo.hpp>
 *   echo::info("Hello, world!");
 *   echo::debug("Value: ", 42);
 *
 * Log level control:
 *
 *   1. Compile-time via build system:
 *      -DLOGLEVEL=Trace|Debug|Info|Warn|Error|Critical
 *
 *   2. In-file before including (overrides build system):
 *      #define LOGLEVEL Trace
 *      #include <echo/echo.hpp>
 *
 *   3. Runtime control:
 *      echo::set_level(echo::Level::Debug);
 *      auto level = echo::get_level();
 *
 * Timestamp support:
 *   - Enable timestamps with: #define ECHO_ENABLE_TIMESTAMP
 *   - Format: [HH:MM:SS][level] Message
 *   - Default: disabled (no timestamp)
 *
 * Thread safety:
 *   - Thread-safe by default (uses mutex)
 *   - Multiple threads can log concurrently without message corruption
 *
 * Structured logging:
 *   - Use kv() for key-value pairs: echo::info("Login: ", kv("user", "john", "age", 25))
 *   - Output format: key=value key2=value2
 *
 * Separators:
 *   - echo::separator() - full-width line
 *   - echo::separator("text") - centered text with separators
 *   - echo::separator("text", '=') - custom separator character
 *
 * Supports logging of:
 *   - Anything convertible to string (via operator<<)
 *   - Objects with pretty_print() method (preferred)
 *   - Objects with print() method
 */

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>

#ifdef ECHO_ENABLE_TIMESTAMP
#include <chrono>
#include <iomanip>
#endif

// Terminal width detection (Unix/Linux)
#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace echo {

    // =================================================================================================
    // Log levels
    // =================================================================================================

    enum class Level { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Critical = 5, Off = 6 };

    // =================================================================================================
    // Compile-time log level configuration
    // =================================================================================================

#if defined(LOGLEVEL)
#define ECHO_STRINGIFY(x) #x
#define ECHO_TOSTRING(x) ECHO_STRINGIFY(x)
#define ECHO_LOGLEVEL_STR ECHO_TOSTRING(LOGLEVEL)
#endif

    namespace detail {

        // =================================================================================================
        // Thread safety
        // =================================================================================================

        inline std::mutex &get_log_mutex() {
            static std::mutex log_mutex;
            return log_mutex;
        }

        // =================================================================================================
        // Log level parsing
        // =================================================================================================

        inline constexpr Level parse_level() {
#if defined(LOGLEVEL)
            constexpr const char *level_str = ECHO_LOGLEVEL_STR;
            if (level_str[0] == 'T' || level_str[0] == 't')
                return Level::Trace;
            if (level_str[0] == 'D' || level_str[0] == 'd')
                return Level::Debug;
            if (level_str[0] == 'I' || level_str[0] == 'i')
                return Level::Info;
            if (level_str[0] == 'W' || level_str[0] == 'w')
                return Level::Warn;
            if (level_str[0] == 'E' || level_str[0] == 'e')
                return Level::Error;
            if (level_str[0] == 'C' || level_str[0] == 'c')
                return Level::Critical;
            if (level_str[0] == 'O' || level_str[0] == 'o')
                return Level::Off;
#endif
            return Level::Info; // Default level
        }

        inline constexpr Level ACTIVE_LEVEL = parse_level();

        // =================================================================================================
        // Runtime log level control
        // =================================================================================================

        inline Level &get_runtime_level() {
            static Level runtime_level = Level::Off; // Off means "use compile-time level"
            return runtime_level;
        }

        inline Level get_effective_level() {
            Level runtime = get_runtime_level();
            return (runtime == Level::Off) ? ACTIVE_LEVEL : runtime;
        }

        // =================================================================================================
        // Type traits for detecting print methods
        // =================================================================================================

        template <typename T, typename = void> struct has_pretty : std::false_type {};

        template <typename T>
        struct has_pretty<T, std::void_t<decltype(std::declval<T>().pretty())>> : std::true_type {};

        template <typename T, typename = void> struct has_print : std::false_type {};

        template <typename T> struct has_print<T, std::void_t<decltype(std::declval<T>().print())>> : std::true_type {};

        template <typename T, typename = void> struct has_to_string : std::false_type {};

        template <typename T>
        struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

        template <typename T, typename = void> struct is_streamable : std::false_type {};

        template <typename T>
        struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
            : std::true_type {};

        // =================================================================================================
        // Convert value to string
        // =================================================================================================

        template <typename T> inline std::string stringify(const T &value) {
            if constexpr (has_pretty<T>::value) {
                return value.pretty();
            } else if constexpr (has_print<T>::value) {
                return value.print();
            } else if constexpr (has_to_string<T>::value) {
                return value.to_string();
            } else if constexpr (std::is_same_v<T, std::string>) {
                return value;
            } else if constexpr (std::is_same_v<T, const char *> || std::is_same_v<T, char *>) {
                return std::string(value);
            } else if constexpr (is_streamable<T>::value) {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            } else {
                return "[unprintable]";
            }
        }

        // =================================================================================================
        // ANSI color codes
        // =================================================================================================

        inline const char *level_color(Level level) {
            switch (level) {
            case Level::Trace:
                return "\033[90m"; // Gray
            case Level::Debug:
                return "\033[36m"; // Cyan
            case Level::Info:
                return "\033[32m"; // Green
            case Level::Warn:
                return "\033[33m"; // Yellow
            case Level::Error:
                return "\033[31m"; // Red
            case Level::Critical:
                return "\033[35;1m"; // Bold Magenta
            default:
                return "\033[0m";
            }
        }

        inline const char *level_name(Level level) {
            switch (level) {
            case Level::Trace:
                return "trace";
            case Level::Debug:
                return "debug";
            case Level::Info:
                return "info";
            case Level::Warn:
                return "warning";
            case Level::Error:
                return "error";
            case Level::Critical:
                return "critical";
            default:
                return "unknown";
            }
        }

        constexpr const char *RESET = "\033[0m";

        // =================================================================================================
        // Terminal width detection
        // =================================================================================================

        inline int get_terminal_width() {
#ifdef _WIN32
            // Windows implementation would go here
            // CONSOLE_SCREEN_BUFFER_INFO csbi;
            // if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            //     return csbi.srWindow.Right - csbi.srWindow.Left + 1;
            // }
            return 80; // Fallback for Windows
#else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
                return w.ws_col;
            }
            return 80; // Fallback
#endif
        }

        // =================================================================================================
        // Timestamp support
        // =================================================================================================

#ifdef ECHO_ENABLE_TIMESTAMP
        inline std::string get_timestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto tm = *std::localtime(&time);

            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":" << std::setw(2) << tm.tm_min << ":"
                << std::setw(2) << tm.tm_sec;
            return oss.str();
        }
#endif

        // =================================================================================================
        // Core logging implementation
        // =================================================================================================

        inline void append_args(std::ostringstream &) {}

        template <typename T, typename... Args>
        inline void append_args(std::ostringstream &oss, const T &first, const Args &...rest) {
            oss << stringify(first);
            append_args(oss, rest...);
        }

        template <Level L, typename... Args> inline void log(const Args &...args) {
            if constexpr (static_cast<int>(L) >= static_cast<int>(ACTIVE_LEVEL)) {
                // Runtime level check
                if (static_cast<int>(L) < static_cast<int>(get_effective_level())) {
                    return;
                }

                std::ostringstream oss;
                append_args(oss, args...);

                std::lock_guard<std::mutex> lock(get_log_mutex());
                std::ostream &out = (L >= Level::Error) ? std::cerr : std::cout;
#ifdef ECHO_ENABLE_TIMESTAMP
                out << "[" << get_timestamp() << "]";
#endif
                out << level_color(L) << "[" << level_name(L) << "]" << RESET << " " << oss.str() << "\n";
            }
        }

    } // namespace detail

    // =================================================================================================
    // Public logging functions
    // =================================================================================================

    template <typename... Args> inline void trace(const Args &...args) { detail::log<Level::Trace>(args...); }

    template <typename... Args> inline void debug(const Args &...args) { detail::log<Level::Debug>(args...); }

    template <typename... Args> inline void info(const Args &...args) { detail::log<Level::Info>(args...); }

    template <typename... Args> inline void warn(const Args &...args) { detail::log<Level::Warn>(args...); }

    template <typename... Args> inline void error(const Args &...args) { detail::log<Level::Error>(args...); }

    template <typename... Args> inline void critical(const Args &...args) { detail::log<Level::Critical>(args...); }

    // =================================================================================================
    // Utility: Get/check current log level
    // =================================================================================================

    inline constexpr Level current_level() { return detail::ACTIVE_LEVEL; }

    template <Level L> inline constexpr bool is_enabled() {
        return static_cast<int>(L) >= static_cast<int>(detail::ACTIVE_LEVEL);
    }

    // =================================================================================================
    // Runtime log level control
    // =================================================================================================

    inline void set_level(Level level) { detail::get_runtime_level() = level; }

    inline Level get_level() { return detail::get_effective_level(); }

    // =================================================================================================
    // Structured logging (key-value pairs)
    // =================================================================================================

    namespace detail {
        // Base case: no more arguments
        inline void append_kv(std::ostringstream &) {}

        // Recursive case: key-value pairs
        template <typename K, typename V, typename... Rest>
        inline void append_kv(std::ostringstream &oss, const K &key, const V &value, const Rest &...rest) {
            oss << stringify(key) << "=" << stringify(value);
            if constexpr (sizeof...(rest) > 0) {
                oss << " ";
                append_kv(oss, rest...);
            }
        }
    } // namespace detail

    template <typename... Args> inline std::string kv(const Args &...args) {
        static_assert(sizeof...(args) % 2 == 0, "kv() requires an even number of arguments (key-value pairs)");
        std::ostringstream oss;
        detail::append_kv(oss, args...);
        return oss.str();
    }

    // =================================================================================================
    // Separator functions
    // =================================================================================================

    inline void separator(const std::string &text = "", char sep_char = '-') {
        int width = detail::get_terminal_width();

        if (text.empty()) {
            // Just print a full line of separators
            std::cout << std::string(width, sep_char) << "\n";
        } else {
            // Format: ----[ text ]----
            int text_len = text.length() + 4; // "[ text ]" = text + 4
            if (text_len >= width) {
                // Text too long, just print it with minimal separators
                std::cout << sep_char << "[ " << text << " ]" << sep_char << "\n";
            } else {
                int total_sep = width - text_len;
                int left_sep = total_sep / 2;
                int right_sep = total_sep - left_sep; // Handle odd widths

                std::cout << std::string(left_sep, sep_char) << "[ " << text << " ]" << std::string(right_sep, sep_char)
                          << "\n";
            }
        }
    }

} // namespace echo
