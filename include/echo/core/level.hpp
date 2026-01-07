#pragma once

/**
 * @file core/level.hpp
 * @brief Log level definitions and parsing
 */

#include <cstdlib>
#include <string>

namespace echo {

    // =================================================================================================
    // Log levels
    // =================================================================================================

    enum class Level { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Critical = 5, Off = 6 };

    namespace detail {

        // =================================================================================================
        // Log level parsing
        // =================================================================================================

        inline constexpr Level parse_level() {
#if defined(LOGLEVEL) || defined(ECHOLEVEL)
#ifdef LOGLEVEL
#define ECHO_STRINGIFY(x) #x
#define ECHO_TOSTRING(x) ECHO_STRINGIFY(x)
#define ECHO_LOGLEVEL_STR ECHO_TOSTRING(LOGLEVEL)
#elif defined(ECHOLEVEL)
#define ECHO_STRINGIFY(x) #x
#define ECHO_TOSTRING(x) ECHO_STRINGIFY(x)
#define ECHO_LOGLEVEL_STR ECHO_TOSTRING(ECHOLEVEL)
#endif
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

        inline Level &get_runtime_level() noexcept {
            static Level runtime_level = init_runtime_level();
            return runtime_level;
        }

        [[nodiscard]] inline Level get_effective_level() noexcept {
            Level runtime = get_runtime_level();
            return (runtime == Level::Off) ? ACTIVE_LEVEL : runtime;
        }

        // =================================================================================================
        // ANSI color codes for levels
        // =================================================================================================

        [[nodiscard]] inline const char *level_color(Level level) noexcept {
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

        [[nodiscard]] inline const char *level_name(Level level) noexcept {
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

    } // namespace detail

    // =================================================================================================
    // Runtime log level control (public API)
    // =================================================================================================

    inline void set_level(Level level) noexcept { detail::get_runtime_level() = level; }

    [[nodiscard]] inline Level get_level() noexcept { return detail::get_effective_level(); }

    [[nodiscard]] inline constexpr Level current_level() noexcept { return detail::ACTIVE_LEVEL; }

    template <Level L> [[nodiscard]] inline constexpr bool is_enabled() noexcept {
        return static_cast<int>(L) >= static_cast<int>(detail::ACTIVE_LEVEL);
    }

} // namespace echo
