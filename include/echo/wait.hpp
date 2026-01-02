#pragma once

/**
 * @file wait.hpp
 * @brief Progress bars, spinners, and step indicators for Echo logging library
 *
 * This header provides progress tracking and waiting indicators.
 * It includes echo.hpp automatically, so you only need to include this file.
 *
 * Usage:
 *   #include <echo/wait.hpp>
 *
 *   // Progress bar
 *   echo::progress_bar bar(100);
 *   bar.tick();
 *   bar.set_progress(50);
 *   bar.finish();
 *
 *   // Spinner
 *   echo::spinner spin;
 *   spin.tick();
 *   spin.stop("Done!");
 *
 *   // Steps
 *   echo::steps workflow({"Init", "Load", "Process"});
 *   workflow.next();
 *   workflow.complete();
 *
 * Features:
 *   - Progress bars with percentage and time tracking
 *   - 15 ASCII/ANSI spinner styles
 *   - Step indicators (finite and infinite)
 *   - Thread-safe operations
 *   - Terminal-aware
 *   - HEX color support with gradients
 */

#include <echo/color.hpp>
#include <echo/echo.hpp>

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

namespace echo {

    // =================================================================================================
    // Cursor control utilities
    // =================================================================================================

    namespace detail {

        inline void hide_cursor() { std::cout << "\033[?25l" << std::flush; }

        inline void show_cursor() { std::cout << "\033[?25h" << std::flush; }

        inline void move_cursor_up(int lines = 1) { std::cout << "\033[" << lines << "A" << std::flush; }

        inline void clear_line() { std::cout << "\033[2K\r" << std::flush; }

        inline std::mutex &get_wait_mutex() {
            static std::mutex mtx;
            return mtx;
        }

    } // namespace detail

    // =================================================================================================
    // Spinner styles
    // =================================================================================================

    enum class spinner_style {
        line,           // - \ | /
        pipe,           // ┤ ┘ ┴ └ ├ ┌ ┬ ┐
        simple_dots,    // . .. ...
        dots_scrolling, // . .. ... .. .
        flip,           // _ - ` ' ´
        toggle,         // = * -
        layer,          // - = ≡
        point,          // ∙∙∙ ●∙∙ ∙●∙
        dqpb,           // d q p b
        bouncing_bar,   // [====]
        bouncing_ball,  // ( ● )
        aesthetic,      // ▰▰▰▰▰▰▰
        binary,         // 010010
        grow_vertical,  // ▁ ▃ ▄ ▅ ▆ ▇
        grow_horizontal // ▏ ▎ ▍ ▌ ▋
    };

    // =================================================================================================
    // Spinner class
    // =================================================================================================

    class spinner {
      private:
        spinner_style style_;
        std::vector<std::string> frames_;
        int interval_ms_;
        size_t current_frame_ = 0;
        std::string prefix_;
        std::string message_;
        bool is_running_ = true;
        std::vector<std::string> gradient_colors_;
        bool use_gradient_ = false;

        void init_frames() {
            switch (style_) {
            case spinner_style::line:
                frames_ = {"-", "\\", "|", "/"};
                interval_ms_ = 130;
                break;
            case spinner_style::pipe:
                frames_ = {"┤", "┘", "┴", "└", "├", "┌", "┬", "┐"};
                interval_ms_ = 100;
                break;
            case spinner_style::simple_dots:
                frames_ = {".  ", ".. ", "...", "   "};
                interval_ms_ = 400;
                break;
            case spinner_style::dots_scrolling:
                frames_ = {".  ", ".. ", "...", " ..", "  .", "   "};
                interval_ms_ = 200;
                break;
            case spinner_style::flip:
                frames_ = {"_", "_", "_", "-", "`", "`", "'", "´", "-", "_", "_", "_"};
                interval_ms_ = 70;
                break;
            case spinner_style::toggle:
                frames_ = {"=", "*", "-"};
                interval_ms_ = 80;
                break;
            case spinner_style::layer:
                frames_ = {"-", "=", "≡"};
                interval_ms_ = 150;
                break;
            case spinner_style::point:
                frames_ = {"∙∙∙", "●∙∙", "∙●∙", "∙∙●", "∙∙∙"};
                interval_ms_ = 125;
                break;
            case spinner_style::dqpb:
                frames_ = {"d", "q", "p", "b"};
                interval_ms_ = 100;
                break;
            case spinner_style::bouncing_bar:
                frames_ = {"[    ]", "[=   ]", "[==  ]", "[=== ]", "[====]", "[ ===]", "[  ==]", "[   =]",
                           "[    ]", "[   =]", "[  ==]", "[ ===]", "[====]", "[=== ]", "[==  ]", "[=   ]"};
                interval_ms_ = 80;
                break;
            case spinner_style::bouncing_ball:
                frames_ = {"( ●    )", "(  ●   )", "(   ●  )", "(    ● )", "(     ●)",
                           "(    ● )", "(   ●  )", "(  ●   )", "( ●    )", "(●     )"};
                interval_ms_ = 80;
                break;
            case spinner_style::aesthetic:
                frames_ = {"▰▱▱▱▱▱▱", "▰▰▱▱▱▱▱", "▰▰▰▱▱▱▱", "▰▰▰▰▱▱▱", "▰▰▰▰▰▱▱", "▰▰▰▰▰▰▱", "▰▰▰▰▰▰▰", "▰▱▱▱▱▱▱"};
                interval_ms_ = 80;
                break;
            case spinner_style::binary:
                frames_ = {"010010", "001100", "100101", "111010", "111101",
                           "010111", "101011", "111000", "110011", "110101"};
                interval_ms_ = 80;
                break;
            case spinner_style::grow_vertical:
                frames_ = {"▁", "▃", "▄", "▅", "▆", "▇", "▆", "▅", "▄", "▃"};
                interval_ms_ = 120;
                break;
            case spinner_style::grow_horizontal:
                frames_ = {"▏", "▎", "▍", "▌", "▋", "▊", "▉", "▊", "▋", "▌", "▍", "▎"};
                interval_ms_ = 120;
                break;
            }
        }

      public:
        spinner(spinner_style style = spinner_style::line) : style_(style) { init_frames(); }

        void set_message(const std::string &message) { message_ = message; }

        void set_prefix(const std::string &prefix) { prefix_ = prefix; }

        void set_color(const std::string &hex) {
            gradient_colors_ = {hex};
            use_gradient_ = false;
        }

        void set_gradient(const std::vector<std::string> &hexes) {
            gradient_colors_ = hexes;
            use_gradient_ = true;
        }

        void tick() {
            if (!is_running_)
                return;

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            detail::clear_line();

            // Get color for current frame
            std::string color_code;
            if (!gradient_colors_.empty()) {
                if (use_gradient_) {
                    // Cycle through gradient colors
                    float pos = static_cast<float>(current_frame_) / static_cast<float>(frames_.size() - 1);
                    color_code = detail::get_gradient_color(gradient_colors_, pos);
                } else {
                    // Single color
                    color_code = detail::get_single_color(gradient_colors_[0]);
                }
            }

            std::cout << prefix_;
            if (!prefix_.empty())
                std::cout << " ";
            std::cout << color_code << frames_[current_frame_] << detail::reset_color();
            if (!message_.empty())
                std::cout << " " << message_;
            std::cout << std::flush;

            current_frame_ = (current_frame_ + 1) % frames_.size();
        }

        void stop(const std::string &final_message = "") {
            if (!is_running_)
                return;

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            is_running_ = false;
            detail::clear_line();

            if (!final_message.empty()) {
                std::cout << final_message << "\n";
            }
        }

        int get_interval_ms() const { return interval_ms_; }

        bool is_running() const { return is_running_; }
    };

    // =================================================================================================
    // Progress bar class
    // =================================================================================================

    class progress_bar {
      private:
        size_t current_ = 0;
        size_t total_ = 100;
        int bar_width_ = 50;
        char fill_char_ = '=';
        char lead_char_ = '>';
        char remainder_char_ = ' ';
        std::string prefix_;
        std::string postfix_;
        bool show_percentage_ = true;
        bool show_elapsed_ = false;
        bool show_remaining_ = false;
        std::chrono::steady_clock::time_point start_time_;
        bool started_ = false;
        std::vector<std::string> gradient_colors_;
        bool use_gradient_ = false;

        std::string format_time(int seconds) const {
            int mins = seconds / 60;
            int secs = seconds % 60;
            if (mins > 0) {
                return std::to_string(mins) + "m" + std::to_string(secs) + "s";
            }
            return std::to_string(secs) + "s";
        }

      public:
        progress_bar(size_t total = 100) : total_(total) { start_time_ = std::chrono::steady_clock::now(); }

        void set_bar_width(int width) { bar_width_ = width; }

        void set_fill_char(char c) { fill_char_ = c; }

        void set_lead_char(char c) { lead_char_ = c; }

        void set_remainder_char(char c) { remainder_char_ = c; }

        void set_prefix(const std::string &prefix) { prefix_ = prefix; }

        void set_postfix(const std::string &postfix) { postfix_ = postfix; }

        void set_show_percentage(bool show) { show_percentage_ = show; }

        void set_show_elapsed(bool show) { show_elapsed_ = show; }

        void set_show_remaining(bool show) { show_remaining_ = show; }

        void set_color(const std::string &hex) {
            gradient_colors_ = {hex};
            use_gradient_ = false;
        }

        void set_gradient(const std::vector<std::string> &hexes) {
            gradient_colors_ = hexes;
            use_gradient_ = true;
        }

        void tick() {
            if (current_ < total_) {
                current_++;
            }
            display();
        }

        void set_progress(size_t value) {
            if (value <= total_) {
                current_ = value;
            }
            display();
        }

        void display() {
            if (!started_) {
                started_ = true;
                start_time_ = std::chrono::steady_clock::now();
            }

            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            detail::clear_line();

            // Prefix
            if (!prefix_.empty()) {
                std::cout << prefix_ << " ";
            }

            // Calculate progress
            float progress = total_ > 0 ? static_cast<float>(current_) / static_cast<float>(total_) : 0.0f;
            int filled = static_cast<int>(progress * bar_width_);

            // Bar
            std::cout << "[";
            for (int i = 0; i < bar_width_; ++i) {
                // Get color for this position
                std::string color_code;
                if (!gradient_colors_.empty()) {
                    if (use_gradient_) {
                        // Gradient follows progress
                        float pos = static_cast<float>(i) / static_cast<float>(bar_width_ - 1);
                        color_code = detail::get_gradient_color(gradient_colors_, pos);
                    } else {
                        // Single color
                        color_code = detail::get_single_color(gradient_colors_[0]);
                    }
                }

                if (i < filled - 1) {
                    std::cout << color_code << fill_char_ << detail::reset_color();
                } else if (i == filled - 1 && filled < bar_width_) {
                    std::cout << color_code << lead_char_ << detail::reset_color();
                } else {
                    std::cout << remainder_char_;
                }
            }
            std::cout << "]";

            // Percentage
            if (show_percentage_) {
                std::cout << " " << static_cast<int>(progress * 100) << "%";
            }

            // Time tracking
            if (show_elapsed_ || show_remaining_) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();

                std::cout << " [";
                if (show_elapsed_) {
                    std::cout << format_time(elapsed);
                }

                if (show_remaining_ && current_ > 0 && current_ < total_) {
                    int remaining = static_cast<int>((elapsed * (total_ - current_)) / current_);
                    std::cout << "<" << format_time(remaining);
                }
                std::cout << "]";
            }

            // Postfix
            if (!postfix_.empty()) {
                std::cout << " " << postfix_;
            }

            std::cout << std::flush;
        }

        void finish() {
            current_ = total_;
            display();
            std::cout << "\n";
        }

        bool is_completed() const { return current_ >= total_; }

        size_t get_progress() const { return current_; }
    };

    // =================================================================================================
    // Steps class
    // =================================================================================================

    class steps {
      private:
        std::vector<std::string> step_names_;
        size_t current_step_ = 0;
        bool is_infinite_ = false;
        std::vector<bool> completed_steps_;
        std::string step_color_;

      public:
        // Finite steps (known total)
        steps(const std::vector<std::string> &step_names) : step_names_(step_names), is_infinite_(false) {
            completed_steps_.resize(step_names_.size(), false);
        }

        // Infinite steps (unknown total)
        steps() : is_infinite_(true) {}

        void add_step(const std::string &step_name) {
            if (is_infinite_) {
                step_names_.push_back(step_name);
                completed_steps_.push_back(false);
            }
        }

        void set_color(const std::string &hex) { step_color_ = hex; }

        void next() {
            if (current_step_ < step_names_.size()) {
                display_current();
                current_step_++;
            }
        }

        void complete() {
            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                completed_steps_[current_step_ - 1] = true;
                display_completed();
            }
        }

        void fail() {
            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                display_failed();
            }
        }

        void display_current() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ < step_names_.size()) {
                if (is_infinite_) {
                    std::cout << color_code << "Step " << (current_step_ + 1) << ": " << step_names_[current_step_]
                              << reset << "\n";
                } else {
                    std::cout << color_code << "Step " << (current_step_ + 1) << "/" << step_names_.size() << ": "
                              << step_names_[current_step_] << reset << "\n";
                }
            }
        }

        void display_completed() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                std::cout << color_code << "✓ " << step_names_[current_step_ - 1] << " - Complete" << reset << "\n";
            }
        }

        void display_failed() {
            std::lock_guard<std::mutex> lock(detail::get_wait_mutex());
            std::string color_code = detail::get_single_color(step_color_);
            std::string reset = color_code.empty() ? "" : detail::reset_color();

            if (current_step_ > 0 && current_step_ <= step_names_.size()) {
                std::cout << color_code << "✗ " << step_names_[current_step_ - 1] << " - Failed" << reset << "\n";
            }
        }

        bool is_complete() const { return current_step_ >= step_names_.size(); }

        size_t get_current_step() const { return current_step_; }

        size_t get_total_steps() const { return step_names_.size(); }
    };

} // namespace echo
