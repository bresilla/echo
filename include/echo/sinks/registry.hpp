#pragma once

/**
 * @file sinks/registry.hpp
 * @brief Sink registry for managing multiple output destinations
 */

#include <echo/core/level.hpp>
#include <echo/core/mutex.hpp>
#include <echo/sinks/console_sink.hpp>
#include <echo/sinks/sink.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace echo {

    namespace detail {
        // Forward declarations from proxy.hpp
        using SinkWriterFunc = void (*)(Level, const std::string &);
        using PrintWriterFunc = void (*)(const std::string &);
        SinkWriterFunc &get_sink_writer();
        PrintWriterFunc &get_print_writer();

        /**
         * @brief Global sink registry
         *
         * Manages all registered sinks. Thread-safe for concurrent access.
         */
        class SinkRegistry {
          private:
            std::vector<SinkPtr> sinks_;
            mutable std::mutex mutex_;
            bool initialized_ = false;

            /**
             * @brief Initialize default ConsoleSink if no sinks are registered
             */
            void ensure_default_sink() {
                if (!initialized_) {
                    initialized_ = true;
                    if (sinks_.empty()) {
                        // Add default ConsoleSink
                        sinks_.push_back(std::make_shared<ConsoleSink>());
                    }
                }
            }

          public:
            /**
             * @brief Get the singleton instance
             */
            static SinkRegistry &instance() {
                static SinkRegistry registry;
                return registry;
            }

            /**
             * @brief Add a sink to the registry
             * @param sink Shared pointer to the sink
             */
            void add(SinkPtr sink) {
                if (!sink)
                    return;
                std::lock_guard<std::mutex> lock(mutex_);
                sinks_.push_back(sink);
            }

            /**
             * @brief Remove a sink from the registry
             * @param sink Shared pointer to the sink to remove
             */
            void remove(SinkPtr sink) {
                if (!sink)
                    return;
                std::lock_guard<std::mutex> lock(mutex_);
                sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
            }

            /**
             * @brief Remove all sinks
             */
            void clear() {
                std::lock_guard<std::mutex> lock(mutex_);
                sinks_.clear();
            }

            /**
             * @brief Write a message to all registered sinks
             * @param level Log level
             * @param message Formatted message
             */
            void write_all(Level level, const std::string &message) {
                std::lock_guard<std::mutex> lock(mutex_);
                ensure_default_sink();
                for (auto &sink : sinks_) {
                    if (sink && sink->should_log(level)) {
                        sink->write(level, message);
                    }
                }
            }

            /**
             * @brief Flush all registered sinks
             */
            void flush_all() {
                std::lock_guard<std::mutex> lock(mutex_);
                for (auto &sink : sinks_) {
                    if (sink) {
                        sink->flush();
                    }
                }
            }

            /**
             * @brief Get number of registered sinks
             * @return Number of sinks
             */
            [[nodiscard]] size_t count() const {
                std::lock_guard<std::mutex> lock(mutex_);
                return sinks_.size();
            }

            /**
             * @brief Check if any sinks are registered
             * @return true if at least one sink is registered
             */
            [[nodiscard]] bool empty() const {
                std::lock_guard<std::mutex> lock(mutex_);
                return sinks_.empty();
            }
        };

    } // namespace detail

    // =================================================================================================
    // Public API for sink management
    // =================================================================================================

    /**
     * @brief Add a sink to the logging system
     * @param sink Shared pointer to the sink
     *
     * Example:
     *   echo::add_sink(std::make_shared<echo::ConsoleSink>());
     *   echo::add_sink(std::make_shared<echo::FileSink>("app.log"));
     */
    inline void add_sink(SinkPtr sink) { detail::SinkRegistry::instance().add(sink); }

    /**
     * @brief Remove a sink from the logging system
     * @param sink Shared pointer to the sink to remove
     */
    inline void remove_sink(SinkPtr sink) { detail::SinkRegistry::instance().remove(sink); }

    /**
     * @brief Remove all sinks from the logging system
     */
    inline void clear_sinks() { detail::SinkRegistry::instance().clear(); }

    /**
     * @brief Flush all registered sinks
     *
     * Forces all sinks to write any buffered data to their destinations.
     * Useful before program exit or after critical messages.
     */
    inline void flush() { detail::SinkRegistry::instance().flush_all(); }

    /**
     * @brief Get number of registered sinks
     * @return Number of sinks
     */
    [[nodiscard]] inline size_t sink_count() { return detail::SinkRegistry::instance().count(); }

    // =================================================================================================
    // Set up sink writers for proxy.hpp
    // =================================================================================================

    namespace detail {
        // Actual sink writer implementations
        inline void registry_write_to_sinks(Level level, const std::string &formatted_message) {
            SinkRegistry::instance().write_all(level, formatted_message);
        }

        inline void registry_write_print_to_sinks(const std::string &formatted_message) {
            // For print messages, use Info level
            SinkRegistry::instance().write_all(Level::Info, formatted_message);
        }

        // Initialize the function pointers (called automatically via static initialization)
        struct SinkWriterInitializer {
            SinkWriterInitializer() {
                get_sink_writer() = registry_write_to_sinks;
                get_print_writer() = registry_write_print_to_sinks;
            }
        };

        // Static initializer - runs before main()
        static SinkWriterInitializer sink_writer_init;

        // Provide access to sink registry
        inline SinkRegistry &get_sink_registry() { return SinkRegistry::instance(); }
    } // namespace detail

} // namespace echo
