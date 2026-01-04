// Demonstration of progress bars, spinners, and step indicators
#define LOGLEVEL Trace
#include <echo/widget.hpp>

#include <chrono>
#include <thread>

void demo_progress_bars() {
    echo::info("=== PROGRESS BARS ===");

    // Basic progress bar
    echo::info("\n1. Basic Progress Bar:");
    echo::progress_bar bar1(100);
    bar1.set_prefix("Loading");
    bar1.set_show_percentage(true);
    for (int i = 0; i < 100; ++i) {
        bar1.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar1.finish();

    // Progress bar with time tracking
    echo::info("\n2. Progress Bar with Time Tracking:");
    echo::progress_bar bar2(50);
    bar2.set_prefix("Processing");
    bar2.set_show_percentage(true);
    bar2.set_show_elapsed(true);
    bar2.set_show_remaining(true);
    for (int i = 0; i < 50; ++i) {
        bar2.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bar2.finish();

    // Custom styled progress bar
    echo::info("\n3. Custom Styled Progress Bar:");
    echo::progress_bar bar3(75);
    bar3.set_prefix("Downloading");
    bar3.set_fill_char('#');
    bar3.set_lead_char('>');
    bar3.set_remainder_char('-');
    bar3.set_bar_width(40);
    bar3.set_show_percentage(true);
    for (int i = 0; i < 75; ++i) {
        bar3.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
    bar3.finish();

    // Using set_progress directly
    echo::info("\n4. Direct Progress Setting:");
    echo::progress_bar bar4(100);
    bar4.set_prefix("Installing");
    bar4.set_postfix("packages");
    bar4.set_show_percentage(true);
    bar4.set_progress(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    bar4.set_progress(25);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    bar4.set_progress(50);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    bar4.set_progress(75);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    bar4.set_progress(100);
    bar4.finish();
}

void demo_spinners() {
    echo::info("\n\n=== SPINNERS ===");

    // Demo each spinner style
    std::vector<std::pair<echo::spinner_style, std::string>> spinner_styles = {
        {echo::spinner_style::line, "line (classic)"},
        {echo::spinner_style::pipe, "pipe"},
        {echo::spinner_style::simple_dots, "simple_dots"},
        {echo::spinner_style::dots_scrolling, "dots_scrolling"},
        {echo::spinner_style::flip, "flip"},
        {echo::spinner_style::toggle, "toggle"},
        {echo::spinner_style::layer, "layer"},
        {echo::spinner_style::point, "point"},
        {echo::spinner_style::dqpb, "dqpb"},
        {echo::spinner_style::bouncing_bar, "bouncing_bar"},
        {echo::spinner_style::bouncing_ball, "bouncing_ball"},
        {echo::spinner_style::aesthetic, "aesthetic"},
        {echo::spinner_style::binary, "binary"},
        {echo::spinner_style::grow_vertical, "grow_vertical"},
        {echo::spinner_style::grow_horizontal, "grow_horizontal"}};

    for (const auto &[style, name] : spinner_styles) {
        echo::info("\n" + std::to_string(&style - &spinner_styles[0].first + 1) + ". Spinner: " + name);

        echo::spinner spin(style);
        spin.set_message("Processing...");

        // Run spinner for a bit
        int iterations = 20;
        for (int i = 0; i < iterations; ++i) {
            spin.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(spin.get_interval_ms()));
        }

        spin.stop("✓ Done!");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Practical spinner example
    echo::info("\n\nPractical Example - Connecting to server:");
    echo::spinner connect_spin(echo::spinner_style::dots_scrolling);
    connect_spin.set_prefix("🌐");
    connect_spin.set_message("Connecting to server...");

    for (int i = 0; i < 15; ++i) {
        connect_spin.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(connect_spin.get_interval_ms()));
    }
    connect_spin.stop("✓ Connected successfully!");
}

void demo_steps() {
    echo::info("\n\n=== STEP INDICATORS ===");

    // Finite steps
    echo::info("\n1. Finite Steps (Known Total):");
    echo::steps workflow({"Initialize", "Load Configuration", "Connect to Database", "Process Data", "Save Results"});

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    workflow.complete();

    // Infinite steps
    echo::info("\n2. Infinite Steps (Unknown Total):");
    echo::steps infinite_workflow;

    infinite_workflow.add_step("Connecting to server");
    infinite_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    infinite_workflow.complete();

    infinite_workflow.add_step("Authenticating");
    infinite_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    infinite_workflow.complete();

    infinite_workflow.add_step("Downloading file list");
    infinite_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    infinite_workflow.complete();

    infinite_workflow.add_step("Syncing data");
    infinite_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    infinite_workflow.complete();

    // Step with failure
    echo::info("\n3. Steps with Failure:");
    echo::steps error_workflow({"Start", "Validate", "Execute", "Cleanup"});

    error_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    error_workflow.complete();

    error_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    error_workflow.complete();

    error_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    error_workflow.fail(); // This step failed!
}

void demo_combined() {
    echo::info("\n\n=== COMBINED EXAMPLE ===");
    echo::info("Simulating a complete workflow:\n");

    // Step 1: Initialize
    echo::steps main_workflow({"Initialize", "Download", "Process", "Finalize"});
    main_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    main_workflow.complete();

    // Step 2: Download with progress bar
    main_workflow.next();
    echo::progress_bar download_bar(100);
    download_bar.set_prefix("  ");
    download_bar.set_postfix("files");
    download_bar.set_show_percentage(true);
    download_bar.set_show_elapsed(true);
    download_bar.set_show_remaining(true);
    for (int i = 0; i < 100; ++i) {
        download_bar.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    download_bar.finish();
    main_workflow.complete();

    // Step 3: Process with spinner
    main_workflow.next();
    echo::spinner process_spin(echo::spinner_style::aesthetic);
    process_spin.set_prefix("  ");
    process_spin.set_message("Processing data...");
    for (int i = 0; i < 30; ++i) {
        process_spin.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(process_spin.get_interval_ms()));
    }
    process_spin.stop("  ✓ Processing complete!");
    main_workflow.complete();

    // Step 4: Finalize
    main_workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    main_workflow.complete();

    echo::info("\n✓ Workflow completed successfully!");
}

int main() {
    echo::detail::hide_cursor();

    echo::banner("WAIT.HPP DEMO");
    echo::info("Demonstrating progress bars, spinners, and step indicators\n");

    demo_progress_bars();
    demo_spinners();
    demo_steps();
    demo_combined();

    echo::separator();
    echo::banner("DEMO COMPLETE");

    echo::detail::show_cursor();
    return 0;
}
