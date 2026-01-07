#pragma once

/**
 * @file core/formatter.hpp
 * @brief Message formatting and type conversion utilities
 */

#include <sstream>
#include <string>
#include <type_traits>

namespace echo {
    namespace detail {

        // =================================================================================================
        // Type traits for detecting print methods
        // =================================================================================================

        template <typename T, typename = void> struct has_pretty : std::false_type {};

        template <typename T>
        struct has_pretty<T, std::void_t<decltype(std::declval<T>().pretty())>> : std::true_type {};

        template <typename T, typename = void> struct has_print : std::false_type {};

        template <typename T> struct has_print<T, std::void_t<decltype(std::declval<T>().print())>> : std::true_type {};

        template <typename T, typename = void> struct has_to_string : std::false_type {};

        template <typename T>
        struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

        template <typename T, typename = void> struct is_streamable : std::false_type {};

        template <typename T>
        struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
            : std::true_type {};

        // =================================================================================================
        // Convert value to string
        // =================================================================================================

        template <typename T> inline std::string stringify(const T &value) {
            if constexpr (has_pretty<T>::value) {
                return value.pretty();
            } else if constexpr (has_print<T>::value) {
                return value.print();
            } else if constexpr (has_to_string<T>::value) {
                return value.to_string();
            } else if constexpr (std::is_same_v<T, std::string>) {
                return value;
            } else if constexpr (std::is_same_v<T, const char *> || std::is_same_v<T, char *>) {
                return std::string(value);
            } else if constexpr (is_streamable<T>::value) {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            } else {
                return "[unprintable]";
            }
        }

        // =================================================================================================
        // Variadic argument formatting
        // =================================================================================================

        inline void append_args(std::ostringstream &) {}

        template <typename T, typename... Args>
        inline void append_args(std::ostringstream &oss, const T &first, const Args &...rest) {
            oss << stringify(first);
            append_args(oss, rest...);
        }

    } // namespace detail

    // =================================================================================================
    // Structured logging (key-value pairs)
    // =================================================================================================

    namespace detail {
        // Base case: no more arguments
        inline void append_kv(std::ostringstream &) {}

        // Recursive case: key-value pairs
        template <typename K, typename V, typename... Rest>
        inline void append_kv(std::ostringstream &oss, const K &key, const V &value, const Rest &...rest) {
            oss << stringify(key) << "=" << stringify(value);
            if constexpr (sizeof...(rest) > 0) {
                oss << " ";
                append_kv(oss, rest...);
            }
        }
    } // namespace detail

    template <typename... Args> inline std::string kv(const Args &...args) {
        static_assert(sizeof...(args) % 2 == 0, "kv() requires an even number of arguments (key-value pairs)");
        std::ostringstream oss;
        detail::append_kv(oss, args...);
        return oss.str();
    }

} // namespace echo
