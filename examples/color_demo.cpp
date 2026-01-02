// Demonstration of HEX color support in Echo
#define LOGLEVEL Trace
#include <echo/banner.hpp>
#include <echo/wait.hpp>

#include <chrono>
#include <thread>

int main() {
    echo::detail::hide_cursor();

    echo::banner("COLOR DEMO", echo::BoxStyle::Heavy, "#FFD700"); // Gold
    echo::info("Demonstrating HEX color support in Echo library\n");

    // =================================================================================================
    // SEPARATORS WITH COLORS
    // =================================================================================================
    echo::separator("SEPARATORS", '=', "#00BFFF"); // Deep Sky Blue

    echo::info("\n1. Single Color Separator:");
    echo::separator("Red Separator", '-', "#FF0000");

    echo::info("\n2. Gradient Separator (Red to Blue):");
    std::vector<std::string> red_blue = {"#FF0000", "#0000FF"};
    echo::separator("Gradient", '-', red_blue);

    echo::info("\n3. Rainbow Gradient:");
    std::vector<std::string> rainbow = {"#FF0000", "#FF7F00", "#FFFF00", "#00FF00", "#0000FF", "#4B0082", "#9400D3"};
    echo::separator("Rainbow", '=', rainbow);

    // =================================================================================================
    // BOXES WITH COLORS
    // =================================================================================================
    echo::separator("\nBOXES", '=', "#00BFFF");

    echo::info("\n1. Single Color Boxes:");
    echo::box("Cyan Box", echo::BoxStyle::Double, "#00FFFF");
    echo::box("Magenta Box", echo::BoxStyle::Rounded, "#FF00FF");
    echo::box("Yellow Box", echo::BoxStyle::Heavy, "#FFFF00");

    echo::info("\n2. Gradient Boxes:");
    std::vector<std::string> fire = {"#FF0000", "#FF7F00", "#FFFF00"};
    echo::box("Fire Gradient", echo::BoxStyle::Double, fire);
    std::vector<std::string> ocean = {"#000080", "#0000FF", "#00FFFF"};
    echo::box("Ocean Gradient", echo::BoxStyle::Rounded, ocean);
    std::vector<std::string> sunset = {"#FF6B6B", "#FFA500", "#FFD700"};
    echo::box("Sunset Gradient", echo::BoxStyle::Heavy, sunset);

    // =================================================================================================
    // HEADERS AND TITLES WITH COLORS
    // =================================================================================================
    echo::separator("\nHEADERS & TITLES", '=', "#00BFFF");

    echo::info("\n1. Colored Header:");
    echo::header("Success Message", "#00FF00"); // Green

    echo::info("\n2. Colored Title:");
    echo::title("Warning Section", '=', "#FFA500"); // Orange

    // =================================================================================================
    // BANNERS WITH COLORS
    // =================================================================================================
    echo::separator("\nBANNERS", '=', "#00BFFF");

    echo::info("\n1. Colored Banners:");
    echo::banner("SUCCESS", echo::BoxStyle::Heavy, "#00FF00");  // Green
    echo::banner("WARNING", echo::BoxStyle::Dashed, "#FFA500"); // Orange
    echo::banner("ERROR", echo::BoxStyle::Double, "#FF0000");   // Red

    // =================================================================================================
    // PROGRESS BARS WITH COLORS
    // =================================================================================================
    echo::separator("\nPROGRESS BARS", '=', "#00BFFF");

    echo::info("\n1. Single Color Progress Bar (Cyan):");
    echo::progress_bar bar1(50);
    bar1.set_prefix("Loading");
    bar1.set_color("#00FFFF");
    bar1.set_show_percentage(true);
    for (int i = 0; i < 50; ++i) {
        bar1.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar1.finish();

    echo::info("\n2. Gradient Progress Bar (Green to Red):");
    echo::progress_bar bar2(50);
    bar2.set_prefix("Processing");
    std::vector<std::string> green_red = {"#00FF00", "#FFFF00", "#FF0000"};
    bar2.set_gradient(green_red); // Green -> Yellow -> Red
    bar2.set_show_percentage(true);
    for (int i = 0; i < 50; ++i) {
        bar2.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar2.finish();

    echo::info("\n3. Rainbow Gradient Progress Bar:");
    echo::progress_bar bar3(50);
    bar3.set_prefix("Rainbow");
    bar3.set_gradient(rainbow);
    bar3.set_show_percentage(true);
    for (int i = 0; i < 50; ++i) {
        bar3.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    bar3.finish();

    // =================================================================================================
    // SPINNERS WITH COLORS
    // =================================================================================================
    echo::separator("\nSPINNERS", '=', "#00BFFF");

    echo::info("\n1. Single Color Spinner (Magenta):");
    echo::spinner spin1(echo::spinner_style::aesthetic);
    spin1.set_color("#FF00FF");
    spin1.set_message("Processing...");
    for (int i = 0; i < 20; ++i) {
        spin1.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(spin1.get_interval_ms()));
    }
    spin1.stop("✓ Done!");

    echo::info("\n2. Gradient Spinner (Cycling through colors):");
    echo::spinner spin2(echo::spinner_style::bouncing_ball);
    std::vector<std::string> rgb = {"#FF0000", "#00FF00", "#0000FF"};
    spin2.set_gradient(rgb);
    spin2.set_message("Loading...");
    for (int i = 0; i < 30; ++i) {
        spin2.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(spin2.get_interval_ms()));
    }
    spin2.stop("✓ Complete!");

    // =================================================================================================
    // STEPS WITH COLORS
    // =================================================================================================
    echo::separator("\nSTEPS", '=', "#00BFFF");

    echo::info("\n1. Colored Steps:");
    echo::steps workflow({"Initialize", "Load Data", "Process", "Save Results"});
    workflow.set_color("#00FF00"); // Green

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    workflow.complete();

    workflow.next();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    workflow.complete();

    // =================================================================================================
    // FINALE
    // =================================================================================================
    echo::separator();
    echo::banner("DEMO COMPLETE", echo::BoxStyle::Heavy, "#FFD700"); // Gold

    echo::detail::show_cursor();
    return 0;
}
