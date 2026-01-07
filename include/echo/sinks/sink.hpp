#pragma once

/**
 * @file sinks/sink.hpp
 * @brief Abstract base class for logging sinks
 */

#include <echo/core/level.hpp>

#include <memory>
#include <string>

namespace echo {

    /**
     * @brief Abstract base class for all logging sinks
     *
     * A sink is an output destination for log messages. Echo supports multiple
     * sinks simultaneously, each with its own formatting and filtering.
     *
     * Built-in sinks:
     * - ConsoleSink: stdout/stderr (always available)
     * - FileSink: File output with rotation (requires -DECHO_ENABLE_FILE_SINK)
     * - SyslogSink: Unix syslog (requires -DECHO_ENABLE_SYSLOG_SINK)
     * - NetworkSink: TCP/UDP logging (requires -DECHO_ENABLE_NETWORK_SINK)
     *
     * Custom sinks can be created by inheriting from this class.
     */
    class Sink {
      public:
        virtual ~Sink() = default;

        /**
         * @brief Write a log message to this sink
         * @param level Log level
         * @param message Formatted message (may include ANSI codes)
         *
         * This method is called by the logging system for each message.
         * Implementations should:
         * - Check if the message should be logged (level filtering)
         * - Format the message appropriately for the sink
         * - Write to the output destination
         * - Handle errors gracefully
         */
        virtual void write(Level level, const std::string &message) = 0;

        /**
         * @brief Flush any buffered output
         *
         * Called when the user explicitly requests a flush, or during
         * shutdown. Implementations should ensure all buffered data is
         * written to the destination.
         */
        virtual void flush() = 0;

        /**
         * @brief Set minimum log level for this sink
         * @param level Minimum level to log
         *
         * Messages below this level will not be written to this sink.
         * This allows per-sink filtering (e.g., console shows Info+, file shows Debug+).
         */
        virtual void set_level(Level level) { min_level_ = level; }

        /**
         * @brief Get current minimum log level
         * @return Current minimum level
         */
        [[nodiscard]] virtual Level get_level() const noexcept { return min_level_; }

        /**
         * @brief Check if a message at the given level should be logged
         * @param level Message level
         * @return true if message should be logged
         */
        [[nodiscard]] virtual bool should_log(Level level) const noexcept {
            return static_cast<int>(level) >= static_cast<int>(min_level_);
        }

      protected:
        Level min_level_ = Level::Trace; ///< Minimum level to log (default: log everything)
    };

    /// Shared pointer to a sink (for easy management)
    using SinkPtr = std::shared_ptr<Sink>;

} // namespace echo
