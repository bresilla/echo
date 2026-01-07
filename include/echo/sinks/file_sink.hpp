#pragma once

/**
 * @file sinks/file_sink.hpp
 * @brief File output sink with rotation support
 *
 * Only available when compiled with -DECHO_ENABLE_FILE_SINK
 */

#include <echo/sinks/sink.hpp>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>

namespace echo {

    /**
     * @brief Rotation policy for file logging
     */
    enum class RotationPolicy {
        None,      ///< No rotation
        Size,      ///< Rotate based on file size
        Daily,     ///< Rotate daily at midnight
        Hourly,    ///< Rotate every hour
        Interval,  ///< Rotate at custom time intervals
        SizeOrTime ///< Rotate when either size or time threshold is reached
    };

    /**
     * @brief File sink - writes to a file with optional rotation
     *
     * Features:
     * - Automatic ANSI color code stripping
     * - Size-based rotation
     * - Time-based rotation (daily, hourly, custom intervals)
     * - Combined size and time policies
     * - Thread-safe file operations
     * - Buffered writes for performance
     *
     * Example:
     *   auto file = std::make_shared<FileSink>("app.log");
     *   file->enable_rotation(5 * 1024 * 1024, 3);  // 5MB, keep 3 files
     *   file->set_rotation_policy(RotationPolicy::Daily);  // Daily rotation
     *   echo::add_sink(file);
     */
    class FileSink : public Sink {
      private:
        std::string filename_;
        std::ofstream file_;
        mutable std::mutex mutex_;

        // Size-based rotation
        size_t current_size_ = 0;
        size_t max_size_ = 5 * 1024 * 1024; // 5MB default
        int max_files_ = 3;
        bool rotation_enabled_ = false;

        // Time-based rotation
        RotationPolicy policy_ = RotationPolicy::None;
        std::chrono::system_clock::time_point last_rotation_time_;
        std::chrono::seconds rotation_interval_{0};

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

        /**
         * @brief Check if time-based rotation is needed
         * @return true if rotation should occur
         */
        bool should_rotate_by_time() const {
            if (policy_ == RotationPolicy::None || policy_ == RotationPolicy::Size) {
                return false;
            }

            auto now = std::chrono::system_clock::now();

            switch (policy_) {
            case RotationPolicy::Daily: {
                // Rotate at midnight
                auto now_time_t = std::chrono::system_clock::to_time_t(now);
                auto last_time_t = std::chrono::system_clock::to_time_t(last_rotation_time_);

                std::tm now_tm = *std::localtime(&now_time_t);
                std::tm last_tm = *std::localtime(&last_time_t);

                // Check if we've crossed midnight
                return (now_tm.tm_year != last_tm.tm_year || now_tm.tm_mon != last_tm.tm_mon ||
                        now_tm.tm_mday != last_tm.tm_mday);
            }

            case RotationPolicy::Hourly: {
                // Rotate every hour
                auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - last_rotation_time_);
                return elapsed.count() >= 1;
            }

            case RotationPolicy::Interval: {
                // Rotate at custom interval
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_rotation_time_);
                return elapsed >= rotation_interval_;
            }

            case RotationPolicy::SizeOrTime: {
                // Rotate if either size or time threshold is reached
                if (rotation_enabled_ && current_size_ >= max_size_) {
                    return true;
                }
                // Check time-based rotation (use interval if set, otherwise daily)
                if (rotation_interval_.count() > 0) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_rotation_time_);
                    return elapsed >= rotation_interval_;
                } else {
                    // Default to daily rotation
                    auto now_time_t = std::chrono::system_clock::to_time_t(now);
                    auto last_time_t = std::chrono::system_clock::to_time_t(last_rotation_time_);

                    std::tm now_tm = *std::localtime(&now_time_t);
                    std::tm last_tm = *std::localtime(&last_time_t);

                    return (now_tm.tm_year != last_tm.tm_year || now_tm.tm_mon != last_tm.tm_mon ||
                            now_tm.tm_mday != last_tm.tm_mday);
                }
            }

            default:
                return false;
            }
        }

        /**
         * @brief Perform file rotation
         */
        void perform_rotation() {
            file_.close();

            // Generate timestamp for rotated file
            auto now = std::chrono::system_clock::now();
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_time_t);

            char timestamp[32];
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &now_tm);

            // Rotate files: app.2.log -> app.3.log, app.1.log -> app.2.log
            for (int i = max_files_ - 1; i > 0; --i) {
                std::string old_name = filename_ + "." + std::to_string(i);
                std::string new_name = filename_ + "." + std::to_string(i + 1);
                std::rename(old_name.c_str(), new_name.c_str());
            }

            // Rename current with timestamp: app.log -> app.20260107_143022.1.log
            std::string backup = filename_ + "." + timestamp + ".1";
            std::rename(filename_.c_str(), backup.c_str());

            // Open new file
            file_.open(filename_, std::ios::app);
            current_size_ = 0;
            last_rotation_time_ = std::chrono::system_clock::now();
        }

        /**
         * @brief Rotate log files if needed (size or time-based)
         */
        void rotate_if_needed() {
            // Check size-based rotation
            bool should_rotate_size = rotation_enabled_ && current_size_ >= max_size_;

            // Check time-based rotation
            bool should_rotate_time = should_rotate_by_time();

            // Rotate if either condition is met
            if (should_rotate_size || should_rotate_time) {
                perform_rotation();
            }
        }

      public:
        /**
         * @brief Construct a file sink
         * @param filename Path to log file
         */
        explicit FileSink(const std::string &filename) : filename_(filename) {
            file_.open(filename_, std::ios::app);
            if (!file_.is_open()) {
                // TODO: Handle error - maybe throw or set error flag
            }
            last_rotation_time_ = std::chrono::system_clock::now();
        }

        ~FileSink() override {
            if (file_.is_open()) {
                file_.close();
            }
        }

        /**
         * @brief Write message to file
         * @param level Log level
         * @param message Formatted message (may contain ANSI codes)
         */
        void write(Level level, const std::string &message) override {
            if (!should_log(level)) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex_);

            if (!file_.is_open()) {
                return;
            }

            rotate_if_needed();

            // Strip ANSI codes and write
            std::string clean_message = strip_ansi(message);
            file_ << clean_message;
            current_size_ += clean_message.size();
        }

        /**
         * @brief Flush file buffer
         */
        void flush() override {
            std::lock_guard<std::mutex> lock(mutex_);
            if (file_.is_open()) {
                file_.flush();
            }
        }

        /**
         * @brief Enable log rotation
         * @param max_size Maximum file size in bytes before rotation
         * @param max_files Maximum number of rotated files to keep
         */
        void enable_rotation(size_t max_size, int max_files) {
            std::lock_guard<std::mutex> lock(mutex_);
            rotation_enabled_ = true;
            max_size_ = max_size;
            max_files_ = max_files;
        }

        /**
         * @brief Disable log rotation
         */
        void disable_rotation() {
            std::lock_guard<std::mutex> lock(mutex_);
            rotation_enabled_ = false;
        }

        /**
         * @brief Check if rotation is enabled
         * @return true if rotation is enabled
         */
        [[nodiscard]] bool is_rotation_enabled() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return rotation_enabled_;
        }

        /**
         * @brief Get current file size
         * @return Current size in bytes
         */
        [[nodiscard]] size_t get_current_size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return current_size_;
        }

        /**
         * @brief Get filename
         * @return Log file path
         */
        [[nodiscard]] const std::string &get_filename() const { return filename_; }

        /**
         * @brief Set rotation policy
         * @param policy Rotation policy to use
         *
         * Examples:
         *   sink->set_rotation_policy(RotationPolicy::Daily);
         *   sink->set_rotation_policy(RotationPolicy::Hourly);
         *   sink->set_rotation_policy(RotationPolicy::SizeOrTime);
         */
        void set_rotation_policy(RotationPolicy policy) {
            std::lock_guard<std::mutex> lock(mutex_);
            policy_ = policy;
            last_rotation_time_ = std::chrono::system_clock::now();
        }

        /**
         * @brief Set custom rotation interval
         * @param interval Time interval between rotations
         *
         * Example:
         *   sink->set_rotation_interval(std::chrono::hours(6));  // Rotate every 6 hours
         *   sink->set_rotation_interval(std::chrono::minutes(30));  // Rotate every 30 minutes
         */
        void set_rotation_interval(std::chrono::seconds interval) {
            std::lock_guard<std::mutex> lock(mutex_);
            rotation_interval_ = interval;
            policy_ = RotationPolicy::Interval;
            last_rotation_time_ = std::chrono::system_clock::now();
        }

        /**
         * @brief Get current rotation policy
         * @return Current rotation policy
         */
        [[nodiscard]] RotationPolicy get_rotation_policy() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return policy_;
        }

        /**
         * @brief Get rotation interval
         * @return Rotation interval in seconds
         */
        [[nodiscard]] std::chrono::seconds get_rotation_interval() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return rotation_interval_;
        }

        /**
         * @brief Force immediate rotation
         */
        void force_rotation() {
            std::lock_guard<std::mutex> lock(mutex_);
            perform_rotation();
        }
    };

} // namespace echo
