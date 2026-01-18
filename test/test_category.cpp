/**
 * @file test_category.cpp
 * @brief Test category-based filtering
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <iostream>
#include <sstream>
#include <vector>

// Custom test sink that captures messages with categories
class CategoryTestSink : public echo::Sink {
  private:
    std::vector<std::string> messages_;
    mutable std::mutex mutex_;

  public:
    CategoryTestSink() = default;

    void write(echo::Level level, const std::string &message) override {
        if (!should_log(level)) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.push_back(message);
    }

    void flush() override {
        // No-op for test sink
    }

    [[nodiscard]] size_t message_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_.size();
    }

    [[nodiscard]] std::string get_message(size_t index) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (index < messages_.size()) {
            return messages_[index];
        }
        return "";
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.clear();
    }
};

TEST_CASE("Category filtering") {
    SUBCASE("Basic category logging") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Log with categories
        echo::category("network").info("Network message");
        echo::category("database").warn("Database warning");
        echo::category("app").error("App error");

        // All messages should be logged (no filtering yet)
        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Category level filtering") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Clear any previous category levels
        echo::clear_category_levels();

        // Set category level: only warnings and above for "network"
        echo::set_category_level("network", echo::Level::Warn);

        // These should be filtered out
        echo::category("network").trace("Network trace");
        echo::category("network").debug("Network debug");
        echo::category("network").info("Network info");

        // These should pass
        echo::category("network").warn("Network warning");
        echo::category("network").error("Network error");

        // Other categories should use global level (all pass)
        echo::category("database").info("Database info");

        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Hierarchical categories") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Clear any previous category levels
        echo::clear_category_levels();

        // Set level for parent category
        echo::set_category_level("app.*", echo::Level::Warn);

        // Child categories should inherit parent level
        echo::category("app.network").info("Should be filtered");
        echo::category("app.database").debug("Should be filtered");
        echo::category("app.network").warn("Should pass");
        echo::category("app.database").error("Should pass");

        // Non-matching categories should use global level
        echo::category("system").info("Should pass");

        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Wildcard filtering") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Clear any previous category levels
        echo::clear_category_levels();

        // Set wildcard pattern
        echo::set_category_level("app.*", echo::Level::Error);

        // Only errors should pass for app.* categories
        echo::category("app.network").info("Filtered");
        echo::category("app.network").warn("Filtered");
        echo::category("app.network").error("Should pass");

        echo::category("app.database.mysql").info("Filtered");
        echo::category("app.database.mysql").error("Should pass");

        // Non-matching categories use global level
        echo::category("system").info("Should pass");

        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Specific category level") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Clear any previous category levels
        echo::clear_category_levels();

        // Set specific category level
        echo::set_category_level("app.network", echo::Level::Warn);

        // app.network should use Warn level - debug/info filtered
        echo::category("app.network").debug("Filtered");
        echo::category("app.network").info("Filtered");
        echo::category("app.network").warn("Should pass");
        echo::category("app.network").error("Should pass");

        // Other categories use global level
        echo::category("other").info("Should pass");

        CHECK(sink->message_count() == 3);
    }

    SUBCASE("Category with colors") {
        // Clear all sinks and add test sink
        echo::clear_sinks();
        auto sink = std::make_shared<CategoryTestSink>();
        echo::add_sink(sink);

        // Clear any previous category levels
        echo::clear_category_levels();

        // Log with colors
        echo::category("network").info("Colored message").red();
        echo::category("database").warn("Another colored message").green().bold();

        CHECK(sink->message_count() == 2);
    }

    SUBCASE("get_categories") {
        // Clear any previous category levels
        echo::clear_category_levels();

        // Set some category levels
        echo::set_category_level("network", echo::Level::Warn);
        echo::set_category_level("database", echo::Level::Error);
        echo::set_category_level("app.*", echo::Level::Debug);

        // Get all categories
        auto categories = echo::get_categories();

        CHECK(categories.size() == 3);
    }

    SUBCASE("category_proxy name") {
        auto cat_proxy = echo::category("test.category");
        CHECK(cat_proxy.name() == "test.category");
    }
}
