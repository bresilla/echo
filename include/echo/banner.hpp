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
 *   // Separators
 *   echo::separator();                    // Full-width line
 *   echo::separator("Section 1");         // Centered text
 *   echo::separator("IMPORTANT", '=');    // Custom character
 *
 *   // Boxes
 *   echo::box("Hello World");             // Box with single lines
 *   echo::box("Title", BoxStyle::Double); // Box with double lines
 *   echo::box("Info", BoxStyle::Rounded); // Box with rounded corners
 *   echo::box("Alert", BoxStyle::Heavy);  // Box with heavy lines
 *   echo::box("Note", BoxStyle::Dashed);  // Box with dashed lines
 *   echo::box("Plain", BoxStyle::ASCII);  // Box with ASCII characters
 *
 *   // Headers and Titles
 *   echo::header("Application Started");  // Fancy header with double lines
 *   echo::title("My Application");        // Centered title with borders
 *   echo::title("Section", '-');          // Custom border character
 *
 *   // Banners
 *   echo::banner("WELCOME");              // Large decorative banner
 *   echo::banner("ERROR", BoxStyle::Double); // Banner with custom style
 *
 * Features:
 *   - Terminal-aware separators (auto-detects width)
 *   - Centered text with customizable separator characters
 *   - Unicode box drawing with multiple styles (Single, Double, Rounded, Heavy, Dashed, ASCII)
 *   - Fancy headers and banners for visual organization
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

    // =================================================================================================
    // Box styles
    // =================================================================================================

    enum class BoxStyle {
        Single,  // ┌─┐ │ └─┘
        Double,  // ╔═╗ ║ ╚═╝
        Rounded, // ╭─╮ │ ╰─╯
        Heavy,   // ┏━┓ ┃ ┗━┛
        Dashed,  // ┏╍╍┓ ╏ ╏ ┗╍╍┛
        ASCII,   // +--+ | | +--+
    };

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

    // =================================================================================================
    // Box functions
    // =================================================================================================

    /**
     * @brief Draw a box around text
     * @param text Text to display in the box
     * @param style Box style (Single, Double, or Rounded)
     *
     * Examples:
     *   box("Hello");                    // ┌───────┐
     *                                    // │ Hello │
     *                                    // └───────┘
     *
     *   box("Title", BoxStyle::Double);  // ╔═══════╗
     *                                    // ║ Title ║
     *                                    // ╚═══════╝
     *
     *   box("Info", BoxStyle::Rounded);  // ╭──────╮
     *                                    // │ Info │
     *                                    // ╰──────╯
     */
    inline void box(const std::string &text, BoxStyle style = BoxStyle::Single) {
        // Box drawing characters for each style
        const char *top_left, *top_right, *bottom_left, *bottom_right;
        const char *horizontal, *vertical;

        switch (style) {
        case BoxStyle::Single:
            top_left = "┌";
            top_right = "┐";
            bottom_left = "└";
            bottom_right = "┘";
            horizontal = "─";
            vertical = "│";
            break;
        case BoxStyle::Double:
            top_left = "╔";
            top_right = "╗";
            bottom_left = "╚";
            bottom_right = "╝";
            horizontal = "═";
            vertical = "║";
            break;
        case BoxStyle::Rounded:
            top_left = "╭";
            top_right = "╮";
            bottom_left = "╰";
            bottom_right = "╯";
            horizontal = "─";
            vertical = "│";
            break;
        case BoxStyle::Heavy:
            top_left = "┏";
            top_right = "┓";
            bottom_left = "┗";
            bottom_right = "┛";
            horizontal = "━";
            vertical = "┃";
            break;
        case BoxStyle::Dashed:
            top_left = "┏";
            top_right = "┓";
            bottom_left = "┗";
            bottom_right = "┛";
            horizontal = "╍";
            vertical = "╏";
            break;
        case BoxStyle::ASCII:
            top_left = "+";
            top_right = "+";
            bottom_left = "+";
            bottom_right = "+";
            horizontal = "-";
            vertical = "|";
            break;
        }

        // Calculate box width (text + 2 spaces padding + 2 borders)
        int text_width = text.length();
        int box_width = text_width + 2; // 1 space on each side

        // Top border
        std::cout << top_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << horizontal;
        }
        std::cout << top_right << "\n";

        // Middle with text
        std::cout << vertical << " " << text << " " << vertical << "\n";

        // Bottom border
        std::cout << bottom_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << horizontal;
        }
        std::cout << bottom_right << "\n";
    }

    // =================================================================================================
    // Header function
    // =================================================================================================

    /**
     * @brief Print a fancy header with double lines
     * @param text Header text to display
     *
     * Example:
     *   header("Application Started");
     *   // ╔════════════════════════════════════════╗
     *   // ║       Application Started              ║
     *   // ╚════════════════════════════════════════╝
     */
    inline void header(const std::string &text) {
        int width = detail::get_terminal_width();
        int text_len = text.length();
        int padding = 2;               // 1 space on each side
        int content_width = width - 4; // Subtract 2 for borders and 2 for padding

        if (text_len > content_width) {
            content_width = text_len + padding;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        // Top border
        std::cout << "╔";
        for (int i = 0; i < content_width + padding; ++i) {
            std::cout << "═";
        }
        std::cout << "╗\n";

        // Middle with centered text
        std::cout << "║ " << std::string(left_pad, ' ') << text << std::string(right_pad, ' ') << " ║\n";

        // Bottom border
        std::cout << "╚";
        for (int i = 0; i < content_width + padding; ++i) {
            std::cout << "═";
        }
        std::cout << "╝\n";
    }

    // =================================================================================================
    // Title function
    // =================================================================================================

    /**
     * @brief Print a centered title with decorative borders
     * @param text Title text to display
     * @param border_char Character to use for borders (default: '=')
     *
     * Example:
     *   title("My Application");
     *   // ========================================
     *   //          My Application
     *   // ========================================
     */
    inline void title(const std::string &text, char border_char = '=') {
        int width = detail::get_terminal_width();
        int text_len = text.length();

        // Top border
        std::cout << std::string(width, border_char) << "\n";

        // Centered text
        if (text_len >= width) {
            std::cout << text << "\n";
        } else {
            int total_padding = width - text_len;
            int left_pad = total_padding / 2;
            std::cout << std::string(left_pad, ' ') << text << "\n";
        }

        // Bottom border
        std::cout << std::string(width, border_char) << "\n";
    }

    // =================================================================================================
    // Banner function
    // =================================================================================================

    /**
     * @brief Print a large decorative banner
     * @param text Banner text to display
     * @param style Box style for the banner (default: Heavy)
     *
     * Example:
     *   banner("WELCOME");
     *   // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
     *   // ┃                                        ┃
     *   // ┃              WELCOME                   ┃
     *   // ┃                                        ┃
     *   // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
     */
    inline void banner(const std::string &text, BoxStyle style = BoxStyle::Heavy) {
        // Box drawing characters for each style
        const char *top_left, *top_right, *bottom_left, *bottom_right;
        const char *horizontal, *vertical;

        switch (style) {
        case BoxStyle::Single:
            top_left = "┌";
            top_right = "┐";
            bottom_left = "└";
            bottom_right = "┘";
            horizontal = "─";
            vertical = "│";
            break;
        case BoxStyle::Double:
            top_left = "╔";
            top_right = "╗";
            bottom_left = "╚";
            bottom_right = "╝";
            horizontal = "═";
            vertical = "║";
            break;
        case BoxStyle::Rounded:
            top_left = "╭";
            top_right = "╮";
            bottom_left = "╰";
            bottom_right = "╯";
            horizontal = "─";
            vertical = "│";
            break;
        case BoxStyle::Heavy:
            top_left = "┏";
            top_right = "┓";
            bottom_left = "┗";
            bottom_right = "┛";
            horizontal = "━";
            vertical = "┃";
            break;
        case BoxStyle::Dashed:
            top_left = "┏";
            top_right = "┓";
            bottom_left = "┗";
            bottom_right = "┛";
            horizontal = "╍";
            vertical = "╏";
            break;
        case BoxStyle::ASCII:
            top_left = "+";
            top_right = "+";
            bottom_left = "+";
            bottom_right = "+";
            horizontal = "-";
            vertical = "|";
            break;
        }

        int width = detail::get_terminal_width();
        int text_len = text.length();
        int content_width = width - 4; // Subtract 2 for borders and 2 for padding

        if (text_len > content_width) {
            content_width = text_len + 4;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        // Top border
        std::cout << top_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << horizontal;
        }
        std::cout << top_right << "\n";

        // Empty line
        std::cout << vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << vertical << "\n";

        // Middle with centered text
        std::cout << vertical << " " << std::string(left_pad, ' ') << text << std::string(right_pad, ' ') << " "
                  << vertical << "\n";

        // Empty line
        std::cout << vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << vertical << "\n";

        // Bottom border
        std::cout << bottom_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << horizontal;
        }
        std::cout << bottom_right << "\n";
    }

} // namespace echo
