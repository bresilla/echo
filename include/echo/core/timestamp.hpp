#pragma once

/**
 * @file core/timestamp.hpp
 * @brief Timestamp generation for logging
 */

#ifdef ECHO_ENABLE_TIMESTAMP
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#endif

namespace echo {
    namespace detail {

        // =================================================================================================
        // Timestamp support
        // =================================================================================================

#ifdef ECHO_ENABLE_TIMESTAMP
        inline std::string get_timestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto tm = *std::localtime(&time);

            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":" << std::setw(2) << tm.tm_min << ":"
                << std::setw(2) << tm.tm_sec;
            return oss.str();
        }
#endif

    } // namespace detail
} // namespace echo
