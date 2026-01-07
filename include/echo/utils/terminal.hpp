#pragma once

/**
 * @file utils/terminal.hpp
 * @brief Terminal detection and utilities
 */

#include <cstdlib>
#include <string>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#else
#include <io.h>
#include <windows.h>
#endif

namespace echo {
    namespace detail {

        // =================================================================================================
        // Terminal Width Detection
        // =================================================================================================

        /**
         * @brief Get terminal width in columns
         * @return Terminal width (defaults to 80 if detection fails)
         */
        [[nodiscard]] inline int get_terminal_width() noexcept {
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
                return csbi.srWindow.Right - csbi.srWindow.Left + 1;
            }
            return 80; // Fallback
#else
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
                return w.ws_col;
            }
            return 80; // Fallback
#endif
        }

        // =================================================================================================
        // TTY Detection
        // =================================================================================================

        /**
         * @brief Check if stdout is connected to a terminal (TTY)
         * @return true if stdout is a TTY
         *
         * This is useful for determining whether to use colors or not.
         * When output is redirected to a file or pipe, this returns false.
         */
        [[nodiscard]] inline bool is_stdout_tty() noexcept {
#ifdef _WIN32
            return _isatty(_fileno(stdout)) != 0;
#else
            return isatty(STDOUT_FILENO) != 0;
#endif
        }

        /**
         * @brief Check if stderr is connected to a terminal (TTY)
         * @return true if stderr is a TTY
         */
        [[nodiscard]] inline bool is_stderr_tty() noexcept {
#ifdef _WIN32
            return _isatty(_fileno(stderr)) != 0;
#else
            return isatty(STDERR_FILENO) != 0;
#endif
        }

        /**
         * @brief Check if colors should be enabled
         * @return true if colors should be used
         *
         * Colors are enabled if:
         * - Output is a TTY
         * - NO_COLOR environment variable is not set
         * - TERM is not "dumb"
         */
        [[nodiscard]] inline bool should_use_colors() noexcept {
            // Check NO_COLOR environment variable (standard)
            if (std::getenv("NO_COLOR") != nullptr) {
                return false;
            }

            // Check TERM environment variable
            const char *term = std::getenv("TERM");
            if (term && std::string(term) == "dumb") {
                return false;
            }

            // Check if stdout is a TTY
            return is_stdout_tty();
        }

    } // namespace detail
} // namespace echo
