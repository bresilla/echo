#pragma once

/**
 * @file formatters/custom.hpp
 * @brief Custom function-based formatter
 */

#include <echo/formatters/formatter.hpp>

#include <functional>

namespace echo {

    /**
     * @brief Custom formatter using a user-provided function
     *
     * Allows users to provide their own formatting logic via a lambda or function.
     *
     * Example:
     *   auto formatter = std::make_shared<CustomFormatter>(
     *       [](const LogRecord& rec) {
     *           return "[" + rec.timestamp + "] " + rec.message;
     *       }
     *   );
     */
    class CustomFormatter : public Formatter {
      public:
        /// Function type for custom formatting
        using FormatFunc = std::function<std::string(const LogRecord &)>;

      private:
        FormatFunc format_func_;

      public:
        /**
         * @brief Construct custom formatter
         * @param func Formatting function
         */
        explicit CustomFormatter(FormatFunc func) : format_func_(std::move(func)) {}

        std::string format(const LogRecord &record) override {
            if (format_func_) {
                return format_func_(record);
            }
            // Fallback to simple format if no function provided
            return record.message;
        }

        std::unique_ptr<Formatter> clone() const override { return std::make_unique<CustomFormatter>(format_func_); }
    };

} // namespace echo
