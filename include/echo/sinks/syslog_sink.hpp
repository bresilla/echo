#pragma once

/**
 * @file sinks/syslog_sink.hpp
 * @brief Syslog output sink (Unix/Linux only)
 *
 * Only available when compiled with -DECHO_ENABLE_SYSLOG_SINK
 * Only works on Unix/Linux systems
 */

#include <echo/sinks/sink.hpp>

#ifdef __unix__
#include <string>
#include <syslog.h>
#endif

namespace echo {

#ifdef __unix__

    /**
     * @brief Syslog sink - writes to system log
     *
     * Maps Echo log levels to syslog priorities:
     * - Trace   -> LOG_DEBUG
     * - Debug   -> LOG_DEBUG
     * - Info    -> LOG_INFO
     * - Warn    -> LOG_WARNING
     * - Error   -> LOG_ERR
     * - Critical -> LOG_CRIT
     *
     * Example:
     *   auto syslog = std::make_shared<SyslogSink>("myapp", LOG_USER);
     *   echo::add_sink(syslog);
     */
    class SyslogSink : public Sink {
      private:
        std::string ident_;
        int facility_;
        bool opened_ = false;

        /**
         * @brief Map Echo level to syslog priority
         * @param level Echo log level
         * @return Syslog priority
         */
        int level_to_priority(Level level) const noexcept {
            switch (level) {
            case Level::Trace:
                return LOG_DEBUG;
            case Level::Debug:
                return LOG_DEBUG;
            case Level::Info:
                return LOG_INFO;
            case Level::Warn:
                return LOG_WARNING;
            case Level::Error:
                return LOG_ERR;
            case Level::Critical:
                return LOG_CRIT;
            default:
                return LOG_INFO;
            }
        }

        /**
         * @brief Strip ANSI escape codes from string
         * @param str String with ANSI codes
         * @return String without ANSI codes
         */
        std::string strip_ansi(const std::string &str) const {
            std::string result;
            result.reserve(str.size());

            bool in_escape = false;
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
                    in_escape = true;
                    ++i; // Skip '['
                    continue;
                }
                if (in_escape) {
                    if (str[i] == 'm') {
                        in_escape = false;
                    }
                    continue;
                }
                result += str[i];
            }
            return result;
        }

      public:
        /**
         * @brief Construct a syslog sink
         * @param ident Program identifier (appears in syslog)
         * @param facility Syslog facility (LOG_USER, LOG_LOCAL0, etc.)
         */
        explicit SyslogSink(const std::string &ident = "echo", int facility = LOG_USER)
            : ident_(ident), facility_(facility) {
            openlog(ident_.c_str(), LOG_PID | LOG_CONS, facility_);
            opened_ = true;
        }

        ~SyslogSink() override {
            if (opened_) {
                closelog();
            }
        }

        // Prevent copying (syslog is a global resource)
        SyslogSink(const SyslogSink &) = delete;
        SyslogSink &operator=(const SyslogSink &) = delete;

        /**
         * @brief Write message to syslog
         * @param level Log level
         * @param message Formatted message (may contain ANSI codes)
         */
        void write(Level level, const std::string &message) override {
            if (!should_log(level) || !opened_) {
                return;
            }

            // Strip ANSI codes and write to syslog
            std::string clean_message = strip_ansi(message);
            int priority = level_to_priority(level);
            syslog(priority, "%s", clean_message.c_str());
        }

        /**
         * @brief Flush syslog (no-op, syslog is unbuffered)
         */
        void flush() override {
            // Syslog is unbuffered, nothing to flush
        }

        /**
         * @brief Get program identifier
         * @return Identifier string
         */
        [[nodiscard]] const std::string &get_ident() const { return ident_; }

        /**
         * @brief Get syslog facility
         * @return Facility code
         */
        [[nodiscard]] int get_facility() const { return facility_; }
    };

#else
    // Stub for non-Unix platforms
    class SyslogSink : public Sink {
      public:
        explicit SyslogSink(const std::string & = "echo", int = 0) {
            // Not supported on this platform
        }
        void write(Level, const std::string &) override {}
        void flush() override {}
    };
#endif

} // namespace echo
