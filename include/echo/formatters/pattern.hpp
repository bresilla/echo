#pragma once

/**
 * @file formatters/pattern.hpp
 * @brief Pattern-based log formatter
 *
 * Supports placeholders like:
 * - {timestamp} or {time} - Timestamp
 * - {level} - Log level name
 * - {message} or {msg} - Log message
 * - {file} - Source file
 * - {line} - Source line
 * - {function} or {func} - Function name
 * - {thread} - Thread ID
 *
 * Example: "[{time}][{level}] {msg}"
 */

#include <echo/formatters/formatter.hpp>

#include <regex>
#include <sstream>
#include <string>

namespace echo {

    /**
     * @brief Pattern-based formatter
     *
     * Formats log records using a pattern string with placeholders.
     *
     * Example patterns:
     * - "[{time}][{level}] {msg}"
     * - "{level:5} | {msg}"
     * - "[{time}] {file}:{line} - {msg}"
     */
    class PatternFormatter : public Formatter {
      private:
        std::string pattern_;

        /**
         * @brief Replace a placeholder in the pattern
         * @param pattern Pattern string
         * @param placeholder Placeholder name (without braces)
         * @param value Replacement value
         * @return Pattern with placeholder replaced
         */
        static std::string replace_placeholder(const std::string &pattern, const std::string &placeholder,
                                               const std::string &value) {
            std::string result = pattern;
            std::string full_placeholder = "{" + placeholder + "}";
            size_t pos = 0;

            while ((pos = result.find(full_placeholder, pos)) != std::string::npos) {
                result.replace(pos, full_placeholder.length(), value);
                pos += value.length();
            }

            return result;
        }

      public:
        /**
         * @brief Construct pattern formatter
         * @param pattern Pattern string with placeholders
         *
         * Default pattern: "[{time}][{level}] {msg}"
         */
        explicit PatternFormatter(const std::string &pattern = "[{time}][{level}] {msg}") : pattern_(pattern) {}

        std::string format(const LogRecord &record) override {
            std::string result = pattern_;

            // Replace timestamp placeholders
            if (!record.timestamp.empty()) {
                result = replace_placeholder(result, "timestamp", record.timestamp);
                result = replace_placeholder(result, "time", record.timestamp);
            } else {
                result = replace_placeholder(result, "timestamp", "");
                result = replace_placeholder(result, "time", "");
            }

            // Replace level placeholder
            std::string level_name = detail::level_name(record.level);
            result = replace_placeholder(result, "level", level_name);

            // Replace message placeholder
            std::string message = record.message;
            if (record.has_color && !record.color_code.empty()) {
                message = record.color_code + message + detail::RESET;
            }
            result = replace_placeholder(result, "message", message);
            result = replace_placeholder(result, "msg", message);

            // Replace file placeholder
            if (!record.file.empty()) {
                result = replace_placeholder(result, "file", record.file);
            } else {
                result = replace_placeholder(result, "file", "");
            }

            // Replace line placeholder
            if (record.line > 0) {
                result = replace_placeholder(result, "line", std::to_string(record.line));
            } else {
                result = replace_placeholder(result, "line", "");
            }

            // Replace function placeholder
            if (!record.function.empty()) {
                result = replace_placeholder(result, "function", record.function);
                result = replace_placeholder(result, "func", record.function);
            } else {
                result = replace_placeholder(result, "function", "");
                result = replace_placeholder(result, "func", "");
            }

            // Replace thread placeholder
            if (record.thread_id > 0) {
                result = replace_placeholder(result, "thread", std::to_string(record.thread_id));
            } else {
                result = replace_placeholder(result, "thread", "");
            }

            return result;
        }

        std::unique_ptr<Formatter> clone() const override { return std::make_unique<PatternFormatter>(pattern_); }

        /**
         * @brief Get the current pattern
         * @return Pattern string
         */
        [[nodiscard]] const std::string &pattern() const { return pattern_; }

        /**
         * @brief Set a new pattern
         * @param pattern New pattern string
         */
        void set_pattern(const std::string &pattern) { pattern_ = pattern; }
    };

} // namespace echo
