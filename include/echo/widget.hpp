#pragma once

/**
 * @file widget.hpp
 * @brief Visual widgets for Echo logging library (banners, separators, progress bars, spinners)
 *
 * This header provides fancy visual elements for organizing log output.
 * It includes echo.hpp automatically, so you only need to include this file.
 *
 * Usage:
 *   #include <echo/widget.hpp>
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
 *  // Progress bar
 *   echo::progress_bar bar(100);
 *   bar.tick();
 *   bar.set_progress(50);
 *   bar.finish();
 *
 *   // Spinner
 *   echo::spinner spin;
 *   spin.tick();
 *   spin.stop("Done!");
 *
 *   // Steps
 *   echo::steps workflow({"Init", "Load", "Process"});
 *   workflow.next();
 *   workflow.complete();
 *
 * Features:
 *   - Progress bars with percentage and time tracking
 *   - 15 ASCII/ANSI spinner styles
 *   - Step indicators (finite and infinite)
 *   - Thread-safe operations
 *   - Terminal-aware
 *   - HEX color support with gradients
 *   - Terminal-aware separators (auto-detects width)
 *   - Centered text with customizable separator characters
 *   - Unicode box drawing with multiple styles (Single, Double, Rounded, Heavy, Dashed, ASCII)
 *   - Fancy headers and banners for visual organization
 */

#include <echo/echo.hpp>
#include <echo/format.hpp>
#include <echo/utils/color.hpp>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

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

        /**
         * @brief Box drawing characters for different styles
         * Reduces code duplication across box, banner, header functions
         */
        struct BoxChars {
            const char *top_left;
            const char *top_right;
            const char *bottom_left;
            const char *bottom_right;
            const char *horizontal;
            const char *vertical;

            /**
             * @brief Get box characters for a given style
             * @param style The box style
             * @return BoxChars struct with appropriate characters
             */
            [[nodiscard]] static BoxChars get(BoxStyle style) noexcept {
                switch (style) {
                case BoxStyle::Single:
                    return {"┌", "┐", "└", "┘", "─", "│"};
                case BoxStyle::Double:
                    return {"╔", "╗", "╚", "╝", "═", "║"};
                case BoxStyle::Rounded:
                    return {"╭", "╮", "╰", "╯", "─", "│"};
                case BoxStyle::Heavy:
                    return {"┏", "┓", "┗", "┛", "━", "┃"};
                case BoxStyle::Dashed:
                    return {"┏", "┓", "┗", "┛", "╍", "╏"};
                case BoxStyle::ASCII:
                    return {"+", "+", "+", "+", "-", "|"};
                default:
                    return {"┌", "┐", "└", "┘", "─", "│"};
                }
            }
        };

    } // namespace detail

    // =================================================================================================
    // Separator functions
    // =================================================================================================

    /**
     * @brief Print a separator line
     * @param text Optional text to center in the separator
     * @param sep_char Character to use for the separator (default: '-')
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

    /**
     * @brief Print a separator line with single color
     * @param text Optional text to center in the separator
     * @param sep_char Character to use for the separator
     * @param color HEX color string (e.g., "#FF5733" or "FF5733")
     */
    inline void separator(const std::string &text, char sep_char, const std::string &color) {
        int width = detail::get_terminal_width();
        std::string color_code = detail::get_single_color(color);
        std::string reset = color_code.empty() ? "" : detail::reset_color();

        if (text.empty()) {
            std::cout << color_code << std::string(width, sep_char) << reset << "\n";
        } else {
            int text_len = text.length() + 4;
            if (text_len >= width) {
                std::cout << color_code << sep_char << "[ " << text << " ]" << sep_char << reset << "\n";
            } else {
                int total_sep = width - text_len;
                int left_sep = total_sep / 2;
                int right_sep = total_sep - left_sep;

                std::cout << color_code << std::string(left_sep, sep_char) << "[ " << text << " ]"
                          << std::string(right_sep, sep_char) << reset << "\n";
            }
        }
    }

    /**
     * @brief Print a separator line with gradient colors
     * @param text Optional text to center in the separator
     * @param sep_char Character to use for the separator
     * @param gradient Vector of HEX color strings for gradient
     */
    inline void separator(const std::string &text, char sep_char, const std::vector<std::string> &gradient) {
        int width = detail::get_terminal_width();

        if (gradient.empty()) {
            // No gradient, fall back to no color
            separator(text, sep_char);
            return;
        }

        if (text.empty()) {
            // Gradient across full width
            for (int i = 0; i < width; ++i) {
                float pos = static_cast<float>(i) / static_cast<float>(width - 1);
                std::string color = detail::get_gradient_color(gradient, pos);
                std::cout << color << sep_char << detail::reset_color();
            }
            std::cout << "\n";
        } else {
            int text_len = text.length() + 4;
            if (text_len >= width) {
                std::string color = detail::get_gradient_color(gradient, 0.5f);
                std::cout << color << sep_char << "[ " << text << " ]" << sep_char << detail::reset_color() << "\n";
            } else {
                int total_sep = width - text_len;
                int left_sep = total_sep / 2;
                int right_sep = total_sep - left_sep;

                // Gradient across entire width
                int pos = 0;
                for (int i = 0; i < left_sep; ++i, ++pos) {
                    float p = static_cast<float>(pos) / static_cast<float>(width - 1);
                    std::cout << detail::get_gradient_color(gradient, p) << sep_char << detail::reset_color();
                }

                // Text in middle (use middle gradient color)
                std::string mid_color = detail::get_gradient_color(gradient, 0.5f);
                std::cout << mid_color << "[ " << text << " ]" << detail::reset_color();
                pos += text_len;

                for (int i = 0; i < right_sep; ++i, ++pos) {
                    float p = static_cast<float>(pos) / static_cast<float>(width - 1);
                    std::cout << detail::get_gradient_color(gradient, p) << sep_char << detail::reset_color();
                }
                std::cout << "\n";
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
        auto chars = detail::BoxChars::get(style);

        // Calculate box width (text + 2 spaces padding + 2 borders)
        int text_width = text.length();
        int box_width = text_width + 2; // 1 space on each side

        // Top border
        std::cout << chars.top_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.top_right << "\n";

        // Middle with text
        std::cout << chars.vertical << " " << text << " " << chars.vertical << "\n";

        // Bottom border
        std::cout << chars.bottom_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.bottom_right << "\n";
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
        auto chars = detail::BoxChars::get(style);

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
        std::cout << chars.top_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.top_right << "\n";

        // Empty line
        std::cout << chars.vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << chars.vertical << "\n";

        // Middle with centered text
        std::cout << chars.vertical << " " << std::string(left_pad, ' ') << text << std::string(right_pad, ' ') << " "
                  << chars.vertical << "\n";

        // Empty line
        std::cout << chars.vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << chars.vertical << "\n";

        // Bottom border
        std::cout << chars.bottom_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.bottom_right << "\n";
    }

    // =================================================================================================
    // Color-enabled box function
    // =================================================================================================

    /**
     * @brief Draw a box around text with single color
     * @param text Text to display in the box
     * @param style Box style
     * @param color HEX color string (e.g., "#FF5733")
     */
    inline void box(const std::string &text, BoxStyle style, const std::string &color) {
        std::string color_code = detail::get_single_color(color);
        std::string reset = color_code.empty() ? "" : detail::reset_color();
        auto chars = detail::BoxChars::get(style);

        int text_width = text.length();
        int box_width = text_width + 2;

        // Top border
        std::cout << color_code << chars.top_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.top_right << reset << "\n";

        // Middle with text
        std::cout << color_code << chars.vertical << reset << " " << text << " " << color_code << chars.vertical
                  << reset << "\n";

        // Bottom border
        std::cout << color_code << chars.bottom_left;
        for (int i = 0; i < box_width; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.bottom_right << reset << "\n";
    }

    /**
     * @brief Draw a box around text with gradient colors
     * @param text Text to display in the box
     * @param style Box style
     * @param gradient Vector of HEX color strings for gradient
     */
    inline void box(const std::string &text, BoxStyle style, const std::vector<std::string> &gradient) {
        if (gradient.empty()) {
            box(text, style);
            return;
        }

        auto chars = detail::BoxChars::get(style);

        int text_width = text.length();
        int box_width = text_width + 2;
        int total_width = box_width + 2; // Including borders

        // Top border with gradient
        int pos = 0;
        std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width) << chars.top_left
                  << detail::reset_color();
        for (int i = 0; i < box_width; ++i) {
            std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width)
                      << chars.horizontal << detail::reset_color();
        }
        std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width) << chars.top_right
                  << detail::reset_color() << "\n";

        // Middle with text
        pos = 0;
        std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width) << chars.vertical
                  << detail::reset_color() << " " << text << " "
                  << detail::get_gradient_color(gradient, static_cast<float>(total_width - 1) / total_width)
                  << chars.vertical << detail::reset_color() << "\n";

        // Bottom border with gradient
        pos = 0;
        std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width) << chars.bottom_left
                  << detail::reset_color();
        for (int i = 0; i < box_width; ++i) {
            std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width)
                      << chars.horizontal << detail::reset_color();
        }
        std::cout << detail::get_gradient_color(gradient, static_cast<float>(pos++) / total_width) << chars.bottom_right
                  << detail::reset_color() << "\n";
    }

    // =================================================================================================
    // Color-enabled header, title, and banner functions
    // =================================================================================================

    /**
     * @brief Print a fancy header with single color
     */
    inline void header(const std::string &text, const std::string &color) {
        std::string color_code = detail::get_single_color(color);
        std::string reset = color_code.empty() ? "" : detail::reset_color();

        int width = detail::get_terminal_width();
        int text_len = text.length();
        int padding = 2;
        int content_width = width - 4;

        if (text_len > content_width) {
            content_width = text_len + padding;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        // Top border
        std::cout << color_code << "╔";
        for (int i = 0; i < content_width + padding; ++i) {
            std::cout << "═";
        }
        std::cout << "╗" << reset << "\n";

        // Middle with centered text
        std::cout << color_code << "║" << reset << " " << std::string(left_pad, ' ') << text
                  << std::string(right_pad, ' ') << " " << color_code << "║" << reset << "\n";

        // Bottom border
        std::cout << color_code << "╚";
        for (int i = 0; i < content_width + padding; ++i) {
            std::cout << "═";
        }
        std::cout << "╝" << reset << "\n";
    }

    /**
     * @brief Print a centered title with single color
     */
    inline void title(const std::string &text, char border_char, const std::string &color) {
        std::string color_code = detail::get_single_color(color);
        std::string reset = color_code.empty() ? "" : detail::reset_color();

        int width = detail::get_terminal_width();
        int text_len = text.length();

        // Top border
        std::cout << color_code << std::string(width, border_char) << reset << "\n";

        // Centered text
        if (text_len >= width) {
            std::cout << text << "\n";
        } else {
            int total_padding = width - text_len;
            int left_pad = total_padding / 2;
            std::cout << std::string(left_pad, ' ') << text << "\n";
        }

        // Bottom border
        std::cout << color_code << std::string(width, border_char) << reset << "\n";
    }

    /**
     * @brief Print a large decorative banner with single color
     */
    inline void banner(const std::string &text, BoxStyle style, const std::string &color) {
        std::string color_code = detail::get_single_color(color);
        std::string reset = color_code.empty() ? "" : detail::reset_color();
        auto chars = detail::BoxChars::get(style);

        int width = detail::get_terminal_width();
        int text_len = text.length();
        int content_width = width - 4;

        if (text_len > content_width) {
            content_width = text_len + 4;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        // Top border
        std::cout << color_code << chars.top_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.top_right << reset << "\n";

        // Empty line
        std::cout << color_code << chars.vertical << reset;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << color_code << chars.vertical << reset << "\n";

        // Middle with centered text
        std::cout << color_code << chars.vertical << reset << " " << std::string(left_pad, ' ') << text
                  << std::string(right_pad, ' ') << " " << color_code << chars.vertical << reset << "\n";

        // Empty line
        std::cout << color_code << chars.vertical << reset;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << " ";
        }
        std::cout << color_code << chars.vertical << reset << "\n";

        // Bottom border
        std::cout << color_code << chars.bottom_left;
        for (int i = 0; i < content_width + 2; ++i) {
            std::cout << chars.horizontal;
        }
        std::cout << chars.bottom_right << reset << "\n";
    }

    // =================================================================================================
    // Cursor control utilities
    // =================================================================================================

    namespace detail {

        inline void hide_cursor() noexcept { std::cout << "\033[?25l" << std::flush; }

        inline void show_cursor() noexcept { std::cout << "\033[?25h" << std::flush; }

        inline void move_cursor_up(int lines = 1) noexcept { std::cout << "\033[" << lines << "A" << std::flush; }

        inline void clear_line() noexcept { std::cout << "\033[2K\r" << std::flush; }

        inline std::mutex &get_wait_mutex() noexcept {
            static std::mutex mtx;
            return mtx;
        }

    } // namespace detail

    // =================================================================================================
    // Spinner styles
    // =================================================================================================

    enum class spinner_style {
        line,           // - \ | /
        pipe,           // ┤ ┘ ┴ └ ├ ┌ ┬ ┐
        simple_dots,    // . .. ...
        dots_scrolling, // . .. ... .. .
        flip,           // _ - ` ' ´
        toggle,         // = * -
        layer,          // - = ≡
        point,          // ∙∙∙ ●∙∙ ∙●∙
        dqpb,           // d q p b
        bouncing_bar,   // [====]
        bouncing_ball,  // ( ● )
        aesthetic,      // ▰▰▰▰▰▰▰
        binary,         // 010010
        grow_vertical,  // ▁ ▃ ▄ ▅ ▆ ▇
        grow_horizontal // ▏ ▎ ▍ ▌ ▋
    };

    // =================================================================================================
    // Spinner class
    // =================================================================================================

    class spinner {
      private:
        spinner_style style_;
        std::vector<std::string> frames_;
        int interval_ms_;
        size_t current_frame_ = 0;
        std::string prefix_;
        std::string message_;
        bool is_running_ = true;
        std::vector<std::string> gradient_colors_;
        bool use_gradient_ = false;

        void init_frames() {
            switch (style_) {
            case spinner_style::line:
                frames_ = {"-", "\\", "|", "/"};
                interval_ms_ = 130;
                break;
            case spinner_style::pipe:
                frames_ = {"┤", "┘", "┴", "└", "├", "┌", "┬", "┐"};
                interval_ms_ = 100;
                break;
            case spinner_style::simple_dots:
                frames_ = {".  ", ".. ", "...", "   "};
                interval_ms_ = 400;
                break;
            case spinner_style::dots_scrolling:
                frames_ = {".  ", ".. ", "...", " ..", "  .", "   "};
                interval_ms_ = 200;
                break;
            case spinner_style::flip:
                frames_ = {"_", "_", "_", "-", "`", "`", "'", "´", "-", "_", "_", "_"};
                interval_ms_ = 70;
                break;
            case spinner_style::toggle:
                frames_ = {"=", "*", "-"};
                interval_ms_ = 80;
                break;
            case spinner_style::layer:
                frames_ = {"-", "=", "≡"};
                interval_ms_ = 150;
                break;
            case spinner_style::point:
                frames_ = {"∙∙∙", "●∙∙", "∙●∙", "∙∙●", "∙∙∙"};
                interval_ms_ = 125;
                break;
            case spinner_style::dqpb:
                frames_ = {"d", "q", "p", "b"};
                interval_ms_ = 100;
                break;
            case spinner_style::bouncing_bar:
                frames_ = {"[    ]", "[=   ]", "[==  ]", "[=== ]", "[====]", "[ ===]", "[  ==]", "[   =]",
                           "[    ]", "[   =]", "[  ==]", "[ ===]", "[====]", "[=== ]", "[==  ]", "[=   ]"};
                interval_ms_ = 80;
                break;
            case spinner_style::bouncing_ball:
                frames_ = {"( ●    )", "(  ●   )", "(   ●  )", "(    ● )", "(     ●)",
                           "(    ● )", "(   ●  )", "(  ●   )", "( ●    )", "(●     )"};
                interval_ms_ = 80;
                break;
            case spinner_style::aesthetic:
                frames_ = {"▰▱▱▱▱▱▱", "▰▰▱▱▱▱▱", "▰▰▰▱▱▱▱", "▰▰▰▰▱▱▱", "▰▰▰▰▰▱▱", "▰▰▰▰▰▰▱", "▰▰▰▰▰▰▰", "▰▱▱▱▱▱▱"};
                interval_ms_ = 80;
                break;
            case spinner_style::binary:
                frames_ = {"010010", "001100", "100101", "111010", "111101",
                           "010111", "101011", "111000", "110011", "110101"};
                interval_ms_ = 80;
                break;
            case spinner_style::grow_vertical:
                frames_ = {"▁", "▃", "▄", "▅", "▆", "▇", "▆", "▅", "▄", "▃"};
                interval_ms_ = 120;
                break;
            case spinner_style::grow_horizontal:
                frames_ = {"▏", "▎", "▍", "▌", "▋", "▊", "▉", "▊", "▋", "▌", "▍", "▎"};
                interval_ms_ = 120;
                break;
            }
        }

      public:
        spinner(spinner_style style = spinner_style::line) : style_(style) { init_frames(); }

        void set_message(const std::string &message) { message_ = message; }

        void set_prefix(const std::string &prefix) { prefix_ = prefix; }

        void set_color(const std::string &hex) {
            gradient_colors_ = {hex};
            use_gradient_ = false;
        }

        void set_gradient(const std::vector<std::string> &hexes) {
            gradient_colors_ = hexes;
            use_gradient_ = true;
        }

        void tick() {
            if (!is_running_)
                return;

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            detail::clear_line();

            // Get color for current frame
            std::string color_code;
            if (!gradient_colors_.empty()) {
                if (use_gradient_) {
                    // Cycle through gradient colors
                    float pos = static_cast<float>(current_frame_) / static_cast<float>(frames_.size() - 1);
                    color_code = detail::get_gradient_color(gradient_colors_, pos);
                } else {
                    // Single color
                    color_code = detail::get_single_color(gradient_colors_[0]);
                }
            }

            std::cout << prefix_;
            if (!prefix_.empty())
                std::cout << " ";
            std::cout << color_code << frames_[current_frame_] << detail::reset_color();
            if (!message_.empty())
                std::cout << " " << message_;
            std::cout << std::flush;

            current_frame_ = (current_frame_ + 1) % frames_.size();
        }

        void stop(const std::string &final_message = "") {
            if (!is_running_)
                return;

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            is_running_ = false;
            detail::clear_line();

            if (!final_message.empty()) {
                std::cout << final_message << "\n";
            }
        }

        [[nodiscard]] int get_interval_ms() const noexcept { return interval_ms_; }

        [[nodiscard]] bool is_running() const noexcept { return is_running_; }
    };

    // =================================================================================================
    // Progress bar styles and themes
    // =================================================================================================

    enum class BarStyle {
        Classic, // [===>    ] - ASCII compatible
        Blocks,  // [███▓▒░  ] - Unicode blocks
        Smooth,  // [████▌   ] - Smooth sub-blocks ▏▎▍▌▋▊▉█
        Arrows,  // [→→→→    ] - Arrow characters
        Dots,    // [●●●●○○○○] - Filled/empty dots
        ASCII    // [###>... ] - Pure ASCII (no special chars)
    };

    struct BarTheme {
        BarStyle style;
        std::string fill;
        std::string lead;
        std::string remainder;
        std::string bar_start;
        std::string bar_end;
        std::vector<std::string> gradient_colors;

        // Pre-configured themes
        static BarTheme classic() { return {BarStyle::Classic, "=", ">", " ", "[", "]", {}}; }

        static BarTheme blocks() { return {BarStyle::Blocks, "█", "▓", "░", "[", "]", {}}; }

        static BarTheme smooth() { return {BarStyle::Smooth, "█", "", "░", "[", "]", {}}; }

        static BarTheme arrows() { return {BarStyle::Arrows, "→", "⇒", " ", "[", "]", {}}; }

        static BarTheme dots() { return {BarStyle::Dots, "●", "◉", "○", "[", "]", {}}; }

        static BarTheme ascii() { return {BarStyle::ASCII, "#", ">", ".", "[", "]", {}}; }

        static BarTheme fire() {
            return {BarStyle::Blocks, "█", "▓", "░", "[", "]", {"#FF0000", "#FF7F00", "#FFFF00"}};
        }

        static BarTheme ocean() {
            return {BarStyle::Smooth, "█", "", "░", "[", "]", {"#000080", "#0000FF", "#00FFFF"}};
        }

        static BarTheme forest() {
            return {BarStyle::Blocks, "█", "▓", "░", "[", "]", {"#006400", "#228B22", "#90EE90"}};
        }

        static BarTheme sunset() {
            return {BarStyle::Smooth, "█", "", "░", "[", "]", {"#FF4500", "#FF6347", "#FFD700"}};
        }

        static BarTheme neon() { return {BarStyle::Dots, "●", "◉", "○", "[", "]", {"#FF00FF", "#00FFFF", "#FFFF00"}}; }
    };

    // =================================================================================================
    // Progress bar class
    // =================================================================================================

    class progress_bar {
      private:
        size_t current_ = 0;
        size_t total_ = 100;
        int bar_width_ = -1; // -1 means auto (full terminal width)
        std::string fill_str_ = "=";
        std::string lead_str_ = ">";
        std::string remainder_str_ = " ";
        std::string bar_start_ = "[";
        std::string bar_end_ = "]";
        std::string prefix_;
        std::string postfix_;
        bool show_percentage_ = true;
        bool show_elapsed_ = false;
        bool show_remaining_ = false;
        bool show_bytes_ = false;
        bool show_speed_ = false;
        std::chrono::steady_clock::time_point start_time_;
        bool started_ = false;
        std::vector<std::string> gradient_colors_;
        bool use_gradient_ = false;
        BarStyle bar_style_ = BarStyle::Classic;
        size_t bytes_per_unit_ = 1; // For byte formatting

        std::string format_time(int seconds) const {
            int mins = seconds / 60;
            int secs = seconds % 60;
            if (mins > 0) {
                return std::to_string(mins) + "m" + std::to_string(secs) + "s";
            }
            return std::to_string(secs) + "s";
        }

        std::string format_bytes(size_t bytes) const {
            const char *units[] = {"B", "KB", "MB", "GB", "TB"};
            int unit_index = 0;
            double size = static_cast<double>(bytes);

            while (size >= 1024.0 && unit_index < 4) {
                size /= 1024.0;
                unit_index++;
            }

            char buffer[32];
            if (size >= 100.0) {
                snprintf(buffer, sizeof(buffer), "%.0f %s", size, units[unit_index]);
            } else if (size >= 10.0) {
                snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unit_index]);
            } else {
                snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_index]);
            }
            return std::string(buffer);
        }

        std::string format_speed(double bytes_per_sec) const {
            const char *units[] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};
            int unit_index = 0;
            double speed = bytes_per_sec;

            while (speed >= 1024.0 && unit_index < 4) {
                speed /= 1024.0;
                unit_index++;
            }

            char buffer[32];
            if (speed >= 100.0) {
                snprintf(buffer, sizeof(buffer), "%.0f %s", speed, units[unit_index]);
            } else if (speed >= 10.0) {
                snprintf(buffer, sizeof(buffer), "%.1f %s", speed, units[unit_index]);
            } else {
                snprintf(buffer, sizeof(buffer), "%.2f %s", speed, units[unit_index]);
            }
            return std::string(buffer);
        }

      public:
        progress_bar(size_t total = 100) : total_(total) { start_time_ = std::chrono::steady_clock::now(); }

        void set_bar_width(int width) { bar_width_ = width; }

        void set_fill_char(char c) { fill_str_ = std::string(1, c); }

        void set_lead_char(char c) { lead_str_ = std::string(1, c); }

        void set_remainder_char(char c) { remainder_str_ = std::string(1, c); }

        void set_bar_style(BarStyle style) {
            bar_style_ = style;
            // Apply default characters for the style
            switch (style) {
            case BarStyle::Classic:
                fill_str_ = "=";
                lead_str_ = ">";
                remainder_str_ = " ";
                break;
            case BarStyle::Blocks:
                fill_str_ = "█";
                lead_str_ = "▓";
                remainder_str_ = "░";
                break;
            case BarStyle::Smooth:
                fill_str_ = "█";
                lead_str_ = "";
                remainder_str_ = "░";
                break;
            case BarStyle::Arrows:
                fill_str_ = "→";
                lead_str_ = "⇒";
                remainder_str_ = " ";
                break;
            case BarStyle::Dots:
                fill_str_ = "●";
                lead_str_ = "◉";
                remainder_str_ = "○";
                break;
            case BarStyle::ASCII:
                fill_str_ = "#";
                lead_str_ = ">";
                remainder_str_ = ".";
                break;
            }
        }

        void set_theme(const BarTheme &theme) {
            bar_style_ = theme.style;
            fill_str_ = theme.fill;
            lead_str_ = theme.lead;
            remainder_str_ = theme.remainder;
            bar_start_ = theme.bar_start;
            bar_end_ = theme.bar_end;
            if (!theme.gradient_colors.empty()) {
                set_gradient(theme.gradient_colors);
            }
        }

        void set_show_bytes(bool show, size_t bytes_per_unit = 1) {
            show_bytes_ = show;
            bytes_per_unit_ = bytes_per_unit;
        }

        void set_show_speed(bool show) { show_speed_ = show; }

        void set_prefix(const std::string &prefix) { prefix_ = prefix; }

        void set_postfix(const std::string &postfix) { postfix_ = postfix; }

        void set_show_percentage(bool show) { show_percentage_ = show; }

        void set_show_elapsed(bool show) { show_elapsed_ = show; }

        void set_show_remaining(bool show) { show_remaining_ = show; }

        void set_color(const std::string &hex) {
            gradient_colors_ = {hex};
            use_gradient_ = false;
        }

        void set_gradient(const std::vector<std::string> &hexes) {
            gradient_colors_ = hexes;
            use_gradient_ = true;
        }

        void tick() {
            if (current_ < total_) {
                current_++;
            }
            display();
        }

        void set_progress(size_t value) {
            if (value <= total_) {
                current_ = value;
            }
            display();
        }

        int calculate_bar_width() const {
            if (bar_width_ > 0) {
                return bar_width_; // User-specified width
            }

            // Auto-calculate based on terminal width
            int term_width = detail::get_terminal_width();
            int used_width = 0;

            // Account for prefix
            if (!prefix_.empty()) {
                used_width += prefix_.length() + 1; // +1 for space
            }

            // Account for bar brackets
            used_width += bar_start_.length() + bar_end_.length();

            // Account for percentage or bytes
            if (show_bytes_) {
                used_width += 30; // Estimate for "XXX.XX MB / XXX.XX MB"
            } else if (show_percentage_) {
                used_width += 5; // " 100%"
            }

            // Account for time/speed display
            if (show_elapsed_ || show_remaining_ || show_speed_) {
                used_width += 20; // Estimate for " [1m30s<2m15s, 1.2 MB/s]"
            }

            // Account for postfix
            if (!postfix_.empty()) {
                used_width += postfix_.length() + 1; // +1 for space
            }

            // Calculate remaining width for the bar, with minimum of 20
            int available = term_width - used_width;
            return available > 20 ? available : 20;
        }

        void display() {
            if (!started_) {
                started_ = true;
                start_time_ = std::chrono::steady_clock::now();
            }

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            detail::clear_line();

            // Prefix
            if (!prefix_.empty()) {
                std::cout << prefix_ << " ";
            }

            // Calculate actual bar width
            int actual_bar_width = calculate_bar_width();

            // Calculate progress
            float progress = total_ > 0 ? static_cast<float>(current_) / static_cast<float>(total_) : 0.0f;
            int filled = static_cast<int>(progress * actual_bar_width);

            // Bar
            std::cout << bar_start_;

            // For smooth style, use sub-block characters for fractional progress
            if (bar_style_ == BarStyle::Smooth && filled < actual_bar_width) {
                const std::string smooth_chars[] = {"", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};
                float exact_filled = progress * actual_bar_width;
                int whole_filled = static_cast<int>(exact_filled);
                float fraction = exact_filled - whole_filled;
                int smooth_index = static_cast<int>(fraction * 8);

                for (int i = 0; i < actual_bar_width; ++i) {
                    std::string color_code;
                    if (!gradient_colors_.empty()) {
                        if (use_gradient_) {
                            float pos = static_cast<float>(i) / static_cast<float>(actual_bar_width - 1);
                            color_code = detail::get_gradient_color(gradient_colors_, pos);
                        } else {
                            color_code = detail::get_single_color(gradient_colors_[0]);
                        }
                    }

                    if (i < whole_filled) {
                        std::cout << color_code << fill_str_ << detail::reset_color();
                    } else if (i == whole_filled && smooth_index > 0) {
                        std::cout << color_code << smooth_chars[smooth_index] << detail::reset_color();
                    } else {
                        std::cout << remainder_str_;
                    }
                }
            } else {
                // Standard rendering for other styles
                for (int i = 0; i < actual_bar_width; ++i) {
                    std::string color_code;
                    if (!gradient_colors_.empty()) {
                        if (use_gradient_) {
                            float pos = static_cast<float>(i) / static_cast<float>(actual_bar_width - 1);
                            color_code = detail::get_gradient_color(gradient_colors_, pos);
                        } else {
                            color_code = detail::get_single_color(gradient_colors_[0]);
                        }
                    }

                    if (i < filled - 1) {
                        std::cout << color_code << fill_str_ << detail::reset_color();
                    } else if (i == filled - 1 && filled < actual_bar_width && !lead_str_.empty()) {
                        std::cout << color_code << lead_str_ << detail::reset_color();
                    } else if (i >= filled) {
                        std::cout << remainder_str_;
                    } else {
                        std::cout << color_code << fill_str_ << detail::reset_color();
                    }
                }
            }
            std::cout << bar_end_;

            // Byte count or percentage
            if (show_bytes_) {
                size_t current_bytes = current_ * bytes_per_unit_;
                size_t total_bytes = total_ * bytes_per_unit_;
                std::cout << " " << format_bytes(current_bytes) << " / " << format_bytes(total_bytes);
            } else if (show_percentage_) {
                std::cout << " " << static_cast<int>(progress * 100) << "%";
            }

            // Time tracking and speed
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();

            if (show_elapsed_ || show_remaining_ || show_speed_) {
                std::cout << " [";

                if (show_elapsed_) {
                    std::cout << format_time(elapsed);
                }

                if (show_remaining_ && current_ > 0 && current_ < total_) {
                    int remaining = static_cast<int>((elapsed * (total_ - current_)) / current_);
                    if (show_elapsed_) {
                        std::cout << " < ";
                    }
                    std::cout << format_time(remaining);
                }

                if (show_speed_ && elapsed > 0) {
                    double bytes_per_sec = (current_ * bytes_per_unit_) / static_cast<double>(elapsed);
                    if (show_elapsed_ || show_remaining_) {
                        std::cout << ", ";
                    }
                    std::cout << format_speed(bytes_per_sec);
                }

                std::cout << "]";
            }

            // Postfix
            if (!postfix_.empty()) {
                std::cout << " " << postfix_;
            }

            std::cout << std::flush;
        }

        void finish() {
            current_ = total_;
            display();
            std::cout << "\n";
        }

        [[nodiscard]] bool is_completed() const noexcept { return current_ >= total_; }

        [[nodiscard]] size_t get_progress() const noexcept { return current_; }
    };

    // =================================================================================================
    // Steps class
    // =================================================================================================

    class steps {
      private:
        std::vector<std::string> step_names_;
        size_t current_step_ = 0;
        bool is_infinite_ = false;
        std::vector<bool> completed_steps_;
        std::string step_color_;

      public:
        // Finite steps (known total)
        steps(const std::vector<std::string> &step_names) : step_names_(step_names), is_infinite_(false) {
            completed_steps_.resize(step_names_.size(), false);
        }

        // Infinite steps (unknown total)
        steps() : is_infinite_(true) {}

        void add_step(const std::string &step_name) {
            if (is_infinite_) {
                step_names_.push_back(step_name);
                completed_steps_.push_back(false);
            }
        }

        void set_color(const std::string &hex) { step_color_ = hex; }

        void next() {
            if (current_step_ < step_names_.size()) {
                display_current();
                current_step_++;
            }
        }

        void complete() {
            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                completed_steps_[current_step_ - 1] = true;
                display_completed();
            }
        }

        void fail() {
            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                display_failed();
            }
        }

        void display_current() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ < step_names_.size()) {
                if (is_infinite_) {
                    std::cout << color_code << "Step " << (current_step_ + 1) << ": " << step_names_[current_step_]
                              << reset << "\n";
                } else {
                    std::cout << color_code << "Step " << (current_step_ + 1) << "/" << step_names_.size() << ": "
                              << step_names_[current_step_] << reset << "\n";
                }
            }
        }

        void display_completed() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                std::cout << color_code << "✓ " << step_names_[current_step_ - 1] << " - Complete" << reset << "\n";
            }
        }

        void display_failed() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                std::cout << color_code << "✗ " << step_names_[current_step_ - 1] << " - Failed" << reset << "\n";
            }
        }

        [[nodiscard]] bool is_complete() const noexcept { return current_step_ >= step_names_.size(); }

        [[nodiscard]] size_t get_current_step() const noexcept { return current_step_; }

        [[nodiscard]] size_t get_total_steps() const noexcept { return step_names_.size(); }
    };

    // =================================================================================================
    // format::String Integration - Widget Functions
    // =================================================================================================

    /**
     * @brief Create a separator as a format::String
     * @param text Optional text to center in the separator
     * @param sep_char Character to use for the separator
     * @return Formatted String object
     */
    [[nodiscard]] inline format::String make_separator(const std::string &text = "", char sep_char = '-') {
        int width = detail::get_terminal_width();
        std::string result;

        if (text.empty()) {
            result = std::string(width, sep_char);
        } else {
            int text_len = text.length() + 4;
            if (text_len >= width) {
                result = std::string(1, sep_char) + "[ " + text + " ]" + std::string(1, sep_char);
            } else {
                int total_sep = width - text_len;
                int left_sep = total_sep / 2;
                int right_sep = total_sep - left_sep;
                result = std::string(left_sep, sep_char) + "[ " + text + " ]" + std::string(right_sep, sep_char);
            }
        }

        return format::String(result);
    }

    /**
     * @brief Create a box as a format::String
     * @param text Text to display in the box
     * @param style Box style
     * @return Formatted String object
     */
    [[nodiscard]] inline format::String make_box(const std::string &text, BoxStyle style = BoxStyle::Single) {
        auto chars = detail::BoxChars::get(style);

        int text_width = text.length();
        int box_width = text_width + 2;

        std::string result;
        result += chars.top_left;
        for (int i = 0; i < box_width; ++i) {
            result += chars.horizontal;
        }
        result += chars.top_right;
        result += "\n";
        result += std::string(chars.vertical) + " " + text + " " + std::string(chars.vertical);
        result += "\n";
        result += chars.bottom_left;
        for (int i = 0; i < box_width; ++i) {
            result += chars.horizontal;
        }
        result += chars.bottom_right;

        return format::String(result);
    }

    /**
     * @brief Create a header as a format::String
     * @param text Header text
     * @return Formatted String object
     */
    [[nodiscard]] inline format::String make_header(const std::string &text) {
        int width = detail::get_terminal_width();
        int text_len = text.length();
        int padding = 2;
        int content_width = width - 4;

        if (text_len > content_width) {
            content_width = text_len + padding;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        std::string result;
        result += "╔";
        for (int i = 0; i < content_width + padding; ++i) {
            result += "═";
        }
        result += "╗\n";
        result += "║ " + std::string(left_pad, ' ') + text + std::string(right_pad, ' ') + " ║\n";
        result += "╚";
        for (int i = 0; i < content_width + padding; ++i) {
            result += "═";
        }
        result += "╝";

        return format::String(result);
    }

    /**
     * @brief Create a title as a format::String
     * @param text Title text
     * @param border_char Border character
     * @return Formatted String object
     */
    [[nodiscard]] inline format::String make_title(const std::string &text, char border_char = '=') {
        int width = detail::get_terminal_width();
        int text_len = text.length();

        std::string result;
        result += std::string(width, border_char) + "\n";

        if (text_len >= width) {
            result += text + "\n";
        } else {
            int total_padding = width - text_len;
            int left_pad = total_padding / 2;
            result += std::string(left_pad, ' ') + text + "\n";
        }

        result += std::string(width, border_char);

        return format::String(result);
    }

    /**
     * @brief Create a banner as a format::String
     * @param text Banner text
     * @param style Box style
     * @return Formatted String object
     */
    [[nodiscard]] inline format::String make_banner(const std::string &text, BoxStyle style = BoxStyle::Heavy) {
        auto chars = detail::BoxChars::get(style);

        int width = detail::get_terminal_width();
        int text_len = text.length();
        int content_width = width - 4;

        if (text_len > content_width) {
            content_width = text_len + 4;
        }

        int total_padding = content_width - text_len;
        int left_pad = total_padding / 2;
        int right_pad = total_padding - left_pad;

        std::string result;
        result += chars.top_left;
        for (int i = 0; i < content_width + 2; ++i) {
            result += chars.horizontal;
        }
        result += chars.top_right;
        result += "\n";

        result += chars.vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            result += " ";
        }
        result += chars.vertical;
        result += "\n";

        result += std::string(chars.vertical) + " " + std::string(left_pad, ' ') + text + std::string(right_pad, ' ') +
                  " " + std::string(chars.vertical);
        result += "\n";

        result += chars.vertical;
        for (int i = 0; i < content_width + 2; ++i) {
            result += " ";
        }
        result += chars.vertical;
        result += "\n";

        result += chars.bottom_left;
        for (int i = 0; i < content_width + 2; ++i) {
            result += chars.horizontal;
        }
        result += chars.bottom_right;

        return format::String(result);
    }

} // namespace echo
