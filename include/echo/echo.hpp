#pragma once

/**
 * @file echo.hpp
 * @brief A minimal, header-only logging library for C++
 *
 * Usage:
 *   #include <echo/echo.hpp>
 *
 *   // Simple printing (no log levels, always shows, no prefix)
 *   echo("Hello, world!");
 *   echo("Colored text").red();
 *   echo("Custom color").hex("#FF5733").bold();
 *
 *   // Logging with levels (shows [level] prefix, respects log level filtering)
 *   echo::info("Hello, world!");
 *   echo::debug("Value: ", 42);
 *   echo::info("Colored message").red();
 *   echo::warn("Custom color").hex("#FF5733");
 *   echo::error("RGB color").rgb(255, 87, 51).bold();
 *
 * Log level control:
 *
 *   1. Compile-time via build system (disables env var):
 *      -DLOGLEVEL=Trace|Debug|Info|Warn|Error|Critical
 *      -DECHOLEVEL=Trace|Debug|Info|Warn|Error|Critical
 *      (Both are supported; LOGLEVEL takes precedence if both are defined)
 *      (When set, environment variables are ignored)
 *
 *   2. In-file before including (overrides build system, disables env var):
 *      #define LOGLEVEL Trace
 *      #include <echo/echo.hpp>
 *      or
 *      #define ECHOLEVEL Trace
 *      #include <echo/echo.hpp>
 *
 *   3. Environment variable (only when no compile-time level set):
 *      export LOGLEVEL=Debug
 *      export ECHOLEVEL=Trace
 *      (Both are supported; LOGLEVEL takes precedence if both are set)
 *      (Only works if no -DLOGLEVEL/-DECHOLEVEL was specified)
 *
 *   4. Runtime control (always available):
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
 * Fluent interface with colors:
 *   - Named colors: .red(), .green(), .blue(), .yellow(), .cyan(), .magenta(), .white(), .gray()
 *   - Custom hex: .hex("#FF5733") or .hex("FF5733")
 *   - Custom RGB: .rgb(255, 87, 51)
 *   - Modifiers: .bold(), .italic(), .underline()
 *   - Print once: .once() - prints only the first time (useful in loops)
 *   - Print every N ms: .every(1000) - rate-limits to at most once per interval
 *   - Conditional print: .when(condition) - prints only if condition is true
 *   - Print in place: .inplace() - clears line and prints on same line (for updates)
 *   - Chaining: echo::info("message").red().bold().when(i % 10 == 0).every(500)
 *
 * Supports logging of:
 *   - Anything convertible to string (via operator<<)
 *   - Objects with pretty_print() method (preferred)
 *   - Objects with print() method
 */

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#ifdef ECHO_ENABLE_TIMESTAMP
#include <iomanip>

#endif
namespace echo {

    // =================================================================================================
    // Log levels
    // =================================================================================================

    enum class Level { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Critical = 5, Off = 6 };

#ifdef ECHO_LOG_ENABLED
    // Forward declarations for file logging (defined in log.hpp)
    namespace log::detail {
        template <Level L> struct file_writer;
        struct file_writer_print;
    } // namespace log::detail
#endif

    // =================================================================================================
    // Compile-time log level configuration
    // =================================================================================================

// Accept both LOGLEVEL and ECHOLEVEL, with LOGLEVEL taking precedence
#if defined(LOGLEVEL)
#define ECHO_STRINGIFY(x) #x
#define ECHO_TOSTRING(x) ECHO_STRINGIFY(x)
#define ECHO_LOGLEVEL_STR ECHO_TOSTRING(LOGLEVEL)
#elif defined(ECHOLEVEL)
#define ECHO_STRINGIFY(x) #x
#define ECHO_TOSTRING(x) ECHO_STRINGIFY(x)
#define ECHO_LOGLEVEL_STR ECHO_TOSTRING(ECHOLEVEL)
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
        // Once tracking (for .once() functionality)
        // =================================================================================================

        inline std::unordered_set<std::string> &get_once_set() {
            static std::unordered_set<std::string> once_set;
            return once_set;
        }

        inline bool check_and_mark_once(const char *file, int line) {
            std::string key = std::string(file) + ":" + std::to_string(line);
            std::lock_guard<std::mutex> lock(get_log_mutex());
            if (get_once_set().count(key)) {
                return false; // Already printed
            }
            get_once_set().insert(key);
            return true; // First time
        }

        // =================================================================================================
        // Every tracking (for .every() functionality - time-based throttling)
        // =================================================================================================

        using time_point = std::chrono::steady_clock::time_point;

        inline std::unordered_map<std::string, time_point> &get_every_map() {
            static std::unordered_map<std::string, time_point> every_map;
            return every_map;
        }

        inline bool check_every(const char *file, int line, int64_t interval_ms) {
            std::string key = std::string(file) + ":" + std::to_string(line);
            auto now = std::chrono::steady_clock::now();

            std::lock_guard<std::mutex> lock(get_log_mutex());
            auto &every_map = get_every_map();
            auto it = every_map.find(key);

            if (it == every_map.end()) {
                every_map[key] = now;
                return true; // First time, always print
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
            if (elapsed >= interval_ms) {
                it->second = now;
                return true; // Enough time has passed
            }

            return false; // Not enough time has passed
        }

        // =================================================================================================
        // Log level parsing
        // =================================================================================================

        inline constexpr Level parse_level() {
#if defined(LOGLEVEL) || defined(ECHOLEVEL)
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
#else
            // When no compile-time level is set, use Trace to allow runtime/env control
            return Level::Trace;
#endif
            return Level::Info; // Fallback (should never reach here)
        }

        inline constexpr Level ACTIVE_LEVEL = parse_level();

        // =================================================================================================
        // Runtime log level control
        // =================================================================================================

        // Parse level from string (case-insensitive)
        inline Level parse_level_from_string(const char *str) {
            if (!str || str[0] == '\0')
                return Level::Off;

            char first = str[0];
            // Convert to lowercase for comparison
            if (first >= 'A' && first <= 'Z')
                first = first + ('a' - 'A');

            switch (first) {
            case 't':
                return Level::Trace;
            case 'd':
                return Level::Debug;
            case 'i':
                return Level::Info;
            case 'w':
                return Level::Warn;
            case 'e':
                return Level::Error;
            case 'c':
                return Level::Critical;
            case 'o':
                return Level::Off;
            default:
                return Level::Off;
            }
        }

        // Initialize runtime level from environment variable
        // Only used when no compile-time level is set
        inline Level init_runtime_level() {
#if !defined(LOGLEVEL) && !defined(ECHOLEVEL)
            // Only check environment variables if no compile-time level was set
            const char *env_level = std::getenv("LOGLEVEL");
            if (!env_level) {
                env_level = std::getenv("ECHOLEVEL");
            }

            if (env_level) {
                Level parsed = parse_level_from_string(env_level);
                if (parsed != Level::Off) {
                    return parsed;
                }
            }
            // Default to Info when no env var is set and no compile-time level
            return Level::Info;
#else
            // When compile-time level is set, use Off to defer to ACTIVE_LEVEL
            return Level::Off;
#endif
        }

        inline Level &get_runtime_level() {
            static Level runtime_level = init_runtime_level();
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
                return "\033[38;2;128;128;128m\033[1m"; // Bold Gray (RGB: 128,128,128)
            case Level::Debug:
                return "\033[38;2;0;255;255m\033[1m"; // Bold Cyan (RGB: 0,255,255)
            case Level::Info:
                return "\033[38;2;0;255;0m\033[1m"; // Bold Green (RGB: 0,255,0)
            case Level::Warn:
                return "\033[38;2;255;255;0m\033[1m"; // Bold Yellow (RGB: 255,255,0)
            case Level::Error:
                return "\033[38;2;255;0;0m\033[1m"; // Bold Red (RGB: 255,0,0)
            case Level::Critical:
                return "\033[38;2;255;0;255m\033[1m"; // Bold Magenta (RGB: 255,0,255)
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
// Use the color utilities from color.hpp if available
#ifdef ECHO_COLOR_HPP
            color_code_ = detail::rgb_to_ansi(detail::hex_to_rgb(hex_color));
#else
            // Fallback: parse hex manually
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
#endif
            return *this;
        }

        // Custom RGB color
        log_proxy &rgb(int r, int g, int b) {
            color_code_ = "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
            return *this;
        }

        // Print only once (internal - use ONCE macro instead)
        log_proxy &once_impl(const char *file, int line) {
            std::string key = std::string(file) + ":" + std::to_string(line);
            std::lock_guard<std::mutex> lock(detail::get_log_mutex());
            static std::unordered_set<std::string> once_set;
            if (once_set.count(key)) {
                skip_print_ = true;
            } else {
                once_set.insert(key);
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
            return *this;
        }

        // Custom RGB color
        print_proxy &rgb(int r, int g, int b) {
            color_code_ = "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
            return *this;
        }

        // Print only once (internal - use ONCE macro instead)
        print_proxy &once_impl(const char *file, int line) {
            std::string key = std::string(file) + ":" + std::to_string(line);
            std::lock_guard<std::mutex> lock(detail::get_log_mutex());
            static std::unordered_set<std::string> once_set;
            if (once_set.count(key)) {
                skip_print_ = true;
            } else {
                once_set.insert(key);
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
// .once() macro helper (captures call site location)
// =================================================================================================

/**
 * @brief Helper macro for .once() that captures file and line
 *
 * This is a workaround because we can't get __FILE__ and __LINE__ inside a method.
 * The macro intercepts the call and injects the location information.
 */
#define once() once_impl(__FILE__, __LINE__)

/**
 * @brief Helper macro for .every(ms) that captures file and line
 *
 * Prints at most once every N milliseconds. Useful for rate-limiting logs in tight loops.
 * Usage: echo::info("Status update").every(1000)  // prints at most once per second
 */
#define every(ms) every_impl(__FILE__, __LINE__, ms)
