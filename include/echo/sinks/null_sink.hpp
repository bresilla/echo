#pragma once

/**
 * @file sinks/null_sink.hpp
 * @brief Null sink that discards all log messages
 *
 * This sink is useful for:
 * - Performance testing/benchmarking (measure logging overhead)
 * - Temporarily disabling output without removing log calls
 * - Testing logging code without producing output
 *
 * Requires: -DECHO_ENABLE_NULL_SINK
 */

#include <echo/sinks/sink.hpp>

namespace echo {

    /**
     * @brief Null sink - discards all log messages
     *
     * This sink does nothing with log messages. It's useful for:
     * - Benchmarking: Measure the overhead of logging without I/O
     * - Testing: Verify logging code without producing output
     * - Disabling: Temporarily disable output without code changes
     *
     * Example:
     *   // Benchmark logging performance
     *   echo::clear_sinks();
     *   echo::add_sink(std::make_shared<echo::NullSink>());
     *
     *   auto start = std::chrono::high_resolution_clock::now();
     *   for (int i = 0; i < 1000000; ++i) {
     *       echo::info("Test message ", i);
     *   }
     *   auto end = std::chrono::high_resolution_clock::now();
     *   // Measure time without I/O overhead
     */
    class NullSink : public Sink {
      public:
        NullSink() = default;
        ~NullSink() override = default;

        /**
         * @brief Write message (does nothing)
         * @param level Log level (ignored)
         * @param message Formatted message (ignored)
         */
        void write(Level level, const std::string &message) override {
            // Check level filtering (for consistency with other sinks)
            if (!should_log(level)) {
                return;
            }
            // Do nothing - discard the message
        }

        /**
         * @brief Flush (does nothing)
         */
        void flush() override {
            // Do nothing - no buffering
        }
    };

} // namespace echo
