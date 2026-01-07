#pragma once

/**
 * @file core/once.hpp
 * @brief .once() and .every() tracking for rate-limited logging
 */

#include <echo/core/mutex.hpp>
#include <echo/utils/hash.hpp>

#include <chrono>
#include <unordered_map>
#include <unordered_set>

namespace echo {
    namespace detail {

        // =================================================================================================
        // Once tracking (for .once() functionality)
        // Uses hash-based keys to avoid string allocations in hot paths
        // =================================================================================================

        inline std::unordered_set<size_t> &get_once_set() noexcept {
            static std::unordered_set<size_t> once_set;
            return once_set;
        }

        /**
         * @brief Compute a hash key from file hash and line number
         * @param file_hash Pre-computed compile-time hash of __FILE__
         * @param line Line number
         * @return Combined hash key
         *
         * This version accepts a pre-computed hash, allowing compile-time
         * evaluation of the file path hash for better performance.
         */
        constexpr size_t make_location_key(uint64_t file_hash, int line) noexcept {
            // Combine file hash with line number using hash_combine
            return hash_combine(static_cast<size_t>(file_hash), static_cast<size_t>(line));
        }

        [[nodiscard]] inline bool check_and_mark_once(uint64_t file_hash, int line) {
            size_t key = make_location_key(file_hash, line);
            std::lock_guard<std::mutex> lock(get_log_mutex());
            if (get_once_set().count(key)) {
                return false; // Already printed
            }
            get_once_set().insert(key);
            return true; // First time
        }

        // =================================================================================================
        // Every tracking (for .every() functionality - time-based throttling)
        // Uses hash-based keys to avoid string allocations in hot paths
        // =================================================================================================

        using time_point = std::chrono::steady_clock::time_point;

        inline std::unordered_map<size_t, time_point> &get_every_map() noexcept {
            static std::unordered_map<size_t, time_point> every_map;
            return every_map;
        }

        [[nodiscard]] inline bool check_every(uint64_t file_hash, int line, int64_t interval_ms) {
            size_t key = make_location_key(file_hash, line);
            auto now = std::chrono::steady_clock::now();

            std::lock_guard<std::mutex> lock(get_log_mutex());
            auto &every_map = get_every_map();

            // Use try_emplace for atomic insert-or-update (fixes TOCTOU race condition)
            auto [it, inserted] = every_map.try_emplace(key, now);

            if (inserted) {
                return true; // First time, always print
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
            if (elapsed >= interval_ms) {
                it->second = now;
                return true; // Enough time has passed
            }

            return false; // Not enough time has passed
        }

    } // namespace detail
} // namespace echo

// =================================================================================================
// .once() and .every() macro helpers (captures call site location)
// =================================================================================================

/**
 * @brief Helper macro for .once() that captures file and line
 *
 * Uses compile-time hashing of __FILE__ for better performance.
 * The file path hash is computed at compile time, reducing runtime overhead.
 */
#define once() once_impl(echo::detail::hash_string(__FILE__), __LINE__)

/**
 * @brief Helper macro for .every(ms) that captures file and line
 *
 * Uses compile-time hashing of __FILE__ for better performance.
 * Prints at most once every N milliseconds. Useful for rate-limiting logs in tight loops.
 * Usage: echo::info("Status update").every(1000)  // prints at most once per second
 */
#define every(ms) every_impl(echo::detail::hash_string(__FILE__), __LINE__, ms)
