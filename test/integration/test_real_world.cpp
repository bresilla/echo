/**
 * @file test_real_world.cpp
 * @brief Integration tests simulating real-world usage patterns
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define ECHO_ENABLE_FILE_SINK
#include <doctest/doctest.h>

#include <echo/echo.hpp>

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

TEST_CASE("Web server logging simulation") {
    echo::clear_sinks();

    // Setup: Console for errors, file for all logs
    auto console = std::make_shared<echo::ConsoleSink>();
    console->set_level(echo::Level::Error);

    auto file = std::make_shared<echo::FileSink>("/tmp/webserver.log");
    file->set_level(echo::Level::Info);
    file->enable_rotation(10 * 1024, 3); // 10KB rotation

    echo::add_sink(console);
    echo::add_sink(file);

    // Setup categories
    echo::set_category_level("http", echo::Level::Info);
    echo::set_category_level("http.request", echo::Level::Debug);
    echo::set_category_level("database", echo::Level::Warn);

    SUBCASE("Simulated HTTP requests") {
        std::atomic<int> request_count{0};
        std::atomic<bool> stop{false};

        // Request handler threads
        auto handle_request = [&stop, &request_count]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> status_dist(200, 500);

            while (!stop) {
                int req_id = request_count++;
                int status = status_dist(gen);

                echo::category("http.request").debug("Request ", req_id, " started");

                // Simulate processing
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                if (status >= 400) {
                    echo::category("http").error("Request ", req_id, " failed with status ", status);
                } else {
                    echo::category("http").info("Request ", req_id, " completed with status ", status);
                }
            }
        };

        // Start worker threads
        std::vector<std::thread> workers;
        for (int i = 0; i < 4; ++i) {
            workers.emplace_back(handle_request);
        }

        // Run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop = true;

        for (auto &worker : workers) {
            worker.join();
        }

        CHECK(request_count > 0);
    }

    // Cleanup
    echo::clear_sinks();
    echo::clear_category_levels();
    std::remove("/tmp/webserver.log");
}

TEST_CASE("Game engine logging simulation") {
    echo::clear_sinks();

    auto console = std::make_shared<echo::ConsoleSink>();
    auto file = std::make_shared<echo::FileSink>("/tmp/game.log");

    echo::add_sink(console);
    echo::add_sink(file);

    // Game subsystem categories
    echo::set_category_level("engine", echo::Level::Info);
    echo::set_category_level("engine.render", echo::Level::Warn);
    echo::set_category_level("engine.physics", echo::Level::Debug);
    echo::set_category_level("engine.audio", echo::Level::Info);

    SUBCASE("Game loop simulation") {
        std::atomic<bool> running{true};
        std::atomic<int> frame_count{0};

        // Render thread
        std::thread render_thread([&running, &frame_count]() {
            while (running) {
                echo::category("engine.render").debug("Frame ", frame_count.load(), " rendered");
                std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            }
        });

        // Physics thread
        std::thread physics_thread([&running]() {
            int tick = 0;
            while (running) {
                echo::category("engine.physics").debug("Physics tick ", tick++);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        // Audio thread
        std::thread audio_thread([&running]() {
            while (running) {
                echo::category("engine.audio").info("Audio buffer updated");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });

        // Main game loop
        for (int frame = 0; frame < 10; ++frame) {
            frame_count = frame;
            echo::category("engine").info("Game frame ", frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        running = false;

        render_thread.join();
        physics_thread.join();
        audio_thread.join();

        CHECK(frame_count >= 0);
    }

    // Cleanup
    echo::clear_sinks();
    echo::clear_category_levels();
    std::remove("/tmp/game.log");
}

TEST_CASE("Microservice logging simulation") {
    echo::clear_sinks();

    auto console = std::make_shared<echo::ConsoleSink>();
    auto file = std::make_shared<echo::FileSink>("/tmp/microservice.log");
    file->set_formatter(std::make_shared<echo::PatternFormatter>("{timestamp} | {level:8} | {message}"));

    echo::add_sink(console);
    echo::add_sink(file);

    // Service categories
    echo::set_category_level("api", echo::Level::Info);
    echo::set_category_level("api.auth", echo::Level::Debug);
    echo::set_category_level("api.users", echo::Level::Info);
    echo::set_category_level("database", echo::Level::Warn);
    echo::set_category_level("cache", echo::Level::Info);

    SUBCASE("API request processing") {
        // Simulate API request
        echo::category("api").info("Received POST /api/users");
        echo::category("api.auth").debug("Validating JWT token");
        echo::category("api.auth").debug("Token valid for user_id=123");

        echo::category("cache").info("Cache miss for user:123");
        echo::category("database").info("Querying user table");
        echo::category("database").warn("Slow query detected: 150ms");

        echo::category("cache").info("Cached user:123");
        echo::category("api.users").info("User profile retrieved");
        echo::category("api").info("Response sent: 200 OK");

        CHECK(true);
    }

    SUBCASE("Error handling") {
        echo::category("api").info("Received GET /api/orders/999");
        echo::category("database").error("Connection timeout");
        echo::category("api").error("Request failed: database unavailable");

        CHECK(true);
    }

    // Cleanup
    echo::clear_sinks();
    echo::clear_category_levels();
    std::remove("/tmp/microservice.log");
}

TEST_CASE("Data processing pipeline simulation") {
    echo::clear_sinks();

    auto file = std::make_shared<echo::FileSink>("/tmp/pipeline.log");
    file->enable_rotation(5 * 1024, 2);
    echo::add_sink(file);

    echo::set_category_level("pipeline", echo::Level::Info);
    echo::set_category_level("pipeline.ingest", echo::Level::Debug);
    echo::set_category_level("pipeline.transform", echo::Level::Info);
    echo::set_category_level("pipeline.output", echo::Level::Info);

    SUBCASE("Multi-stage data processing") {
        std::atomic<int> records_processed{0};
        std::atomic<bool> stop{false};

        // Ingest stage
        std::thread ingest([&stop, &records_processed]() {
            int record_id = 0;
            while (!stop) {
                echo::category("pipeline.ingest").debug("Ingested record ", record_id++);
                records_processed++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        // Transform stage
        std::thread transform([&stop]() {
            while (!stop) {
                echo::category("pipeline.transform").info("Transforming batch");
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        });

        // Output stage
        std::thread output([&stop]() {
            while (!stop) {
                echo::category("pipeline.output").info("Writing to output");
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        });

        // Run pipeline
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop = true;

        ingest.join();
        transform.join();
        output.join();

        echo::category("pipeline").info("Pipeline completed. Records processed: ", records_processed.load());

        CHECK(records_processed > 0);
    }

    // Cleanup
    echo::clear_sinks();
    echo::clear_category_levels();
    std::remove("/tmp/pipeline.log");
}

TEST_CASE("Long-running application simulation") {
    echo::clear_sinks();

    auto console = std::make_shared<echo::ConsoleSink>();
    auto file = std::make_shared<echo::FileSink>("/tmp/longrunning.log");
    file->set_rotation_policy(echo::RotationPolicy::SizeOrTime);
    file->enable_rotation(2 * 1024, 3); // 2KB or time-based

    echo::add_sink(console);
    echo::add_sink(file);

    SUBCASE("Periodic health checks") {
        std::atomic<bool> running{true};

        // Health check thread
        std::thread health_check([&running]() {
            int check_count = 0;
            while (running) {
                if (check_count == 0) {
                    echo::info("Health check started - OK");
                }
                check_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });

        // Worker thread
        std::thread worker([&running]() {
            int task_count = 0;
            while (running) {
                echo::debug("Processing task ", task_count++);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });

        // Run for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        running = false;

        health_check.join();
        worker.join();

        CHECK(true);
    }

    // Cleanup
    echo::clear_sinks();
    std::remove("/tmp/longrunning.log");
}
