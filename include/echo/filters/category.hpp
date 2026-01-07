#pragma once

/**
 * @file filters/category.hpp
 * @brief Category-based log filtering
 *
 * Allows filtering logs by category or module name with hierarchical support.
 *
 * Features:
 * - Tag logs with category: echo::category("network").info("msg")
 * - Set per-category log levels
 * - Category hierarchy support (e.g., "app.network.tcp")
 * - Wildcard filtering ("app.*")
 *
 * Example:
 *   echo::category("network").info("Connected");
 *   echo::set_category_level("network", Level::Warn);
 *   echo::set_category_level("app.*", Level::Debug);
 */

#include <echo/core/level.hpp>
#include <echo/core/mutex.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace echo {

    namespace detail {
        /**
         * @brief Thread-safe category registry
         *
         * Manages category-specific log levels with hierarchical support.
         */
        class CategoryRegistry {
          private:
            // Map of category name to log level
            std::unordered_map<std::string, Level> category_levels_;
            mutable std::mutex mutex_;

            /**
             * @brief Check if a pattern matches a category name
             * @param pattern Pattern with optional wildcard (e.g., "app.*")
             * @param category Category name to match
             * @return true if pattern matches
             */
            static bool matches_pattern(const std::string &pattern, const std::string &category) {
                // Exact match
                if (pattern == category) {
                    return true;
                }

                // Wildcard match: "app.*" matches "app.network", "app.network.tcp", etc.
                if (pattern.size() >= 2 && pattern.substr(pattern.size() - 2) == ".*") {
                    std::string prefix = pattern.substr(0, pattern.size() - 2);
                    // Match if category starts with prefix and has a dot after it
                    if (category.size() > prefix.size() && category.substr(0, prefix.size()) == prefix &&
                        (category[prefix.size()] == '.' || category.size() == prefix.size())) {
                        return true;
                    }
                }

                return false;
            }

            /**
             * @brief Find the most specific matching level for a category
             * @param category Category name
             * @return Level if found, nullopt otherwise
             */
            std::optional<Level> find_matching_level(const std::string &category) const {
                // First, try exact match
                auto it = category_levels_.find(category);
                if (it != category_levels_.end()) {
                    return it->second;
                }

                // Then, try hierarchical match (most specific first)
                // For "app.network.tcp", try "app.network.*", then "app.*"
                std::string current = category;
                while (true) {
                    size_t last_dot = current.find_last_of('.');
                    if (last_dot == std::string::npos) {
                        break;
                    }

                    current = current.substr(0, last_dot);
                    std::string pattern = current + ".*";

                    it = category_levels_.find(pattern);
                    if (it != category_levels_.end()) {
                        return it->second;
                    }
                }

                // Finally, try wildcard patterns
                for (const auto &[pattern, level] : category_levels_) {
                    if (matches_pattern(pattern, category)) {
                        return level;
                    }
                }

                return std::nullopt;
            }

          public:
            /**
             * @brief Get the singleton instance
             */
            static CategoryRegistry &instance() {
                static CategoryRegistry registry;
                return registry;
            }

            /**
             * @brief Set log level for a category
             * @param category Category name (supports wildcards like "app.*")
             * @param level Log level
             */
            void set_level(const std::string &category, Level level) {
                std::lock_guard<std::mutex> lock(mutex_);
                category_levels_[category] = level;
            }

            /**
             * @brief Get log level for a category
             * @param category Category name
             * @return Level if set, nullopt otherwise
             */
            std::optional<Level> get_level(const std::string &category) const {
                std::lock_guard<std::mutex> lock(mutex_);
                return find_matching_level(category);
            }

            /**
             * @brief Check if a log should be printed for a category
             * @param category Category name
             * @param level Log level
             * @return true if log should be printed
             */
            bool should_log(const std::string &category, Level level) const {
                std::lock_guard<std::mutex> lock(mutex_);

                // Get category-specific level
                auto cat_level = find_matching_level(category);
                if (cat_level.has_value()) {
                    return static_cast<int>(level) >= static_cast<int>(cat_level.value());
                }

                // No category-specific level, use global level
                return static_cast<int>(level) >= static_cast<int>(get_effective_level());
            }

            /**
             * @brief Clear all category levels
             */
            void clear() {
                std::lock_guard<std::mutex> lock(mutex_);
                category_levels_.clear();
            }

            /**
             * @brief Get all registered categories
             * @return Vector of category names
             */
            std::vector<std::string> get_categories() const {
                std::lock_guard<std::mutex> lock(mutex_);
                std::vector<std::string> categories;
                categories.reserve(category_levels_.size());
                for (const auto &[cat, _] : category_levels_) {
                    categories.push_back(cat);
                }
                return categories;
            }
        };

    } // namespace detail

    // =================================================================================================
    // Public API for category filtering
    // =================================================================================================

    /**
     * @brief Set log level for a category
     * @param category Category name (supports wildcards like "app.*")
     * @param level Log level
     *
     * Examples:
     *   echo::set_category_level("network", Level::Warn);
     *   echo::set_category_level("app.*", Level::Debug);
     *   echo::set_category_level("app.network.tcp", Level::Trace);
     */
    inline void set_category_level(const std::string &category, Level level) {
        detail::CategoryRegistry::instance().set_level(category, level);
    }

    /**
     * @brief Get log level for a category
     * @param category Category name
     * @return Level if set, nullopt otherwise
     */
    inline std::optional<Level> get_category_level(const std::string &category) {
        return detail::CategoryRegistry::instance().get_level(category);
    }

    /**
     * @brief Clear all category levels
     */
    inline void clear_category_levels() { detail::CategoryRegistry::instance().clear(); }

    /**
     * @brief Get all registered categories
     * @return Vector of category names
     */
    inline std::vector<std::string> get_categories() { return detail::CategoryRegistry::instance().get_categories(); }

    // Forward declaration of category_proxy (defined below)
    class category_proxy;

    /**
     * @brief Create a category-aware logging proxy
     * @param category Category name
     * @return Category proxy for fluent logging
     *
     * Example:
     *   echo::category("network").info("Connected");
     *   echo::category("app.database").error("Connection failed");
     */
    inline category_proxy category(const std::string &category);

    // =================================================================================================
    // Category-aware logging proxy
    // =================================================================================================

    // Forward declare log_proxy from proxy.hpp
    template <Level L> class log_proxy;

    /**
     * @brief Category-aware log proxy template
     *
     * Wraps a log_proxy with category filtering.
     */
    template <Level L> class category_log_proxy {
      private:
        std::string category_;
        log_proxy<L> proxy_;
        bool should_log_;

      public:
        template <typename... Args>
        category_log_proxy(std::string category, const Args &...args)
            : category_(std::move(category)), proxy_(args...), should_log_(true) {
            // Check if this category should log at this level
            if (!detail::CategoryRegistry::instance().should_log(category_, L)) {
                should_log_ = false;
            }
        }

        // Move semantics
        category_log_proxy(category_log_proxy &&other) noexcept
            : category_(std::move(other.category_)), proxy_(std::move(other.proxy_)), should_log_(other.should_log_) {
            other.should_log_ = false;
        }

        // Prevent copying
        category_log_proxy(const category_log_proxy &) = delete;
        category_log_proxy &operator=(const category_log_proxy &) = delete;
        category_log_proxy &operator=(category_log_proxy &&) = delete;

        // Forward color methods to underlying proxy
        category_log_proxy &red() {
            if (should_log_)
                proxy_.red();
            return *this;
        }
        category_log_proxy &green() {
            if (should_log_)
                proxy_.green();
            return *this;
        }
        category_log_proxy &yellow() {
            if (should_log_)
                proxy_.yellow();
            return *this;
        }
        category_log_proxy &blue() {
            if (should_log_)
                proxy_.blue();
            return *this;
        }
        category_log_proxy &magenta() {
            if (should_log_)
                proxy_.magenta();
            return *this;
        }
        category_log_proxy &cyan() {
            if (should_log_)
                proxy_.cyan();
            return *this;
        }
        category_log_proxy &white() {
            if (should_log_)
                proxy_.white();
            return *this;
        }
        category_log_proxy &gray() {
            if (should_log_)
                proxy_.gray();
            return *this;
        }
        category_log_proxy &bold() {
            if (should_log_)
                proxy_.bold();
            return *this;
        }
        category_log_proxy &italic() {
            if (should_log_)
                proxy_.italic();
            return *this;
        }
        category_log_proxy &underline() {
            if (should_log_)
                proxy_.underline();
            return *this;
        }
        category_log_proxy &hex(const std::string &hex_color) {
            if (should_log_)
                proxy_.hex(hex_color);
            return *this;
        }
        category_log_proxy &rgb(int r, int g, int b) {
            if (should_log_)
                proxy_.rgb(r, g, b);
            return *this;
        }
        category_log_proxy &when(bool condition) {
            if (should_log_)
                proxy_.when(condition);
            return *this;
        }
        category_log_proxy &inplace() {
            if (should_log_)
                proxy_.inplace();
            return *this;
        }

        // Destructor - proxy destructor will handle actual logging
        ~category_log_proxy() {
            // If category filtering says don't log, skip the proxy
            if (!should_log_) {
                // Need to prevent proxy from logging
                // This is handled by the when() method
                proxy_.when(false);
            }
        }
    };

    /**
     * @brief Proxy for category-aware logging
     *
     * Allows fluent logging with category filtering:
     *   echo::category("network").info("message")
     *   echo::category("app.database").error("error")
     */
    class category_proxy {
      private:
        std::string category_;

      public:
        explicit category_proxy(std::string category) : category_(std::move(category)) {}

        /**
         * @brief Log a trace message
         */
        template <typename... Args> auto trace(const Args &...args) const {
            return category_log_proxy<Level::Trace>(category_, args...);
        }

        /**
         * @brief Log a debug message
         */
        template <typename... Args> auto debug(const Args &...args) const {
            return category_log_proxy<Level::Debug>(category_, args...);
        }

        /**
         * @brief Log an info message
         */
        template <typename... Args> auto info(const Args &...args) const {
            return category_log_proxy<Level::Info>(category_, args...);
        }

        /**
         * @brief Log a warning message
         */
        template <typename... Args> auto warn(const Args &...args) const {
            return category_log_proxy<Level::Warn>(category_, args...);
        }

        /**
         * @brief Log an error message
         */
        template <typename... Args> auto error(const Args &...args) const {
            return category_log_proxy<Level::Error>(category_, args...);
        }

        /**
         * @brief Log a critical message
         */
        template <typename... Args> auto critical(const Args &...args) const {
            return category_log_proxy<Level::Critical>(category_, args...);
        }

        /**
         * @brief Get the category name
         */
        [[nodiscard]] const std::string &name() const { return category_; }
    };

    // Implementation of category() function
    inline category_proxy category(const std::string &category) { return category_proxy(category); }

} // namespace echo
