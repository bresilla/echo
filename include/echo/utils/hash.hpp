#pragma once

/**
 * @file utils/hash.hpp
 * @brief Compile-time string hashing utilities
 */

#include <cstddef>
#include <cstdint>

namespace echo {
    namespace detail {

        // =================================================================================================
        // Compile-time FNV-1a hash
        // =================================================================================================

        /**
         * @brief FNV-1a hash constants
         */
        constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;

        /**
         * @brief Compile-time FNV-1a hash function
         * @param str String to hash
         * @param len Length of string
         * @return Hash value
         *
         * FNV-1a (Fowler-Noll-Vo) is a fast, non-cryptographic hash function
         * with good distribution properties. Perfect for hash tables.
         *
         * This implementation is constexpr, allowing compile-time evaluation
         * when used with string literals like __FILE__.
         */
        constexpr uint64_t hash_fnv1a(const char *str, size_t len) noexcept {
            uint64_t hash = FNV_OFFSET_BASIS;
            for (size_t i = 0; i < len; ++i) {
                hash ^= static_cast<uint64_t>(static_cast<unsigned char>(str[i]));
                hash *= FNV_PRIME;
            }
            return hash;
        }

        /**
         * @brief Compile-time string length calculation
         * @param str Null-terminated string
         * @return Length of string (excluding null terminator)
         */
        constexpr size_t constexpr_strlen(const char *str) noexcept {
            size_t len = 0;
            while (str[len] != '\0') {
                ++len;
            }
            return len;
        }

        /**
         * @brief Compile-time string hash (convenience wrapper)
         * @param str Null-terminated string
         * @return Hash value
         *
         * Usage:
         *   constexpr auto hash = hash_string(__FILE__);
         */
        constexpr uint64_t hash_string(const char *str) noexcept { return hash_fnv1a(str, constexpr_strlen(str)); }

        /**
         * @brief Combine two hash values
         * @param seed First hash value
         * @param value Second hash value
         * @return Combined hash
         *
         * Uses boost::hash_combine algorithm
         */
        constexpr size_t hash_combine(size_t seed, size_t value) noexcept {
            return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }

    } // namespace detail
} // namespace echo
