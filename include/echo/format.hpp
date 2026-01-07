#pragma once

/**
 * @file format.hpp
 * @brief String formatting and styling for Echo library
 *
 * Provides echo::format::String class for chainable text formatting,
 * colors, and styles. Inspired by Rust's colored library.
 *
 * Usage:
 *   #include <echo/format.hpp>
 *
 *   using echo::format::String;
 *
 *   String("Hello").red().bold();
 *   String("World").on_blue().white().underline();
 *   String("Text").fg(255, 0, 0).bg("#0000FF");
 *   String("Formatted").center(20).bold();
 *
 * Features:
 *   - Foreground/background colors (standard, bright, RGB, HEX)
 *   - Text styles (bold, italic, underline, etc.)
 *   - Text formatting (alignment, truncation, borders, etc.)
 *   - Method chaining for fluent API
 */

#include <echo/utils/color.hpp>

#include <algorithm>
#include <cctype>
#include <optional>
#include <set>
#include <sstream>
#include <string>

namespace echo {
    namespace format {

        // =================================================================================================
        // Text Style Enum
        // =================================================================================================

        enum class Style { Bold, Dim, Italic, Underline, Blink, Reverse, Hidden, Strikethrough };

        // =================================================================================================
        // String Class
        // =================================================================================================

        class String {
          private:
            std::string text_;
            std::optional<color::Color> fg_color_;
            std::optional<color::Color> bg_color_;
            std::set<Style> styles_;

            // Helper to build ANSI escape codes
            std::string build_ansi_prefix() const {
                std::string result;

                // Foreground color
                if (fg_color_) {
                    result += "\033[38;2;" + std::to_string(fg_color_->r) + ";" + std::to_string(fg_color_->g) + ";" +
                              std::to_string(fg_color_->b) + "m";
                }

                // Background color
                if (bg_color_) {
                    result += "\033[48;2;" + std::to_string(bg_color_->r) + ";" + std::to_string(bg_color_->g) + ";" +
                              std::to_string(bg_color_->b) + "m";
                }

                // Styles
                for (const auto &style : styles_) {
                    switch (style) {
                    case Style::Bold:
                        result += "\033[1m";
                        break;
                    case Style::Dim:
                        result += "\033[2m";
                        break;
                    case Style::Italic:
                        result += "\033[3m";
                        break;
                    case Style::Underline:
                        result += "\033[4m";
                        break;
                    case Style::Blink:
                        result += "\033[5m";
                        break;
                    case Style::Reverse:
                        result += "\033[7m";
                        break;
                    case Style::Hidden:
                        result += "\033[8m";
                        break;
                    case Style::Strikethrough:
                        result += "\033[9m";
                        break;
                    }
                }

                return result;
            }

            std::string build_ansi_suffix() const {
                if (fg_color_ || bg_color_ || !styles_.empty()) {
                    return "\033[0m";
                }
                return "";
            }

          public:
            // =================================================================================================
            // Constructors
            // =================================================================================================

            String() : text_("") {}
            String(const char *text) : text_(text) {}
            String(const std::string &text) : text_(text) {}
            String(const String &other)
                : text_(other.text_), fg_color_(other.fg_color_), bg_color_(other.bg_color_), styles_(other.styles_) {}

            String &operator=(const String &other) {
                if (this != &other) {
                    text_ = other.text_;
                    fg_color_ = other.fg_color_;
                    bg_color_ = other.bg_color_;
                    styles_ = other.styles_;
                }
                return *this;
            }

            // =================================================================================================
            // Foreground Colors - Standard
            // =================================================================================================

            String &black() {
                fg_color_ = color::Color(0, 0, 0);
                return *this;
            }
            String &red() {
                fg_color_ = color::Color(205, 49, 49);
                return *this;
            }
            String &green() {
                fg_color_ = color::Color(13, 188, 121);
                return *this;
            }
            String &yellow() {
                fg_color_ = color::Color(229, 229, 16);
                return *this;
            }
            String &blue() {
                fg_color_ = color::Color(36, 114, 200);
                return *this;
            }
            String &magenta() {
                fg_color_ = color::Color(188, 63, 188);
                return *this;
            }
            String &cyan() {
                fg_color_ = color::Color(17, 168, 205);
                return *this;
            }
            String &white() {
                fg_color_ = color::Color(229, 229, 229);
                return *this;
            }

            // =================================================================================================
            // Foreground Colors - Bright
            // =================================================================================================

            String &bright_black() {
                fg_color_ = color::Color(102, 102, 102);
                return *this;
            }
            String &bright_red() {
                fg_color_ = color::Color(241, 76, 76);
                return *this;
            }
            String &bright_green() {
                fg_color_ = color::Color(35, 209, 139);
                return *this;
            }
            String &bright_yellow() {
                fg_color_ = color::Color(245, 245, 67);
                return *this;
            }
            String &bright_blue() {
                fg_color_ = color::Color(59, 142, 234);
                return *this;
            }
            String &bright_magenta() {
                fg_color_ = color::Color(214, 112, 214);
                return *this;
            }
            String &bright_cyan() {
                fg_color_ = color::Color(41, 184, 219);
                return *this;
            }
            String &bright_white() {
                fg_color_ = color::Color(255, 255, 255);
                return *this;
            }

            // =================================================================================================
            // Background Colors - Standard
            // =================================================================================================

            String &on_black() {
                bg_color_ = color::Color(0, 0, 0);
                return *this;
            }
            String &on_red() {
                bg_color_ = color::Color(205, 49, 49);
                return *this;
            }
            String &on_green() {
                bg_color_ = color::Color(13, 188, 121);
                return *this;
            }
            String &on_yellow() {
                bg_color_ = color::Color(229, 229, 16);
                return *this;
            }
            String &on_blue() {
                bg_color_ = color::Color(36, 114, 200);
                return *this;
            }
            String &on_magenta() {
                bg_color_ = color::Color(188, 63, 188);
                return *this;
            }
            String &on_cyan() {
                bg_color_ = color::Color(17, 168, 205);
                return *this;
            }
            String &on_white() {
                bg_color_ = color::Color(229, 229, 229);
                return *this;
            }

            // =================================================================================================
            // Background Colors - Bright
            // =================================================================================================

            String &on_bright_black() {
                bg_color_ = color::Color(102, 102, 102);
                return *this;
            }
            String &on_bright_red() {
                bg_color_ = color::Color(241, 76, 76);
                return *this;
            }
            String &on_bright_green() {
                bg_color_ = color::Color(35, 209, 139);
                return *this;
            }
            String &on_bright_yellow() {
                bg_color_ = color::Color(245, 245, 67);
                return *this;
            }
            String &on_bright_blue() {
                bg_color_ = color::Color(59, 142, 234);
                return *this;
            }
            String &on_bright_magenta() {
                bg_color_ = color::Color(214, 112, 214);
                return *this;
            }
            String &on_bright_cyan() {
                bg_color_ = color::Color(41, 184, 219);
                return *this;
            }
            String &on_bright_white() {
                bg_color_ = color::Color(255, 255, 255);
                return *this;
            }

            // =================================================================================================
            // RGB/HEX Color Methods
            // =================================================================================================

            String &fg(int r, int g, int b) {
                fg_color_ = color::Color(r, g, b);
                return *this;
            }

            String &fg(const std::string &hex) {
                fg_color_ = color::from_hex(hex);
                return *this;
            }

            String &fg(const color::Color &rgb) {
                fg_color_ = rgb;
                return *this;
            }

            String &bg(int r, int g, int b) {
                bg_color_ = color::Color(r, g, b);
                return *this;
            }

            String &bg(const std::string &hex) {
                bg_color_ = color::from_hex(hex);
                return *this;
            }

            String &bg(const color::Color &rgb) {
                bg_color_ = rgb;
                return *this;
            }

            // =================================================================================================
            // Text Styles
            // =================================================================================================

            String &bold() {
                styles_.insert(Style::Bold);
                return *this;
            }

            String &dim() {
                styles_.insert(Style::Dim);
                return *this;
            }

            String &dimmed() { return dim(); }

            String &italic() {
                styles_.insert(Style::Italic);
                return *this;
            }

            String &underline() {
                styles_.insert(Style::Underline);
                return *this;
            }

            String &blink() {
                styles_.insert(Style::Blink);
                return *this;
            }

            String &reverse() {
                styles_.insert(Style::Reverse);
                return *this;
            }

            String &reversed() { return reverse(); }

            String &hidden() {
                styles_.insert(Style::Hidden);
                return *this;
            }

            String &strikethrough() {
                styles_.insert(Style::Strikethrough);
                return *this;
            }

            // =================================================================================================
            // Utility Methods
            // =================================================================================================

            String &clear() {
                fg_color_.reset();
                bg_color_.reset();
                styles_.clear();
                return *this;
            }

            String &normal() { return clear(); }

            // =================================================================================================
            // Conversion Methods
            // =================================================================================================

            [[nodiscard]] std::string to_string() const { return build_ansi_prefix() + text_ + build_ansi_suffix(); }

            [[nodiscard]] std::string str() const { return text_; }

            operator std::string() const { return to_string(); }

            // =================================================================================================
            // Stream Operator
            // =================================================================================================

            friend std::ostream &operator<<(std::ostream &os, const String &s) {
                os << s.to_string();
                return os;
            }

            // =================================================================================================
            // Text Alignment
            // =================================================================================================

            String &left(size_t width) {
                if (text_.length() < width) {
                    text_ += std::string(width - text_.length(), ' ');
                }
                return *this;
            }

            String &right(size_t width) {
                if (text_.length() < width) {
                    text_ = std::string(width - text_.length(), ' ') + text_;
                }
                return *this;
            }

            String &center(size_t width) {
                if (text_.length() < width) {
                    size_t total_padding = width - text_.length();
                    size_t left_pad = total_padding / 2;
                    size_t right_pad = total_padding - left_pad;
                    text_ = std::string(left_pad, ' ') + text_ + std::string(right_pad, ' ');
                }
                return *this;
            }

            String &pad_left(size_t width, char ch = ' ') {
                if (text_.length() < width) {
                    text_ = std::string(width - text_.length(), ch) + text_;
                }
                return *this;
            }

            String &pad_right(size_t width, char ch = ' ') {
                if (text_.length() < width) {
                    text_ += std::string(width - text_.length(), ch);
                }
                return *this;
            }

            // =================================================================================================
            // Text Transformation
            // =================================================================================================

            String &uppercase() {
                std::transform(text_.begin(), text_.end(), text_.begin(), ::toupper);
                return *this;
            }

            String &to_upper() { return uppercase(); }

            String &lowercase() {
                std::transform(text_.begin(), text_.end(), text_.begin(), ::tolower);
                return *this;
            }

            String &to_lower() { return lowercase(); }

            String &capitalize() {
                if (!text_.empty()) {
                    text_[0] = ::toupper(text_[0]);
                }
                return *this;
            }

            String &title_case() {
                bool capitalize_next = true;
                for (char &c : text_) {
                    if (std::isspace(c)) {
                        capitalize_next = true;
                    } else if (capitalize_next) {
                        c = ::toupper(c);
                        capitalize_next = false;
                    } else {
                        c = ::tolower(c);
                    }
                }
                return *this;
            }

            String &reverse_text() {
                std::reverse(text_.begin(), text_.end());
                return *this;
            }

            String &repeat(size_t n) {
                std::string original = text_;
                text_.clear();
                for (size_t i = 0; i < n; ++i) {
                    text_ += original;
                }
                return *this;
            }

            // =================================================================================================
            // Text Truncation/Wrapping
            // =================================================================================================

            String &truncate(size_t length) {
                if (text_.length() > length) {
                    text_ = text_.substr(0, length);
                }
                return *this;
            }

            String &truncate(size_t length, const std::string &suffix) {
                if (text_.length() > length) {
                    if (length > suffix.length()) {
                        text_ = text_.substr(0, length - suffix.length()) + suffix;
                    } else {
                        text_ = suffix.substr(0, length);
                    }
                }
                return *this;
            }

            String &ellipsis(size_t length) { return truncate(length, "..."); }

            String &wrap(size_t width) {
                if (width == 0)
                    return *this;

                std::string result;
                std::istringstream words(text_);
                std::string word;
                size_t line_length = 0;

                while (words >> word) {
                    if (line_length + word.length() + (line_length > 0 ? 1 : 0) > width) {
                        if (line_length > 0) {
                            result += "\n";
                        }
                        result += word;
                        line_length = word.length();
                    } else {
                        if (line_length > 0) {
                            result += " ";
                            line_length++;
                        }
                        result += word;
                        line_length += word.length();
                    }
                }

                text_ = result;
                return *this;
            }

            // =================================================================================================
            // Borders/Frames
            // =================================================================================================

            String &border(char ch = '-') {
                size_t len = text_.length();
                std::string top_bottom(len + 4, ch);
                text_ = top_bottom + "\n" + ch + " " + text_ + " " + ch + "\n" + top_bottom;
                return *this;
            }

            String &box() {
                size_t len = text_.length();
                std::string horizontal(len + 2, '-');
                // Use ASCII box drawing for compatibility
                std::string top = "+" + horizontal + "+";
                std::string bottom = "+" + horizontal + "+";
                text_ = top + "\n| " + text_ + " |\n" + bottom;
                return *this;
            }

            String &quote() {
                text_ = "\"" + text_ + "\"";
                return *this;
            }

            String &brackets() {
                text_ = "[" + text_ + "]";
                return *this;
            }

            String &parens() {
                text_ = "(" + text_ + ")";
                return *this;
            }

            // =================================================================================================
            // Indentation
            // =================================================================================================

            String &indent(size_t spaces) {
                std::string indent_str(spaces, ' ');
                text_ = indent_str + text_;
                // Also indent newlines
                size_t pos = 0;
                while ((pos = text_.find('\n', pos)) != std::string::npos) {
                    text_.insert(pos + 1, indent_str);
                    pos += indent_str.length() + 1;
                }
                return *this;
            }

            String &indent(size_t level, char ch) {
                std::string indent_str(level, ch);
                text_ = indent_str + text_;
                size_t pos = 0;
                while ((pos = text_.find('\n', pos)) != std::string::npos) {
                    text_.insert(pos + 1, indent_str);
                    pos += indent_str.length() + 1;
                }
                return *this;
            }

            String &dedent() {
                // Remove leading whitespace from each line
                std::istringstream iss(text_);
                std::string line;
                std::string result;
                bool first = true;

                while (std::getline(iss, line)) {
                    if (!first)
                        result += "\n";
                    first = false;

                    // Find first non-whitespace
                    size_t start = line.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        result += line.substr(start);
                    }
                }

                text_ = result;
                return *this;
            }

            // =================================================================================================
            // Prefix/Suffix
            // =================================================================================================

            String &prefix(const std::string &str) {
                text_ = str + text_;
                return *this;
            }

            String &suffix(const std::string &str) {
                text_ += str;
                return *this;
            }

            String &surround(const std::string &left, const std::string &right) {
                text_ = left + text_ + right;
                return *this;
            }

            // =================================================================================================
            // Special Formatting
            // =================================================================================================

            String &monospace() {
                text_ = "`" + text_ + "`";
                return *this;
            }

            String &link(const std::string &url) {
                // OSC 8 hyperlink format (if terminal supports it)
                text_ = "\033]8;;" + url + "\033\\" + text_ + "\033]8;;\033\\";
                return *this;
            }

            String &progress(float percent) {
                percent = std::max(0.0f, std::min(100.0f, percent));
                int filled = static_cast<int>(percent / 10);
                int empty = 10 - filled;
                text_ = "[" + std::string(filled, '=') + std::string(empty, ' ') + "] " +
                        std::to_string(static_cast<int>(percent)) + "%";
                return *this;
            }

            String &badge(const std::string &label) {
                text_ = "[" + label + ": " + text_ + "]";
                return *this;
            }

            // =================================================================================================
            // Numeric Formatting
            // =================================================================================================

            String &format_number(int decimals) {
                try {
                    double num = std::stod(text_);
                    std::ostringstream oss;
                    oss.precision(decimals);
                    oss << std::fixed << num;
                    text_ = oss.str();
                } catch (...) {
                    // If not a number, leave unchanged
                }
                return *this;
            }

            String &format_bytes() {
                try {
                    size_t bytes = std::stoull(text_);
                    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
                    int unit_index = 0;
                    double size = static_cast<double>(bytes);

                    while (size >= 1024.0 && unit_index < 4) {
                        size /= 1024.0;
                        unit_index++;
                    }

                    std::ostringstream oss;
                    if (size >= 100.0) {
                        oss.precision(0);
                    } else if (size >= 10.0) {
                        oss.precision(1);
                    } else {
                        oss.precision(2);
                    }
                    oss << std::fixed << size << " " << units[unit_index];
                    text_ = oss.str();
                } catch (...) {
                    // If not a number, leave unchanged
                }
                return *this;
            }

            String &format_duration() {
                try {
                    int seconds = std::stoi(text_);
                    int mins = seconds / 60;
                    int secs = seconds % 60;
                    int hours = mins / 60;
                    mins = mins % 60;

                    std::ostringstream oss;
                    if (hours > 0) {
                        oss << hours << "h" << mins << "m" << secs << "s";
                    } else if (mins > 0) {
                        oss << mins << "m" << secs << "s";
                    } else {
                        oss << secs << "s";
                    }
                    text_ = oss.str();
                } catch (...) {
                    // If not a number, leave unchanged
                }
                return *this;
            }

            // =================================================================================================
            // String Manipulation
            // =================================================================================================

            String &trim() {
                // Trim left
                size_t start = text_.find_first_not_of(" \t\n\r");
                if (start == std::string::npos) {
                    text_.clear();
                    return *this;
                }

                // Trim right
                size_t end = text_.find_last_not_of(" \t\n\r");
                text_ = text_.substr(start, end - start + 1);
                return *this;
            }

            String &trim_left() {
                size_t start = text_.find_first_not_of(" \t\n\r");
                if (start != std::string::npos) {
                    text_ = text_.substr(start);
                } else {
                    text_.clear();
                }
                return *this;
            }

            String &trim_right() {
                size_t end = text_.find_last_not_of(" \t\n\r");
                if (end != std::string::npos) {
                    text_ = text_.substr(0, end + 1);
                } else {
                    text_.clear();
                }
                return *this;
            }

            String &replace(const std::string &old_str, const std::string &new_str) {
                size_t pos = 0;
                while ((pos = text_.find(old_str, pos)) != std::string::npos) {
                    text_.replace(pos, old_str.length(), new_str);
                    pos += new_str.length();
                }
                return *this;
            }

            String &remove(const std::string &str) { return replace(str, ""); }

            String &slice(size_t start, size_t end) {
                if (start < text_.length()) {
                    if (end > text_.length()) {
                        end = text_.length();
                    }
                    text_ = text_.substr(start, end - start);
                } else {
                    text_.clear();
                }
                return *this;
            }

            // =================================================================================================
            // Inspection Methods
            // =================================================================================================

            [[nodiscard]] size_t length() const noexcept { return text_.length(); }

            [[nodiscard]] size_t len() const noexcept { return text_.length(); }

            [[nodiscard]] size_t width() const noexcept {
                // Calculate display width (excluding ANSI codes)
                // For now, just return text length
                return text_.length();
            }

            [[nodiscard]] bool is_empty() const noexcept { return text_.empty(); }

            [[nodiscard]] bool contains(const std::string &str) const noexcept {
                return text_.find(str) != std::string::npos;
            }
        };

    } // namespace format
} // namespace echo
