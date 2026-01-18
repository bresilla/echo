/**
 * @file test_concurrency.cpp
 * @brief Edge case tests for concurrent logging and thread safety
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

TEST_CASE("Concurrent logging from multiple threads") {
    echo::clear_sinks();
    auto console = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console);

    SUBCASE("Many threads logging simultaneously") {
        constexpr int num_threads = 10;
        constexpr int messages_per_thread = 100;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([t]() {
                for (int i = 0; i < messages_per_thread; ++i) {
                    echo::info("Thread ", t, " message ", i);
                    echo::debug("Thread ", t, " debug ", i);
                    echo::error("Thread ", t, " error ", i);
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        // Should not crash or corrupt output
        CHECK(true);
    }

    SUBCASE("Concurrent .once() calls") {
        constexpr int num_threads = 20;
        std::atomic<int> count{0};

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&count]() {
                for (int i = 0; i < 1000; ++i) {
                    echo::info("Once message").once();
                }
                count++;
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(count == num_threads);
    }

    SUBCASE("Concurrent category logging") {
        constexpr int num_threads = 10;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([t]() {
                std::string category = "thread_" + std::to_string(t);
                for (int i = 0; i < 100; ++i) {
                    echo::category(category).info("Message ", i);
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(true);
    }

    SUBCASE("Concurrent level changes") {
        constexpr int num_threads = 5;
        std::atomic<bool> stop{false};

        // Thread that changes log level
        std::thread level_changer([&stop]() {
            while (!stop) {
                echo::set_level(echo::Level::Debug);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                echo::set_level(echo::Level::Error);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });

        // Threads that log
        std::vector<std::thread> loggers;
        loggers.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            loggers.emplace_back([&stop]() {
                while (!stop) {
                    echo::debug("Debug message");
                    echo::info("Info message");
                    echo::error("Error message");
                }
            });
        }

        // Run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop = true;

        level_changer.join();
        for (auto &logger : loggers) {
            logger.join();
        }

        CHECK(true);
    }
}

TEST_CASE("Concurrent sink operations") {
    SUBCASE("Concurrent sink addition") {
        echo::clear_sinks();
        constexpr int num_threads = 10;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([]() {
                for (int i = 0; i < 10; ++i) {
                    auto sink = std::make_shared<echo::ConsoleSink>();
                    echo::add_sink(sink);
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(true);
    }

    SUBCASE("Concurrent sink removal") {
        echo::clear_sinks();

        // Add many sinks
        for (int i = 0; i < 100; ++i) {
            auto sink = std::make_shared<echo::ConsoleSink>();
            echo::add_sink(sink);
        }

        constexpr int num_threads = 5;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([]() {
                for (int i = 0; i < 10; ++i) {
                    echo::clear_sinks();
                    auto sink = std::make_shared<echo::ConsoleSink>();
                    echo::add_sink(sink);
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(true);
    }

    SUBCASE("Logging while modifying sinks") {
        echo::clear_sinks();
        auto console = std::make_shared<echo::ConsoleSink>();
        echo::add_sink(console);

        std::atomic<bool> stop{false};

        // Thread that modifies sinks
        std::thread sink_modifier([&stop]() {
            while (!stop) {
                echo::clear_sinks();
                auto sink = std::make_shared<echo::ConsoleSink>();
                echo::add_sink(sink);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        // Threads that log
        std::vector<std::thread> loggers;
        constexpr int num_loggers = 5;
        loggers.reserve(num_loggers);

        for (int t = 0; t < num_loggers; ++t) {
            loggers.emplace_back([&stop]() {
                while (!stop) {
                    echo::info("Logging while sinks change");
                }
            });
        }

        // Run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop = true;

        sink_modifier.join();
        for (auto &logger : loggers) {
            logger.join();
        }

        CHECK(true);
    }
}

TEST_CASE("Concurrent category operations") {
    echo::clear_sinks();

    SUBCASE("Concurrent category level changes") {
        constexpr int num_threads = 10;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([t]() {
                std::string category = "cat_" + std::to_string(t);
                for (int i = 0; i < 100; ++i) {
                    echo::set_category_level(category, echo::Level::Debug);
                    echo::set_category_level(category, echo::Level::Error);
                    echo::category(category).info("Message");
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(true);
    }

    SUBCASE("Concurrent category clear") {
        std::atomic<bool> stop{false};

        // Thread that clears categories
        std::thread clearer([&stop]() {
            while (!stop) {
                echo::clear_category_levels();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        // Threads that set and use categories
        std::vector<std::thread> users;
        constexpr int num_users = 5;
        users.reserve(num_users);

        for (int t = 0; t < num_users; ++t) {
            users.emplace_back([&stop, t]() {
                std::string category = "user_" + std::to_string(t);
                while (!stop) {
                    echo::set_category_level(category, echo::Level::Debug);
                    echo::category(category).info("Message");
                }
            });
        }

        // Run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop = true;

        clearer.join();
        for (auto &user : users) {
            user.join();
        }

        CHECK(true);
    }
}

TEST_CASE("Stress test - high throughput") {
    echo::clear_sinks();
    auto console = std::make_shared<echo::ConsoleSink>();
    echo::add_sink(console);

    SUBCASE("Sustained high-rate logging") {
        constexpr int num_threads = 4;
        constexpr int duration_ms = 1000;
        std::atomic<uint64_t> message_count{0};
        std::atomic<bool> stop{false};

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&stop, &message_count]() {
                while (!stop) {
                    echo::info("High throughput message");
                    message_count++;
                }
            });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        stop = true;

        for (auto &thread : threads) {
            thread.join();
        }

        // Should have logged many messages without crashing
        CHECK(message_count > 0);
    }
}

#ifdef ECHO_ENABLE_FILE_SINK
TEST_CASE("Concurrent file sink operations") {
    SUBCASE("Multiple threads writing to same file") {
        echo::clear_sinks();
        auto file = std::make_shared<echo::FileSink>("/tmp/concurrent_test.log");
        echo::add_sink(file);

        constexpr int num_threads = 10;
        constexpr int messages_per_thread = 100;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([t]() {
                for (int i = 0; i < messages_per_thread; ++i) {
                    echo::info("Thread ", t, " message ", i);
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        CHECK(true);

        // Cleanup
        std::remove("/tmp/concurrent_test.log");
    }
}
#endif
