/**
 * @file test_category.cpp
 * @brief Test category-based filtering
 */

#include <echo/echo.hpp>

#include <cassert>
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

void test_basic_category() {
    std::cout << "Testing basic category logging...\n";

    // Clear all sinks and add test sink
    echo::clear_sinks();
    auto sink = std::make_shared<CategoryTestSink>();
    echo::add_sink(sink);

    // Log with categories
    echo::category("network").info("Network message");
    echo::category("database").warn("Database warning");
    echo::category("app").error("App error");

    // All messages should be logged (no filtering yet)
    assert(sink->message_count() == 3 && "Should have 3 messages");

    std::cout << "  Message 1: " << sink->get_message(0) << "\n";
    std::cout << "  Message 2: " << sink->get_message(1) << "\n";
    std::cout << "  Message 3: " << sink->get_message(2) << "\n";

    std::cout << "✓ Basic category logging works\n";
}

void test_category_level_filtering() {
    std::cout << "Testing category level filtering...\n";

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

    assert(sink->message_count() == 3 && "Should have 3 messages (2 network + 1 database)");

    std::cout << "  Passed messages:\n";
    for (size_t i = 0; i < sink->message_count(); ++i) {
        std::cout << "    " << sink->get_message(i) << "\n";
    }

    std::cout << "✓ Category level filtering works\n";
}

void test_hierarchical_categories() {
    std::cout << "Testing hierarchical categories...\n";

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

    assert(sink->message_count() == 3 && "Should have 3 messages");

    std::cout << "  Passed messages:\n";
    for (size_t i = 0; i < sink->message_count(); ++i) {
        std::cout << "    " << sink->get_message(i) << "\n";
    }

    std::cout << "✓ Hierarchical categories work\n";
}

void test_wildcard_filtering() {
    std::cout << "Testing wildcard filtering...\n";

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

    assert(sink->message_count() == 3 && "Should have 3 messages");

    std::cout << "  Passed messages:\n";
    for (size_t i = 0; i < sink->message_count(); ++i) {
        std::cout << "    " << sink->get_message(i) << "\n";
    }

    std::cout << "✓ Wildcard filtering works\n";
}

void test_specific_overrides_wildcard() {
    std::cout << "Testing specific category overrides wildcard...\n";

    // Clear all sinks and add test sink
    echo::clear_sinks();
    auto sink = std::make_shared<CategoryTestSink>();
    echo::add_sink(sink);

    // Clear any previous category levels
    echo::clear_category_levels();

    // Set wildcard pattern
    echo::set_category_level("app.*", echo::Level::Error);

    // Set specific category (should override wildcard)
    echo::set_category_level("app.network", echo::Level::Debug);

    // app.network should use Debug level
    echo::category("app.network").debug("Should pass");
    echo::category("app.network").info("Should pass");

    // app.database should use Error level from wildcard
    echo::category("app.database").debug("Filtered");
    echo::category("app.database").info("Filtered");
    echo::category("app.database").error("Should pass");

    assert(sink->message_count() == 3 && "Should have 3 messages");

    std::cout << "  Passed messages:\n";
    for (size_t i = 0; i < sink->message_count(); ++i) {
        std::cout << "    " << sink->get_message(i) << "\n";
    }

    std::cout << "✓ Specific category overrides wildcard\n";
}

void test_category_with_colors() {
    std::cout << "Testing category with color methods...\n";

    // Clear all sinks and add test sink
    echo::clear_sinks();
    auto sink = std::make_shared<CategoryTestSink>();
    echo::add_sink(sink);

    // Clear any previous category levels
    echo::clear_category_levels();

    // Log with colors
    echo::category("network").info("Colored message").red();
    echo::category("database").warn("Another colored message").green().bold();

    assert(sink->message_count() == 2 && "Should have 2 messages");

    std::cout << "  Message 1: " << sink->get_message(0) << "\n";
    std::cout << "  Message 2: " << sink->get_message(1) << "\n";

    std::cout << "✓ Category with colors works\n";
}

void test_get_categories() {
    std::cout << "Testing get_categories()...\n";

    // Clear any previous category levels
    echo::clear_category_levels();

    // Set some category levels
    echo::set_category_level("network", echo::Level::Warn);
    echo::set_category_level("database", echo::Level::Error);
    echo::set_category_level("app.*", echo::Level::Debug);

    // Get all categories
    auto categories = echo::get_categories();

    assert(categories.size() == 3 && "Should have 3 categories");

    std::cout << "  Registered categories:\n";
    for (const auto &cat : categories) {
        std::cout << "    " << cat << "\n";
    }

    std::cout << "✓ get_categories() works\n";
}

void test_category_proxy_name() {
    std::cout << "Testing category_proxy::name()...\n";

    auto cat_proxy = echo::category("test.category");
    assert(cat_proxy.name() == "test.category" && "Category name should match");

    std::cout << "  Category name: " << cat_proxy.name() << "\n";
    std::cout << "✓ category_proxy::name() works\n";
}

int main() {
    std::cout << "=== Echo Category Filtering Tests ===\n\n";

    try {
        test_basic_category();
        test_category_level_filtering();
        test_hierarchical_categories();
        test_wildcard_filtering();
        test_specific_overrides_wildcard();
        test_category_with_colors();
        test_get_categories();
        test_category_proxy_name();

        std::cout << "\n=== All category tests passed! ===\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
