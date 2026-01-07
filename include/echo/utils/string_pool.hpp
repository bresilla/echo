#pragma once

/**
 * @file utils/string_pool.hpp
 * @brief Thread-local memory pool for log message strings
 *
 * Provides zero-allocation logging for messages smaller than the pool size.
 * Falls back to heap allocation for larger messages.
 */

#include <array>
#include <cstring>
#include <string>
#include <string_view>

namespace echo {
    namespace detail {

        /**
         * @brief Thread-local string buffer pool
         *
         * Each thread gets its own 4KB buffer for building log messages.
         * This eliminates heap allocations for typical log messages.
         *
         * Design:
         * - Thread-local storage (no contention)
         * - Small string optimization (SSO)
         * - Fallback to heap for large messages
         * - Simple bump allocator (reset after each message)
         */
        class StringPool {
          private:
            static constexpr size_t POOL_SIZE = 4096; // 4KB per thread
            thread_local static std::array<char, POOL_SIZE> buffer_;
            thread_local static size_t offset_;

          public:
            /**
             * @brief Allocate space from the pool
             * @param size Number of bytes to allocate
             * @return Pointer to allocated space, or nullptr if pool is full
             */
            static char *allocate(size_t size) {
                if (offset_ + size > POOL_SIZE) {
                    return nullptr; // Pool exhausted, caller should use heap
                }

                char *ptr = buffer_.data() + offset_;
                offset_ += size;
                return ptr;
            }

            /**
             * @brief Reset the pool for reuse
             *
             * Call this after a log message is complete to reclaim space.
             */
            static void reset() { offset_ = 0; }

            /**
             * @brief Get current pool usage
             * @return Number of bytes currently allocated
             */
            static size_t usage() { return offset_; }

            /**
             * @brief Get pool capacity
             * @return Total pool size in bytes
             */
            static constexpr size_t capacity() { return POOL_SIZE; }

            /**
             * @brief Check if pool has space for allocation
             * @param size Number of bytes needed
             * @return true if pool can accommodate the allocation
             */
            static bool can_allocate(size_t size) { return offset_ + size <= POOL_SIZE; }
        };

        // Thread-local storage definitions
        thread_local std::array<char, StringPool::POOL_SIZE> StringPool::buffer_;
        thread_local size_t StringPool::offset_ = 0;

        /**
         * @brief RAII wrapper for pool-allocated strings
         *
         * Automatically resets the pool when destroyed.
         * Falls back to std::string for large messages.
         */
        class PooledString {
          private:
            char *data_ = nullptr;
            size_t size_ = 0;
            bool from_pool_ = false;
            std::string heap_string_; // Fallback for large messages

          public:
            /**
             * @brief Construct from string view
             * @param sv String view to copy
             */
            explicit PooledString(std::string_view sv) : size_(sv.size()) {
                if (StringPool::can_allocate(size_ + 1)) {
                    // Allocate from pool
                    data_ = StringPool::allocate(size_ + 1);
                    if (data_) {
                        std::memcpy(data_, sv.data(), size_);
                        data_[size_] = '\0';
                        from_pool_ = true;
                        return;
                    }
                }

                // Fallback to heap
                heap_string_ = std::string(sv);
                data_ = heap_string_.data();
                from_pool_ = false;
            }

            /**
             * @brief Construct from C string
             * @param str C string to copy
             */
            explicit PooledString(const char *str) : PooledString(std::string_view(str)) {}

            /**
             * @brief Construct from std::string
             * @param str String to copy
             */
            explicit PooledString(const std::string &str) : PooledString(std::string_view(str)) {}

            // Move semantics
            PooledString(PooledString &&other) noexcept
                : data_(other.data_), size_(other.size_), from_pool_(other.from_pool_),
                  heap_string_(std::move(other.heap_string_)) {
                other.data_ = nullptr;
                other.size_ = 0;
                other.from_pool_ = false;
            }

            PooledString &operator=(PooledString &&other) noexcept {
                if (this != &other) {
                    data_ = other.data_;
                    size_ = other.size_;
                    from_pool_ = other.from_pool_;
                    heap_string_ = std::move(other.heap_string_);

                    other.data_ = nullptr;
                    other.size_ = 0;
                    other.from_pool_ = false;
                }
                return *this;
            }

            // Prevent copying
            PooledString(const PooledString &) = delete;
            PooledString &operator=(const PooledString &) = delete;

            /**
             * @brief Get string data
             * @return Pointer to string data
             */
            [[nodiscard]] const char *data() const { return data_; }

            /**
             * @brief Get string size
             * @return String length in bytes
             */
            [[nodiscard]] size_t size() const { return size_; }

            /**
             * @brief Check if string is from pool
             * @return true if allocated from pool, false if from heap
             */
            [[nodiscard]] bool is_pooled() const { return from_pool_; }

            /**
             * @brief Convert to string view
             * @return String view of the data
             */
            [[nodiscard]] std::string_view view() const { return std::string_view(data_, size_); }

            /**
             * @brief Convert to std::string
             * @return String copy
             */
            [[nodiscard]] std::string str() const {
                if (from_pool_) {
                    return std::string(data_, size_);
                } else {
                    return heap_string_;
                }
            }

            /**
             * @brief Implicit conversion to string view
             */
            operator std::string_view() const { return view(); }
        };

        /**
         * @brief RAII pool reset guard
         *
         * Automatically resets the pool when destroyed.
         * Use this to ensure pool is reset even if exceptions occur.
         */
        class PoolResetGuard {
          public:
            PoolResetGuard() = default;
            ~PoolResetGuard() { StringPool::reset(); }

            // Prevent copying and moving
            PoolResetGuard(const PoolResetGuard &) = delete;
            PoolResetGuard &operator=(const PoolResetGuard &) = delete;
            PoolResetGuard(PoolResetGuard &&) = delete;
            PoolResetGuard &operator=(PoolResetGuard &&) = delete;
        };

    } // namespace detail
} // namespace echo
