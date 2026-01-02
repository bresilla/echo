#pragma once

/**
 * @file banner.hpp
 * @brief Visual banners and separators for Echo logging library
 *
 * This header provides fancy visual elements for organizing log output.
 * It includes echo.hpp automatically, so you only need to include this file.
 *
 * Usage:
 *   #include <echo/banner.hpp>
 *
 *   echo::separator();                    // Full-width line
 *   echo::separator("Section 1");         // Centered text
 *   echo::separator("IMPORTANT", '=');    // Custom character
 *
 * Features:
 *   - Terminal-aware separators (auto-detects width)
 *   - Centered text with customizable separator characters
 *   - More fancy banners coming soon!
 */

#include <echo/echo.hpp>

#include <iostream>
#include <string>

// Terminal width detection (Unix/Linux)
#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace echo {

    namespace detail {

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

    } // namespace detail

    // =================================================================================================
    // Separator functions
    // =================================================================================================

    /**
     * @brief Print a separator line
     * @param text Optional text to center in the separator
     * @param sep_char Character to use for the separator (default: '-')
     *
     * Examples:
     *   separator();                  // ----------------------------------------
     *   separator("Section 1");       // ----------[ Section 1 ]----------
     *   separator("IMPORTANT", '=');  // ==========[ IMPORTANT ]==========
     */
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
