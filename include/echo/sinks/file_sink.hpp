#pragma once

/**
 * @file sinks/file_sink.hpp
 * @brief File output sink with rotation support
 *
 * Only available when compiled with -DECHO_ENABLE_FILE_SINK
 */

#include <echo/sinks/sink.hpp>

#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>

namespace echo {

    /**
     * @brief File sink - writes to a file with optional rotation
     *
     * Features:
     * - Automatic ANSI color code stripping
     * - Size-based rotation
     * - Thread-safe file operations
     * - Buffered writes for performance
     *
     * Example:
     *   auto file = std::make_shared<FileSink>("app.log");
     *   file->enable_rotation(5 * 1024 * 1024, 3);  // 5MB, keep 3 files
     *   echo::add_sink(file);
     */
    class FileSink : public Sink {
      private:
        std::string filename_;
        std::ofstream file_;
        mutable std::mutex mutex_;
        size_t current_size_ = 0;
        size_t max_size_ = 5 * 1024 * 1024; // 5MB default
        int max_files_ = 3;
        bool rotation_enabled_ = false;

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
         * @brief Rotate log files if needed
         */
        void rotate_if_needed() {
            if (!rotation_enabled_ || current_size_ < max_size_) {
                return;
            }

            file_.close();

            // Rotate files: app.2.log -> app.3.log, app.1.log -> app.2.log
            for (int i = max_files_ - 1; i > 0; --i) {
                std::string old_name = filename_ + "." + std::to_string(i);
                std::string new_name = filename_ + "." + std::to_string(i + 1);
                std::rename(old_name.c_str(), new_name.c_str());
            }

            // Rename current: app.log -> app.1.log
            std::string backup = filename_ + ".1";
            std::rename(filename_.c_str(), backup.c_str());

            // Open new file
            file_.open(filename_, std::ios::app);
            current_size_ = 0;
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
    };

} // namespace echo
