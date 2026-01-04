#pragma once

/**
 * @file color.hpp
 * @brief RGB/HEX color utilities for Echo visual elements
 *
 * Provides comprehensive RGB color manipulation, blending, and analysis.
 * All operations work purely with RGB values (no HSL/HSV conversions).
 */

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>
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

    // =================================================================================================
    // Public Color API (echo::color namespace)
    // =================================================================================================

    namespace color {

        // Re-export RGB struct for public use
        using RGB = detail::RGB;

        // =================================================================================================
        // Color Conversions
        // =================================================================================================

        /**
         * @brief Convert HEX color string to RGB
         * @param hex HEX color string (e.g., "#FF5733" or "FF5733")
         * @return RGB structure with r, g, b values (0-255)
         */
        inline RGB from_hex(const std::string &hex) { return detail::hex_to_rgb(hex); }

        /**
         * @brief Convert RGB to HEX color string
         * @param rgb RGB color
         * @return HEX string with # prefix (e.g., "#FF5733")
         */
        inline std::string to_hex(const RGB &rgb) {
            char buffer[8];
            snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", rgb.r, rgb.g, rgb.b);
            return std::string(buffer);
        }

        /**
         * @brief Convert RGB to HEX color string
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return HEX string with # prefix
         */
        inline std::string to_hex(int r, int g, int b) { return to_hex(RGB(r, g, b)); }

        /**
         * @brief Convert RGB to ANSI color code
         * @param rgb RGB color
         * @return ANSI escape sequence for foreground color
         */
        inline std::string to_ansi(const RGB &rgb) { return detail::rgb_to_ansi(rgb); }

        /**
         * @brief Convert RGB to ANSI color code
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return ANSI escape sequence for foreground color
         */
        inline std::string to_ansi(int r, int g, int b) { return detail::rgb_to_ansi(r, g, b); }

        // =================================================================================================
        // Utility Functions
        // =================================================================================================

        /**
         * @brief Clamp RGB values to valid range [0, 255]
         * @param rgb RGB color to clamp
         * @return Clamped RGB color
         */
        inline RGB clamp(const RGB &rgb) {
            return RGB(std::max(0, std::min(255, rgb.r)), std::max(0, std::min(255, rgb.g)),
                       std::max(0, std::min(255, rgb.b)));
        }

        /**
         * @brief Generate random RGB color
         * @return Random RGB color
         */
        inline RGB random() {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(0, 255);
            return RGB(dis(gen), dis(gen), dis(gen));
        }

        // =================================================================================================
        // Color Mixing/Blending
        // =================================================================================================

        /**
         * @brief Mix two colors with equal weights (simple average)
         * @param c1 First color
         * @param c2 Second color
         * @return Mixed color
         */
        inline RGB mix(const RGB &c1, const RGB &c2) { return detail::interpolate(c1, c2, 0.5f); }

        /**
         * @brief Mix two colors with custom weight
         * @param c1 First color
         * @param c2 Second color
         * @param weight Weight of second color (0.0 = all c1, 1.0 = all c2)
         * @return Mixed color
         */
        inline RGB mix(const RGB &c1, const RGB &c2, float weight) { return detail::interpolate(c1, c2, weight); }

        /**
         * @brief Additive blend - add RGB values (clamped to 255)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         */
        inline RGB add(const RGB &c1, const RGB &c2) { return clamp(RGB(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b)); }

        /**
         * @brief Multiply blend - multiply RGB values (normalized)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         */
        inline RGB multiply(const RGB &c1, const RGB &c2) {
            return RGB((c1.r * c2.r) / 255, (c1.g * c2.g) / 255, (c1.b * c2.b) / 255);
        }

        /**
         * @brief Screen blend - inverse multiply (lighter result)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         */
        inline RGB screen(const RGB &c1, const RGB &c2) {
            return RGB(255 - ((255 - c1.r) * (255 - c2.r)) / 255, 255 - ((255 - c1.g) * (255 - c2.g)) / 255,
                       255 - ((255 - c1.b) * (255 - c2.b)) / 255);
        }

        /**
         * @brief Overlay blend - combination of multiply and screen
         * @param c1 Base color
         * @param c2 Blend color
         * @return Blended color
         */
        inline RGB overlay(const RGB &c1, const RGB &c2) {
            auto overlay_channel = [](int base, int blend) {
                if (base < 128) {
                    return (2 * base * blend) / 255;
                } else {
                    return 255 - (2 * (255 - base) * (255 - blend)) / 255;
                }
            };
            return RGB(overlay_channel(c1.r, c2.r), overlay_channel(c1.g, c2.g), overlay_channel(c1.b, c2.b));
        }

        // =================================================================================================
        // Color Adjustments
        // =================================================================================================

        /**
         * @brief Lighten color by percentage
         * @param rgb Color to lighten
         * @param amount Amount to lighten (0.0 to 1.0, where 1.0 = white)
         * @return Lightened color
         */
        inline RGB lighten(const RGB &rgb, float amount) {
            amount = std::max(0.0f, std::min(1.0f, amount));
            return mix(rgb, RGB(255, 255, 255), amount);
        }

        /**
         * @brief Darken color by percentage
         * @param rgb Color to darken
         * @param amount Amount to darken (0.0 to 1.0, where 1.0 = black)
         * @return Darkened color
         */
        inline RGB darken(const RGB &rgb, float amount) {
            amount = std::max(0.0f, std::min(1.0f, amount));
            return mix(rgb, RGB(0, 0, 0), amount);
        }

        /**
         * @brief Brighten color by adding fixed amount to RGB
         * @param rgb Color to brighten
         * @param amount Amount to add (0-255)
         * @return Brightened color
         */
        inline RGB brighten(const RGB &rgb, int amount) {
            return clamp(RGB(rgb.r + amount, rgb.g + amount, rgb.b + amount));
        }

        /**
         * @brief Dim color by subtracting fixed amount from RGB
         * @param rgb Color to dim
         * @param amount Amount to subtract (0-255)
         * @return Dimmed color
         */
        inline RGB dim(const RGB &rgb, int amount) {
            return clamp(RGB(rgb.r - amount, rgb.g - amount, rgb.b - amount));
        }

        /**
         * @brief Saturate color - increase intensity (move away from gray)
         * @param rgb Color to saturate
         * @param amount Amount to saturate (0.0 to 1.0)
         * @return Saturated color
         */
        inline RGB saturate(const RGB &rgb, float amount) {
            amount = std::max(0.0f, std::min(1.0f, amount));
            int gray = (rgb.r + rgb.g + rgb.b) / 3;
            RGB gray_color(gray, gray, gray);
            return mix(gray_color, rgb, 1.0f + amount);
        }

        /**
         * @brief Desaturate color - decrease intensity (move toward gray)
         * @param rgb Color to desaturate
         * @param amount Amount to desaturate (0.0 to 1.0, where 1.0 = full gray)
         * @return Desaturated color
         */
        inline RGB desaturate(const RGB &rgb, float amount) {
            amount = std::max(0.0f, std::min(1.0f, amount));
            int gray = (rgb.r + rgb.g + rgb.b) / 3;
            RGB gray_color(gray, gray, gray);
            return mix(rgb, gray_color, amount);
        }

        /**
         * @brief Invert color
         * @param rgb Color to invert
         * @return Inverted color
         */
        inline RGB invert(const RGB &rgb) { return RGB(255 - rgb.r, 255 - rgb.g, 255 - rgb.b); }

        /**
         * @brief Convert to grayscale using luminance formula
         * @param rgb Color to convert
         * @return Grayscale color
         */
        inline RGB grayscale(const RGB &rgb) {
            // Use standard luminance formula: 0.299*R + 0.587*G + 0.114*B
            int gray = static_cast<int>(0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b);
            return RGB(gray, gray, gray);
        }

        // =================================================================================================
        // Color Analysis
        // =================================================================================================

        /**
         * @brief Calculate perceived luminance (brightness) of color
         * @param rgb Color to analyze
         * @return Luminance value (0-255)
         */
        inline int luminance(const RGB &rgb) {
            return static_cast<int>(0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b);
        }

        /**
         * @brief Check if color is dark
         * @param rgb Color to check
         * @param threshold Luminance threshold (default 128)
         * @return True if color is dark
         */
        inline bool is_dark(const RGB &rgb, int threshold = 128) { return luminance(rgb) < threshold; }

        /**
         * @brief Check if color is light
         * @param rgb Color to check
         * @param threshold Luminance threshold (default 128)
         * @return True if color is light
         */
        inline bool is_light(const RGB &rgb, int threshold = 128) { return luminance(rgb) >= threshold; }

        /**
         * @brief Calculate Euclidean distance between two colors
         * @param c1 First color
         * @param c2 Second color
         * @return Distance value (0-441, where 441 = max distance)
         */
        inline float distance(const RGB &c1, const RGB &c2) {
            int dr = c1.r - c2.r;
            int dg = c1.g - c2.g;
            int db = c1.b - c2.b;
            return std::sqrt(static_cast<float>(dr * dr + dg * dg + db * db));
        }

        /**
         * @brief Calculate contrast ratio between two colors
         * @param c1 First color
         * @param c2 Second color
         * @return Contrast ratio (1.0 to 21.0)
         */
        inline float contrast_ratio(const RGB &c1, const RGB &c2) {
            float l1 = luminance(c1) / 255.0f;
            float l2 = luminance(c2) / 255.0f;
            float lighter = std::max(l1, l2);
            float darker = std::min(l1, l2);
            return (lighter + 0.05f) / (darker + 0.05f);
        }

        // =================================================================================================
        // Color Schemes
        // =================================================================================================

        /**
         * @brief Get complementary color (opposite/inverted)
         * @param rgb Input color
         * @return Complementary color
         */
        inline RGB complementary(const RGB &rgb) { return invert(rgb); }

        /**
         * @brief Generate analogous colors (nearby colors)
         * @param rgb Base color
         * @param count Number of analogous colors to generate
         * @param angle Rotation angle (0.0 to 1.0, default 0.1 = slight shift)
         * @return Vector of analogous colors
         */
        inline std::vector<RGB> analogous(const RGB &rgb, int count = 2, float angle = 0.1f) {
            std::vector<RGB> colors;
            for (int i = 0; i < count; ++i) {
                float shift = angle * (i + 1);
                // Simple RGB rotation approximation
                int r = static_cast<int>(rgb.r + (rgb.g - rgb.b) * shift);
                int g = static_cast<int>(rgb.g + (rgb.b - rgb.r) * shift);
                int b = static_cast<int>(rgb.b + (rgb.r - rgb.g) * shift);
                colors.push_back(clamp(RGB(r, g, b)));
            }
            return colors;
        }

        /**
         * @brief Generate triadic color scheme (three evenly spaced colors)
         * @param rgb Base color
         * @return Vector of three colors (including base)
         */
        inline std::vector<RGB> triadic(const RGB &rgb) {
            std::vector<RGB> colors;
            colors.push_back(rgb);
            // Rotate RGB channels
            colors.push_back(RGB(rgb.g, rgb.b, rgb.r));
            colors.push_back(RGB(rgb.b, rgb.r, rgb.g));
            return colors;
        }

        /**
         * @brief Generate tints (lighter versions by mixing with white)
         * @param rgb Base color
         * @param count Number of tints to generate
         * @return Vector of tints from original to white
         */
        inline std::vector<RGB> tints(const RGB &rgb, int count = 5) {
            std::vector<RGB> colors;
            for (int i = 0; i < count; ++i) {
                float amount = static_cast<float>(i) / static_cast<float>(count - 1);
                colors.push_back(lighten(rgb, amount));
            }
            return colors;
        }

        /**
         * @brief Generate shades (darker versions by mixing with black)
         * @param rgb Base color
         * @param count Number of shades to generate
         * @return Vector of shades from original to black
         */
        inline std::vector<RGB> shades(const RGB &rgb, int count = 5) {
            std::vector<RGB> colors;
            for (int i = 0; i < count; ++i) {
                float amount = static_cast<float>(i) / static_cast<float>(count - 1);
                colors.push_back(darken(rgb, amount));
            }
            return colors;
        }

        /**
         * @brief Generate tones (muted versions by mixing with gray)
         * @param rgb Base color
         * @param count Number of tones to generate
         * @return Vector of tones from original to gray
         */
        inline std::vector<RGB> tones(const RGB &rgb, int count = 5) {
            std::vector<RGB> colors;
            for (int i = 0; i < count; ++i) {
                float amount = static_cast<float>(i) / static_cast<float>(count - 1);
                colors.push_back(desaturate(rgb, amount));
            }
            return colors;
        }

        /**
         * @brief Generate color from temperature (warm/cool)
         * @param temp Temperature value (-1.0 = cool/blue, 0.0 = neutral, 1.0 = warm/red)
         * @return RGB color representing the temperature
         */
        inline RGB from_temperature(float temp) {
            temp = std::max(-1.0f, std::min(1.0f, temp));
            if (temp < 0) {
                // Cool colors (blue)
                float t = -temp;
                return RGB(static_cast<int>(255 * (1 - t)), static_cast<int>(255 * (1 - t * 0.5f)), 255);
            } else {
                // Warm colors (red/orange)
                return RGB(255, static_cast<int>(255 * (1 - temp * 0.5f)), static_cast<int>(255 * (1 - temp)));
            }
        }

    } // namespace color

} // namespace echo
