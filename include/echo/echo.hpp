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
 * Log level control (two ways):
 *
 *   1. Compile-time via build system:
 *      -DLOGLEVEL=Trace|Debug|Info|Warn|Error|Critical
 *
 *   2. In-file before including (overrides build system):
 *      #define LOGLEVEL Trace
 *      #include <echo/echo.hpp>
 *
 * Supports logging of:
 *   - Anything convertible to string (via operator<<)
 *   - Objects with pretty_print() method (preferred)
 *   - Objects with print() method
 */

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

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
                std::ostringstream oss;
                append_args(oss, args...);

                std::ostream &out = (L >= Level::Error) ? std::cerr : std::cout;
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

} // namespace echo
