#pragma once

/**
 * @file sinks/console_sink.hpp
 * @brief Console output sink (stdout/stderr)
 *
 * This sink is ALWAYS available and is the default output destination.
 */

#include <echo/sinks/sink.hpp>

#include <iostream>

namespace echo {

    /**
     * @brief Console sink - writes to stdout/stderr
     *
     * This is the default sink, always available. It writes:
     * - Trace, Debug, Info, Warn → stdout
     * - Error, Critical → stderr
     *
     * ANSI color codes are preserved (not stripped).
     *
     * Example:
     *   auto console = std::make_shared<ConsoleSink>();
     *   console->set_level(Level::Info);  // Only log Info and above
     *   echo::add_sink(console);
     */
    class ConsoleSink : public Sink {
      public:
        ConsoleSink() = default;
        ~ConsoleSink() override = default;

        /**
         * @brief Write message to console
         * @param level Log level
         * @param message Formatted message (with ANSI codes)
         */
        void write(Level level, const std::string &message) override {
            if (!should_log(level)) {
                return;
            }

            // Error and Critical go to stderr, everything else to stdout
            std::ostream &out = (level >= Level::Error) ? std::cerr : std::cout;
            out << message << std::flush;
        }

        /**
         * @brief Flush console output
         */
        void flush() override {
            std::cout << std::flush;
            std::cerr << std::flush;
        }
    };

} // namespace echo
