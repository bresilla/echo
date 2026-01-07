#pragma once

/**
 * @file formatters/formatter.hpp
 * @brief Base formatter interface for log messages
 */

#include <echo/core/level.hpp>

#include <memory>
#include <string>

namespace echo {

    /**
     * @brief Log record containing all information about a log event
     */
    struct LogRecord {
        Level level;                 ///< Log level
        std::string message;         ///< Formatted message
        std::string timestamp;       ///< Timestamp string
        std::string file;            ///< Source file (optional)
        int line = 0;                ///< Source line (optional)
        std::string function;        ///< Function name (optional)
        unsigned long thread_id = 0; ///< Thread ID (optional)
        std::string color_code;      ///< ANSI color code (if any)
        bool has_color = false;      ///< Whether message has color
    };

    /**
     * @brief Abstract base class for log formatters
     *
     * Formatters convert LogRecord objects into formatted strings.
     * Each sink can have its own formatter.
     */
    class Formatter {
      public:
        virtual ~Formatter() = default;

        /**
         * @brief Format a log record into a string
         * @param record Log record to format
         * @return Formatted string
         */
        virtual std::string format(const LogRecord &record) = 0;

        /**
         * @brief Clone this formatter
         * @return Unique pointer to a copy of this formatter
         */
        virtual std::unique_ptr<Formatter> clone() const = 0;
    };

    /// Shared pointer to a formatter
    using FormatterPtr = std::shared_ptr<Formatter>;

    /**
     * @brief Default formatter - simple format with level and message
     *
     * Format: [{timestamp}][{level}] {message}
     */
    class DefaultFormatter : public Formatter {
      private:
        bool include_timestamp_;
        bool include_level_;

      public:
        /**
         * @brief Construct default formatter
         * @param include_timestamp Include timestamp in output
         * @param include_level Include log level in output
         */
        explicit DefaultFormatter(bool include_timestamp = true, bool include_level = true)
            : include_timestamp_(include_timestamp), include_level_(include_level) {}

        std::string format(const LogRecord &record) override {
            std::string result;

            if (include_timestamp_ && !record.timestamp.empty()) {
                result += "[" + record.timestamp + "]";
            }

            if (include_level_) {
                result += "[" + std::string(detail::level_name(record.level)) + "]";
            }

            if (!result.empty()) {
                result += " ";
            }

            if (record.has_color && !record.color_code.empty()) {
                result += record.color_code + record.message + detail::RESET;
            } else {
                result += record.message;
            }

            return result;
        }

        std::unique_ptr<Formatter> clone() const override {
            return std::make_unique<DefaultFormatter>(include_timestamp_, include_level_);
        }
    };

} // namespace echo
