#pragma once

/**
 * @file color.hpp
 * @brief Color utilities for Echo visual elements
 *
 * Provides comprehensive color manipulation, blending, and analysis.
 * All operations work purely with Color values (no HSL/HSV conversions).
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
        [[nodiscard]] inline int get_terminal_width() noexcept {
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
        // Color Structure
        // =================================================================================================

        struct Color {
            int r, g, b;

            Color() : r(0), g(0), b(0) {}
            Color(int red, int green, int blue) : r(red), g(green), b(blue) {}
        };

        // =================================================================================================
        // HEX to Color Conversion
        // =================================================================================================

        /**
         * @brief Convert HEX color string to Color
         * @param hex HEX color string (e.g., "#FF5733" or "FF5733")
         * @return Color structure with r, g, b values (0-255)
         *
         * Accepts both "#RRGGBB" and "RRGGBB" formats.
         * Returns {0, 0, 0} if invalid format.
         * @note Exception-safe: returns black on any parsing error
         */
        [[nodiscard]] inline Color hex_to_rgb(const std::string &hex) noexcept {
            try {
                if (hex.empty())
                    return Color(0, 0, 0);

                std::string h = hex;

                // Remove '#' if present
                if (h[0] == '#') {
                    h = h.substr(1);
                }

                // Validate length
                if (h.length() != 6) {
                    return Color(0, 0, 0);
                }

                // Validate hex characters
                for (char c : h) {
                    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                        return Color(0, 0, 0);
                    }
                }

                // Convert to Color
                Color rgb;
                rgb.r = std::stoi(h.substr(0, 2), nullptr, 16);
                rgb.g = std::stoi(h.substr(2, 2), nullptr, 16);
                rgb.b = std::stoi(h.substr(4, 2), nullptr, 16);

                return rgb;
            } catch (const std::exception &) {
                return Color(0, 0, 0);
            }
        }

        // =================================================================================================
        // Color to ANSI Color Code
        // =================================================================================================

        /**
         * @brief Convert Color to ANSI 24-bit color escape code
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return ANSI escape sequence for foreground color
         */
        [[nodiscard]] inline std::string rgb_to_ansi(int r, int g, int b) {
            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }

        /**
         * @brief Convert Color struct to ANSI color code
         */
        [[nodiscard]] inline std::string rgb_to_ansi(const Color &rgb) { return rgb_to_ansi(rgb.r, rgb.g, rgb.b); }

        /**
         * @brief Get ANSI reset code to return to default color
         */
        [[nodiscard]] inline constexpr const char *reset_color() noexcept { return "\033[0m"; }

        // =================================================================================================
        // Color Interpolation for Gradients
        // =================================================================================================

        /**
         * @brief Linearly interpolate between two Color colors
         * @param c1 First color
         * @param c2 Second color
         * @param t Interpolation factor (0.0 = c1, 1.0 = c2)
         * @return Interpolated Color color
         */
        [[nodiscard]] inline Color interpolate(const Color &c1, const Color &c2, float t) noexcept {
            // Clamp t to [0, 1]
            t = std::max(0.0f, std::min(1.0f, t));

            return Color(static_cast<int>(c1.r + (c2.r - c1.r) * t), static_cast<int>(c1.g + (c2.g - c1.g) * t),
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

        // Re-export Color struct for public use
        using Color = detail::Color;

        // =================================================================================================
        // Color Conversions
        // =================================================================================================

        /**
         * @brief Convert HEX color string to Color
         * @param hex HEX color string (e.g., "#FF5733" or "FF5733")
         * @return Color structure with r, g, b values (0-255)
         */
        [[nodiscard]] inline Color from_hex(const std::string &hex) noexcept { return detail::hex_to_rgb(hex); }

        /**
         * @brief Convert Color to HEX color string
         * @param rgb Color color
         * @return HEX string with # prefix (e.g., "#FF5733")
         */
        [[nodiscard]] inline std::string to_hex(const Color &rgb) {
            char buffer[8];
            snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", rgb.r, rgb.g, rgb.b);
            return std::string(buffer);
        }

        /**
         * @brief Convert Color to HEX color string
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return HEX string with # prefix
         */
        [[nodiscard]] inline std::string to_hex(int r, int g, int b) { return to_hex(Color(r, g, b)); }

        /**
         * @brief Convert Color to ANSI color code
         * @param rgb Color color
         * @return ANSI escape sequence for foreground color
         */
        [[nodiscard]] inline std::string to_ansi(const Color &rgb) { return detail::rgb_to_ansi(rgb); }

        /**
         * @brief Convert Color to ANSI color code
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return ANSI escape sequence for foreground color
         */
        [[nodiscard]] inline std::string to_ansi(int r, int g, int b) { return detail::rgb_to_ansi(r, g, b); }

        // =================================================================================================
        // Utility Functions
        // =================================================================================================

        /**
         * @brief Clamp Color values to valid range [0, 255]
         * @param rgb Color color to clamp
         * @return Clamped Color color
         */
        [[nodiscard]] inline Color clamp(const Color &rgb) noexcept {
            return Color(std::max(0, std::min(255, rgb.r)), std::max(0, std::min(255, rgb.g)),
                         std::max(0, std::min(255, rgb.b)));
        }

        /**
         * @brief Generate random Color color
         * @return Random Color color
         */
        [[nodiscard]] inline Color random() {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(0, 255);
            return Color(dis(gen), dis(gen), dis(gen));
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
        [[nodiscard]] inline Color mix(const Color &c1, const Color &c2) noexcept {
            return detail::interpolate(c1, c2, 0.5f);
        }

        /**
         * @brief Mix two colors with custom weight
         * @param c1 First color
         * @param c2 Second color
         * @param weight Weight of second color (0.0 = all c1, 1.0 = all c2)
         * @return Mixed color
         */
        [[nodiscard]] inline Color mix(const Color &c1, const Color &c2, float weight) noexcept {
            return detail::interpolate(c1, c2, weight);
        }

        /**
         * @brief Additive blend - add Color values (clamped to 255)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         */
        [[nodiscard]] inline Color add(const Color &c1, const Color &c2) noexcept {
            return clamp(Color(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b));
        }

        /**
         * @brief Multiply blend - multiply Color values (normalized)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         * @note Uses larger intermediate type to prevent overflow
         */
        [[nodiscard]] inline Color multiply(const Color &c1, const Color &c2) noexcept {
            return Color(static_cast<int>((static_cast<long>(c1.r) * c2.r) / 255),
                         static_cast<int>((static_cast<long>(c1.g) * c2.g) / 255),
                         static_cast<int>((static_cast<long>(c1.b) * c2.b) / 255));
        }

        /**
         * @brief Screen blend - inverse multiply (lighter result)
         * @param c1 First color
         * @param c2 Second color
         * @return Blended color
         * @note Uses larger intermediate type to prevent overflow
         */
        [[nodiscard]] inline Color screen(const Color &c1, const Color &c2) noexcept {
            return Color(255 - static_cast<int>((static_cast<long>(255 - c1.r) * (255 - c2.r)) / 255),
                         255 - static_cast<int>((static_cast<long>(255 - c1.g) * (255 - c2.g)) / 255),
                         255 - static_cast<int>((static_cast<long>(255 - c1.b) * (255 - c2.b)) / 255));
        }

        /**
         * @brief Overlay blend - combination of multiply and screen
         * @param c1 Base color
         * @param c2 Blend color
         * @return Blended color
         * @note Uses larger intermediate type to prevent overflow
         */
        [[nodiscard]] inline Color overlay(const Color &c1, const Color &c2) noexcept {
            auto overlay_channel = [](int base, int blend) noexcept -> int {
                if (base < 128) {
                    return static_cast<int>((2L * base * blend) / 255);
                } else {
                    return 255 - static_cast<int>((2L * (255 - base) * (255 - blend)) / 255);
                }
            };
            return Color(overlay_channel(c1.r, c2.r), overlay_channel(c1.g, c2.g), overlay_channel(c1.b, c2.b));
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
        [[nodiscard]] inline Color lighten(const Color &rgb, float amount) noexcept {
            amount = std::max(0.0f, std::min(1.0f, amount));
            return mix(rgb, Color(255, 255, 255), amount);
        }

        /**
         * @brief Darken color by percentage
         * @param rgb Color to darken
         * @param amount Amount to darken (0.0 to 1.0, where 1.0 = black)
         * @return Darkened color
         */
        [[nodiscard]] inline Color darken(const Color &rgb, float amount) noexcept {
            amount = std::max(0.0f, std::min(1.0f, amount));
            return mix(rgb, Color(0, 0, 0), amount);
        }

        /**
         * @brief Brighten color by adding fixed amount to Color
         * @param rgb Color to brighten
         * @param amount Amount to add (0-255)
         * @return Brightened color
         */
        [[nodiscard]] inline Color brighten(const Color &rgb, int amount) noexcept {
            return clamp(Color(rgb.r + amount, rgb.g + amount, rgb.b + amount));
        }

        /**
         * @brief Dim color by subtracting fixed amount from Color
         * @param rgb Color to dim
         * @param amount Amount to subtract (0-255)
         * @return Dimmed color
         */
        [[nodiscard]] inline Color dim(const Color &rgb, int amount) noexcept {
            return clamp(Color(rgb.r - amount, rgb.g - amount, rgb.b - amount));
        }

        /**
         * @brief Saturate color - increase intensity (move away from gray)
         * @param rgb Color to saturate
         * @param amount Amount to saturate (0.0 to 1.0)
         * @return Saturated color
         */
        [[nodiscard]] inline Color saturate(const Color &rgb, float amount) noexcept {
            amount = std::max(0.0f, std::min(1.0f, amount));
            int gray = (rgb.r + rgb.g + rgb.b) / 3;
            Color gray_color(gray, gray, gray);
            return mix(gray_color, rgb, 1.0f + amount);
        }

        /**
         * @brief Desaturate color - decrease intensity (move toward gray)
         * @param rgb Color to desaturate
         * @param amount Amount to desaturate (0.0 to 1.0, where 1.0 = full gray)
         * @return Desaturated color
         */
        [[nodiscard]] inline Color desaturate(const Color &rgb, float amount) noexcept {
            amount = std::max(0.0f, std::min(1.0f, amount));
            int gray = (rgb.r + rgb.g + rgb.b) / 3;
            Color gray_color(gray, gray, gray);
            return mix(rgb, gray_color, amount);
        }

        /**
         * @brief Invert color
         * @param rgb Color to invert
         * @return Inverted color
         */
        [[nodiscard]] inline Color invert(const Color &rgb) noexcept {
            return Color(255 - rgb.r, 255 - rgb.g, 255 - rgb.b);
        }

        /**
         * @brief Convert to grayscale using luminance formula
         * @param rgb Color to convert
         * @return Grayscale color
         */
        [[nodiscard]] inline Color grayscale(const Color &rgb) noexcept {
            // Use standard luminance formula: 0.299*R + 0.587*G + 0.114*B
            int gray = static_cast<int>(0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b);
            return Color(gray, gray, gray);
        }

        // =================================================================================================
        // Color Analysis
        // =================================================================================================

        /**
         * @brief Calculate perceived luminance (brightness) of color
         * @param rgb Color to analyze
         * @return Luminance value (0-255)
         */
        [[nodiscard]] inline int luminance(const Color &rgb) noexcept {
            return static_cast<int>(0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b);
        }

        /**
         * @brief Check if color is dark
         * @param rgb Color to check
         * @param threshold Luminance threshold (default 128)
         * @return True if color is dark
         */
        [[nodiscard]] inline bool is_dark(const Color &rgb, int threshold = 128) noexcept {
            return luminance(rgb) < threshold;
        }

        /**
         * @brief Check if color is light
         * @param rgb Color to check
         * @param threshold Luminance threshold (default 128)
         * @return True if color is light
         */
        [[nodiscard]] inline bool is_light(const Color &rgb, int threshold = 128) noexcept {
            return luminance(rgb) >= threshold;
        }

        /**
         * @brief Calculate Euclidean distance between two colors
         * @param c1 First color
         * @param c2 Second color
         * @return Distance value (0-441, where 441 = max distance)
         */
        [[nodiscard]] inline float distance(const Color &c1, const Color &c2) noexcept {
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
        [[nodiscard]] inline float contrast_ratio(const Color &c1, const Color &c2) noexcept {
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
        [[nodiscard]] inline Color complementary(const Color &rgb) noexcept { return invert(rgb); }

        /**
         * @brief Generate analogous colors (nearby colors)
         * @param rgb Base color
         * @param count Number of analogous colors to generate
         * @param angle Rotation angle (0.0 to 1.0, default 0.1 = slight shift)
         * @return Vector of analogous colors
         */
        [[nodiscard]] inline std::vector<Color> analogous(const Color &rgb, int count = 2, float angle = 0.1f) {
            std::vector<Color> colors;
            for (int i = 0; i < count; ++i) {
                float shift = angle * (i + 1);
                // Simple Color rotation approximation
                int r = static_cast<int>(rgb.r + (rgb.g - rgb.b) * shift);
                int g = static_cast<int>(rgb.g + (rgb.b - rgb.r) * shift);
                int b = static_cast<int>(rgb.b + (rgb.r - rgb.g) * shift);
                colors.push_back(clamp(Color(r, g, b)));
            }
            return colors;
        }

        /**
         * @brief Generate triadic color scheme (three evenly spaced colors)
         * @param rgb Base color
         * @return Vector of three colors (including base)
         */
        [[nodiscard]] inline std::vector<Color> triadic(const Color &rgb) {
            std::vector<Color> colors;
            colors.push_back(rgb);
            // Rotate Color channels
            colors.push_back(Color(rgb.g, rgb.b, rgb.r));
            colors.push_back(Color(rgb.b, rgb.r, rgb.g));
            return colors;
        }

        /**
         * @brief Generate tints (lighter versions by mixing with white)
         * @param rgb Base color
         * @param count Number of tints to generate
         * @return Vector of tints from original to white
         */
        [[nodiscard]] inline std::vector<Color> tints(const Color &rgb, int count = 5) {
            std::vector<Color> colors;
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
        [[nodiscard]] inline std::vector<Color> shades(const Color &rgb, int count = 5) {
            std::vector<Color> colors;
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
        [[nodiscard]] inline std::vector<Color> tones(const Color &rgb, int count = 5) {
            std::vector<Color> colors;
            for (int i = 0; i < count; ++i) {
                float amount = static_cast<float>(i) / static_cast<float>(count - 1);
                colors.push_back(desaturate(rgb, amount));
            }
            return colors;
        }

        /**
         * @brief Generate color from temperature (warm/cool)
         * @param temp Temperature value (-1.0 = cool/blue, 0.0 = neutral, 1.0 = warm/red)
         * @return Color color representing the temperature
         */
        [[nodiscard]] inline Color from_temperature(float temp) noexcept {
            temp = std::max(-1.0f, std::min(1.0f, temp));
            if (temp < 0) {
                // Cool colors (blue)
                float t = -temp;
                return Color(static_cast<int>(255 * (1 - t)), static_cast<int>(255 * (1 - t * 0.5f)), 255);
            } else {
                // Warm colors (red/orange)
                return Color(255, static_cast<int>(255 * (1 - temp * 0.5f)), static_cast<int>(255 * (1 - temp)));
            }
        }

    } // namespace color

} // namespace echo
