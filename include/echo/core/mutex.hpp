#pragma once

/**
 * @file core/mutex.hpp
 * @brief Thread safety primitives for Echo logging
 */

#include <mutex>

namespace echo {
    namespace detail {

        // =================================================================================================
        // Thread safety
        // =================================================================================================

        inline std::mutex &get_log_mutex() noexcept {
            static std::mutex log_mutex;
            return log_mutex;
        }

    } // namespace detail
} // namespace echo
