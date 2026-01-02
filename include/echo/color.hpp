#pragma once

/**
 * @file color.hpp
 * @brief HEX color utilities for Echo visual elements
 *
 * Provides HEX color support with gradient capabilities.
 * Used internally by banner.hpp and wait.hpp.
 */

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
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
        inline int get_terminal_width() {
#ifdef _WIN32
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
        // RGB Color Structure
        // =================================================================================================

        struct RGB {
            int r, g, b;

            RGB() : r(0), g(0), b(0) {}
            RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
        };

        // =================================================================================================
        // HEX to RGB Conversion
        // =================================================================================================

        /**
         * @brief Convert HEX color string to RGB
         * @param hex HEX color string (e.g., "#FF5733" or "FF5733")
         * @return RGB structure with r, g, b values (0-255)
         *
         * Accepts both "#RRGGBB" and "RRGGBB" formats.
         * Returns {0, 0, 0} if invalid format.
         */
        inline RGB hex_to_rgb(const std::string &hex) {
            if (hex.empty())
                return RGB(0, 0, 0);

            std::string h = hex;

            // Remove '#' if present
            if (h[0] == '#') {
                h = h.substr(1);
            }

            // Validate length
            if (h.length() != 6) {
                return RGB(0, 0, 0);
            }

            // Validate hex characters
            for (char c : h) {
                if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                    return RGB(0, 0, 0);
                }
            }

            // Convert to RGB
            RGB rgb;
            rgb.r = std::stoi(h.substr(0, 2), nullptr, 16);
            rgb.g = std::stoi(h.substr(2, 2), nullptr, 16);
            rgb.b = std::stoi(h.substr(4, 2), nullptr, 16);

            return rgb;
        }

        // =================================================================================================
        // RGB to ANSI Color Code
        // =================================================================================================

        /**
         * @brief Convert RGB to ANSI 24-bit color escape code
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return ANSI escape sequence for foreground color
         */
        inline std::string rgb_to_ansi(int r, int g, int b) {
            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }

        /**
         * @brief Convert RGB struct to ANSI color code
         */
        inline std::string rgb_to_ansi(const RGB &rgb) { return rgb_to_ansi(rgb.r, rgb.g, rgb.b); }

        /**
         * @brief Get ANSI reset code to return to default color
         */
        inline std::string reset_color() { return "\033[0m"; }

        // =================================================================================================
        // Color Interpolation for Gradients
        // =================================================================================================

        /**
         * @brief Linearly interpolate between two RGB colors
         * @param c1 First color
         * @param c2 Second color
         * @param t Interpolation factor (0.0 = c1, 1.0 = c2)
         * @return Interpolated RGB color
         */
        inline RGB interpolate(const RGB &c1, const RGB &c2, float t) {
            // Clamp t to [0, 1]
            t = std::max(0.0f, std::min(1.0f, t));

            return RGB(static_cast<int>(c1.r + (c2.r - c1.r) * t), static_cast<int>(c1.g + (c2.g - c1.g) * t),
                       static_cast<int>(c1.b + (c2.b - c1.b) * t));
        }

        // =================================================================================================
        // Gradient Color Generation
        // =================================================================================================

        /**
         * @brief Get color from gradient at specific position
         * @param hex_colors Vector of HEX color strings
         * @param position Position in gradient (0.0 to 1.0)
         * @return ANSI color code for the interpolated color
         *
         * If empty vector, returns empty string (no color).
         * If single color, returns that color.
         * If multiple colors, interpolates between them based on position.
         */
        inline std::string get_gradient_color(const std::vector<std::string> &hex_colors, float position) {
            if (hex_colors.empty()) {
                return "";
            }

            if (hex_colors.size() == 1) {
                auto rgb = hex_to_rgb(hex_colors[0]);
                if (rgb.r == 0 && rgb.g == 0 && rgb.b == 0 && hex_colors[0] != "#000000" && hex_colors[0] != "000000") {
                    // Invalid color, return no color
                    return "";
                }
                return rgb_to_ansi(rgb);
            }

            // Clamp position to [0, 1]
            position = std::max(0.0f, std::min(1.0f, position));

            // Multi-color gradient
            int segments = hex_colors.size() - 1;
            float segment_pos = position * segments;
            int segment = static_cast<int>(segment_pos);

            // Handle edge case where position = 1.0
            if (segment >= segments) {
                segment = segments - 1;
            }

            float local_t = segment_pos - segment;

            auto c1 = hex_to_rgb(hex_colors[segment]);
            auto c2 = hex_to_rgb(hex_colors[segment + 1]);

            // Check for invalid colors
            if ((c1.r == 0 && c1.g == 0 && c1.b == 0 && hex_colors[segment] != "#000000" &&
                 hex_colors[segment] != "000000") ||
                (c2.r == 0 && c2.g == 0 && c2.b == 0 && hex_colors[segment + 1] != "#000000" &&
                 hex_colors[segment + 1] != "000000")) {
                return "";
            }

            auto interpolated = interpolate(c1, c2, local_t);
            return rgb_to_ansi(interpolated);
        }

        /**
         * @brief Get single color from HEX string
         * @param hex HEX color string
         * @return ANSI color code, or empty string if invalid
         */
        inline std::string get_single_color(const std::string &hex) {
            if (hex.empty()) {
                return "";
            }

            auto rgb = hex_to_rgb(hex);

            // Check if conversion failed (invalid hex)
            if (rgb.r == 0 && rgb.g == 0 && rgb.b == 0 && hex != "#000000" && hex != "000000") {
                return "";
            }

            return rgb_to_ansi(rgb);
        }

    } // namespace detail
} // namespace echo
