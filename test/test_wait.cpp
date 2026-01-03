#include <doctest/doctest.h>

#define LOGLEVEL Trace
#include <echo/wait.hpp>

#include <sstream>
#include <string>
#include <thread>

// Helper to capture output
struct OutputCapture {
    std::streambuf *old_cout;
    std::ostringstream cout_stream;

    OutputCapture() { old_cout = std::cout.rdbuf(cout_stream.rdbuf()); }

    ~OutputCapture() { std::cout.rdbuf(old_cout); }

    std::string get() { return cout_stream.str(); }
};

// =================================================================================================
// Progress Bar Tests
// =================================================================================================

TEST_CASE("progress_bar creates basic progress bar") {
    OutputCapture capture;
    echo::progress_bar bar(100);
    bar.set_progress(0);
    bar.display();
    std::string output = capture.get();

    // Should contain progress bar elements
    CHECK(output.find("[") != std::string::npos);
    CHECK(output.find("]") != std::string::npos);
    CHECK(output.find("0%") != std::string::npos);
}

TEST_CASE("progress_bar tick increments progress") {
    OutputCapture capture;
    echo::progress_bar bar(10);
    bar.set_show_percentage(true);

    bar.tick();
    bar.display();
    std::string output = capture.get();

    // After 1 tick out of 10, should be 10%
    CHECK(output.find("10%") != std::string::npos);
}

TEST_CASE("progress_bar set_progress sets exact value") {
    OutputCapture capture;
    echo::progress_bar bar(100);
    bar.set_show_percentage(true);
    bar.set_progress(50);
    std::string output = capture.get();

    CHECK(output.find("50%") != std::string::npos);
}

TEST_CASE("progress_bar shows prefix and postfix") {
    OutputCapture capture;
    echo::progress_bar bar(100);
    bar.set_prefix("Loading");
    bar.set_postfix("files");
    bar.set_progress(25);
    std::string output = capture.get();

    CHECK(output.find("Loading") != std::string::npos);
    CHECK(output.find("files") != std::string::npos);
}

TEST_CASE("progress_bar custom characters") {
    OutputCapture capture;
    echo::progress_bar bar(100);
    bar.set_fill_char('#');
    bar.set_lead_char('>');
    bar.set_remainder_char('-');
    bar.set_progress(50);
    std::string output = capture.get();

    // Should contain custom characters
    CHECK(output.find("#") != std::string::npos);
    CHECK(output.find(">") != std::string::npos);
    CHECK(output.find("-") != std::string::npos);
}

TEST_CASE("progress_bar is_completed returns correct state") {
    echo::progress_bar bar(100);

    CHECK_FALSE(bar.is_completed());

    bar.set_progress(50);
    CHECK_FALSE(bar.is_completed());

    bar.set_progress(100);
    CHECK(bar.is_completed());
}

TEST_CASE("progress_bar get_progress returns current value") {
    echo::progress_bar bar(100);

    CHECK(bar.get_progress() == 0);

    bar.set_progress(25);
    CHECK(bar.get_progress() == 25);

    bar.set_progress(75);
    CHECK(bar.get_progress() == 75);
}

TEST_CASE("progress_bar with time tracking") {
    OutputCapture capture;
    echo::progress_bar bar(100);
    bar.set_show_elapsed(true);
    bar.set_show_remaining(true);
    bar.set_progress(50);
    std::string output = capture.get();

    // Should contain time brackets
    CHECK(output.find("[") != std::string::npos);
    CHECK(output.find("s") != std::string::npos); // seconds
}

// =================================================================================================
// Spinner Tests
// =================================================================================================

TEST_CASE("spinner creates with default style") {
    echo::spinner spin;
    CHECK(spin.is_running());
}

TEST_CASE("spinner line style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::line);
    spin.tick();
    std::string output = capture.get();

    // Line style uses: - \ | /
    // First frame should be one of these
    bool has_line_char = (output.find("-") != std::string::npos) || (output.find("\\") != std::string::npos) ||
                         (output.find("|") != std::string::npos) || (output.find("/") != std::string::npos);
    CHECK(has_line_char);
}

TEST_CASE("spinner pipe style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::pipe);
    spin.tick();
    std::string output = capture.get();

    // Pipe style uses box drawing characters
    CHECK(output.length() > 0);
}

TEST_CASE("spinner simple_dots style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::simple_dots);
    spin.tick();
    std::string output = capture.get();

    // Simple dots uses: . .. ...
    CHECK(output.find(".") != std::string::npos);
}

TEST_CASE("spinner dots_scrolling style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::dots_scrolling);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner flip style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::flip);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner toggle style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::toggle);
    spin.tick();
    std::string output = capture.get();

    // Toggle uses: = * -
    bool has_toggle = (output.find("=") != std::string::npos) || (output.find("*") != std::string::npos) ||
                      (output.find("-") != std::string::npos);
    CHECK(has_toggle);
}

TEST_CASE("spinner layer style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::layer);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner point style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::point);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner dqpb style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::dqpb);
    spin.tick();
    std::string output = capture.get();

    // dqpb uses letters: d q p b
    bool has_letter = (output.find("d") != std::string::npos) || (output.find("q") != std::string::npos) ||
                      (output.find("p") != std::string::npos) || (output.find("b") != std::string::npos);
    CHECK(has_letter);
}

TEST_CASE("spinner bouncing_bar style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::bouncing_bar);
    spin.tick();
    std::string output = capture.get();

    // Bouncing bar uses brackets
    CHECK(output.find("[") != std::string::npos);
    CHECK(output.find("]") != std::string::npos);
}

TEST_CASE("spinner bouncing_ball style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::bouncing_ball);
    spin.tick();
    std::string output = capture.get();

    // Bouncing ball uses parentheses
    CHECK(output.find("(") != std::string::npos);
    CHECK(output.find(")") != std::string::npos);
}

TEST_CASE("spinner aesthetic style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::aesthetic);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner binary style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::binary);
    spin.tick();
    std::string output = capture.get();

    // Binary uses 0 and 1
    bool has_binary = (output.find("0") != std::string::npos) || (output.find("1") != std::string::npos);
    CHECK(has_binary);
}

TEST_CASE("spinner grow_vertical style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::grow_vertical);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner grow_horizontal style") {
    OutputCapture capture;
    echo::spinner spin(echo::spinner_style::grow_horizontal);
    spin.tick();
    std::string output = capture.get();

    CHECK(output.length() > 0);
}

TEST_CASE("spinner with message") {
    OutputCapture capture;
    echo::spinner spin;
    spin.set_message("Processing...");
    spin.tick();
    std::string output = capture.get();

    CHECK(output.find("Processing...") != std::string::npos);
}

TEST_CASE("spinner with prefix") {
    OutputCapture capture;
    echo::spinner spin;
    spin.set_prefix("🌐");
    spin.tick();
    std::string output = capture.get();

    CHECK(output.find("🌐") != std::string::npos);
}

TEST_CASE("spinner stop") {
    OutputCapture capture;
    echo::spinner spin;
    spin.stop("Done!");
    std::string output = capture.get();

    CHECK(output.find("Done!") != std::string::npos);
    CHECK_FALSE(spin.is_running());
}

TEST_CASE("spinner get_interval_ms returns valid interval") {
    echo::spinner spin(echo::spinner_style::line);
    int interval = spin.get_interval_ms();

    CHECK(interval > 0);
    CHECK(interval <= 500); // Reasonable interval
}

// =================================================================================================
// Steps Tests
// =================================================================================================

TEST_CASE("steps finite workflow") {
    OutputCapture capture;
    echo::steps workflow({"Init", "Load", "Process"});

    workflow.next();
    std::string output1 = capture.get();
    CHECK(output1.find("Step 1/3") != std::string::npos);
    CHECK(output1.find("Init") != std::string::npos);

    workflow.complete();
    std::string output2 = capture.get();
    CHECK(output2.find("✓") != std::string::npos);
    CHECK(output2.find("Init") != std::string::npos);
}

TEST_CASE("steps infinite workflow") {
    OutputCapture capture;
    echo::steps workflow;

    workflow.add_step("First");
    workflow.next();
    std::string output1 = capture.get();
    CHECK(output1.find("Step 1") != std::string::npos);
    CHECK(output1.find("First") != std::string::npos);
    // Should NOT have "/X" format for infinite
    CHECK(output1.find("/") == std::string::npos);

    workflow.complete();
    workflow.add_step("Second");
    workflow.next();
    std::string output2 = capture.get();
    CHECK(output2.find("Step 2") != std::string::npos);
}

TEST_CASE("steps complete marks step as done") {
    OutputCapture capture;
    echo::steps workflow({"Task1", "Task2"});

    workflow.next();
    workflow.complete();
    std::string output = capture.get();

    CHECK(output.find("✓") != std::string::npos);
    CHECK(output.find("Complete") != std::string::npos);
}

TEST_CASE("steps fail marks step as failed") {
    OutputCapture capture;
    echo::steps workflow({"Task1", "Task2"});

    workflow.next();
    workflow.fail();
    std::string output = capture.get();

    CHECK(output.find("✗") != std::string::npos);
    CHECK(output.find("Failed") != std::string::npos);
}

TEST_CASE("steps is_complete returns correct state") {
    echo::steps workflow({"Task1", "Task2"});

    CHECK_FALSE(workflow.is_complete());

    workflow.next();
    workflow.complete();
    CHECK_FALSE(workflow.is_complete());

    workflow.next();
    workflow.complete();
    CHECK(workflow.is_complete());
}

TEST_CASE("steps get_current_step returns correct value") {
    echo::steps workflow({"Task1", "Task2", "Task3"});

    CHECK(workflow.get_current_step() == 0);

    workflow.next();
    CHECK(workflow.get_current_step() == 1);

    workflow.next();
    CHECK(workflow.get_current_step() == 2);
}

TEST_CASE("steps get_total_steps returns correct count") {
    echo::steps workflow({"Task1", "Task2", "Task3"});

    CHECK(workflow.get_total_steps() == 3);
}

TEST_CASE("steps infinite can add steps dynamically") {
    echo::steps workflow;

    CHECK(workflow.get_total_steps() == 0);

    workflow.add_step("First");
    CHECK(workflow.get_total_steps() == 1);

    workflow.add_step("Second");
    CHECK(workflow.get_total_steps() == 2);

    workflow.add_step("Third");
    CHECK(workflow.get_total_steps() == 3);
}

// =================================================================================================
// Cursor Control Tests
// =================================================================================================

TEST_CASE("cursor control functions exist and don't crash") {
    // These functions output ANSI codes, just verify they don't crash
    CHECK_NOTHROW(echo::detail::hide_cursor());
    CHECK_NOTHROW(echo::detail::show_cursor());
    CHECK_NOTHROW(echo::detail::clear_line());
    CHECK_NOTHROW(echo::detail::move_cursor_up(1));
}
