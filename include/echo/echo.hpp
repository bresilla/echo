#pragma once

/**
 * @file echo.hpp
 * @brief Echo - Modern C++20 logging library
 *
 * Single header includes everything:
 * - Core logging (levels, proxies, formatters)
 * - Multiple sinks (console always available, others via compile flags)
 * - Custom formatters and patterns
 * - Category-based filtering
 * - Visual widgets (progress bars, spinners, banners)
 * - Color utilities
 *
 * Compile-time sink selection:
 *   -DECHO_ENABLE_FILE_SINK      - Enable file logging with rotation
 *   -DECHO_ENABLE_SYSLOG_SINK    - Enable syslog integration (Unix only)
 *   -DECHO_ENABLE_NETWORK_SINK   - Enable TCP/UDP logging
 *   -DECHO_ENABLE_NULL_SINK      - Enable null sink (for testing)
 *
 * ConsoleSink is ALWAYS available (default).
 *
 * Usage:
 *   #include <echo/echo.hpp>
 *
 *   // Console logging (always available)
 *   echo::info("Hello, world!");
 *   echo::error("Error").red();
 *
 *   // Add file sink (only if ECHO_ENABLE_FILE_SINK defined)
 *   #ifdef ECHO_ENABLE_FILE_SINK
 *   echo::add_sink(std::make_shared<echo::FileSink>("app.log"));
 *   #endif
 */

// Core components (always included)
#include <echo/core/formatter.hpp>
#include <echo/core/level.hpp>
#include <echo/core/mutex.hpp>
#include <echo/core/once.hpp>
#include <echo/core/proxy.hpp>
#include <echo/core/timestamp.hpp>

// Utilities (always included)
#include <echo/utils/color.hpp>
#include <echo/utils/hash.hpp>
#include <echo/utils/string_pool.hpp>
#include <echo/utils/terminal.hpp>

// Sinks - Base and Console (always included)
#include <echo/sinks/console_sink.hpp>
#include <echo/sinks/registry.hpp>
#include <echo/sinks/sink.hpp>

// Optional sinks (compile-time)
#ifdef ECHO_ENABLE_FILE_SINK
#include <echo/sinks/file_sink.hpp>
#endif

#ifdef ECHO_ENABLE_SYSLOG_SINK
#include <echo/sinks/syslog_sink.hpp>
#endif

#ifdef ECHO_ENABLE_NETWORK_SINK
#include <echo/sinks/network_sink.hpp>
#endif

#ifdef ECHO_ENABLE_NULL_SINK
#include <echo/sinks/null_sink.hpp>
#endif

// Formatters (always included)
#include <echo/formatters/custom.hpp>
#include <echo/formatters/formatter.hpp>
#include <echo/formatters/pattern.hpp>

// Filters (always included)
#include <echo/filters/category.hpp>
// TODO: Created in future tasks
// #include <echo/filters/level.hpp>
// #include <echo/filters/composite.hpp>

// Widgets (always included)
// TODO: Split widget.hpp in future
// #include <echo/widgets/widget.hpp>
// #include <echo/widgets/progress.hpp>
// #include <echo/widgets/spinner.hpp>
// #include <echo/widgets/banner.hpp>

// String formatting (existing - keep as is for now)
#include <echo/format.hpp>

// Widgets (existing - keep as is for now, will split later)
#include <echo/widget.hpp>

namespace echo {
// All components are now available

// Version information
#define ECHO_VERSION_MAJOR 0
#define ECHO_VERSION_MINOR 1
#define ECHO_VERSION_PATCH 0
#define ECHO_VERSION "0.1.0"
} // namespace echo
