/**
 * @file format_text_demo.cpp
 * @brief Demonstrates echo::format::String text formatting methods
 */

#define LOGLEVEL Trace
#include <echo/echo.hpp>
#include <echo/format.hpp>

#include <iostream>

using echo::format::String;

void demo_alignment() {
    echo::info("=== TEXT ALIGNMENT ===\n");

    std::cout << "|" << String("Left").left(20) << "|\n";
    std::cout << "|" << String("Right").right(20) << "|\n";
    std::cout << "|" << String("Center").center(20) << "|\n";
    std::cout << "|" << String("Pad Left").pad_left(20, '-') << "|\n";
    std::cout << "|" << String("Pad Right").pad_right(20, '*') << "|\n";
    std::cout << "\n";
}

void demo_transformation() {
    echo::info("=== TEXT TRANSFORMATION ===\n");

    std::cout << String("hello world").uppercase() << "\n";
    std::cout << String("HELLO WORLD").lowercase() << "\n";
    std::cout << String("hello world").capitalize() << "\n";
    std::cout << String("hello world from echo").title_case() << "\n";
    std::cout << String("Hello").reverse_text() << "\n";
    std::cout << String("Echo").repeat(3) << "\n";
    std::cout << "\n";
}

void demo_truncation() {
    echo::info("=== TRUNCATION & WRAPPING ===\n");

    std::cout << String("This is a very long text").truncate(10) << "\n";
    std::cout << String("This is a very long text").truncate(15, "...") << "\n";
    std::cout << String("This is a very long text").ellipsis(12) << "\n";
    std::cout << String("The quick brown fox jumps over the lazy dog").wrap(20) << "\n";
    std::cout << "\n";
}

void demo_borders() {
    echo::info("=== BORDERS & FRAMES ===\n");

    std::cout << String("Bordered").border() << "\n\n";
    std::cout << String("Custom Border").border('=') << "\n\n";
    std::cout << String("Boxed Text").box() << "\n\n";
    std::cout << String("Quoted").quote() << "\n";
    std::cout << String("Bracketed").brackets() << "\n";
    std::cout << String("Parenthesized").parens() << "\n";
    std::cout << "\n";
}

void demo_indentation() {
    echo::info("=== INDENTATION ===\n");

    std::cout << String("Indented 4 spaces").indent(4) << "\n";
    std::cout << String("Indented with tabs").indent(2, '\t') << "|\n";
    std::cout << String("Line 1\nLine 2\nLine 3").indent(2) << "\n";
    std::cout << String("  Dedented text  ").dedent() << "|\n";
    std::cout << "\n";
}

void demo_prefix_suffix() {
    echo::info("=== PREFIX & SUFFIX ===\n");

    std::cout << String("World").prefix("Hello ") << "\n";
    std::cout << String("Hello").suffix(" World") << "\n";
    std::cout << String("Text").surround("<", ">") << "\n";
    std::cout << String("Text").surround("[[", "]]") << "\n";
    std::cout << "\n";
}

void demo_special() {
    echo::info("=== SPECIAL FORMATTING ===\n");

    std::cout << String("code").monospace() << "\n";
    std::cout << String("Click here").link("https://example.com") << "\n";
    std::cout << String("").progress(0) << "\n";
    std::cout << String("").progress(33.3) << "\n";
    std::cout << String("").progress(66.6) << "\n";
    std::cout << String("").progress(100) << "\n";
    std::cout << String("v1.0.0").badge("version") << "\n";
    std::cout << String("passing").badge("status") << "\n";
    std::cout << "\n";
}

void demo_numeric() {
    echo::info("=== NUMERIC FORMATTING ===\n");

    std::cout << String("3.14159265").format_number(2) << "\n";
    std::cout << String("3.14159265").format_number(4) << "\n";
    std::cout << String("1024").format_bytes() << "\n";
    std::cout << String("1048576").format_bytes() << "\n";
    std::cout << String("1073741824").format_bytes() << "\n";
    std::cout << String("90").format_duration() << "\n";
    std::cout << String("3665").format_duration() << "\n";
    std::cout << String("7200").format_duration() << "\n";
    std::cout << "\n";
}

void demo_manipulation() {
    echo::info("=== STRING MANIPULATION ===\n");

    std::cout << "|" << String("  trimmed  ").trim() << "|\n";
    std::cout << "|" << String("  left trim").trim_left() << "|\n";
    std::cout << "|" << String("right trim  ").trim_right() << "|\n";
    std::cout << String("Hello World").replace("World", "Echo") << "\n";
    std::cout << String("Remove this word").remove("this ") << "\n";
    std::cout << String("0123456789").slice(2, 7) << "\n";
    std::cout << "\n";
}

void demo_inspection() {
    echo::info("=== INSPECTION METHODS ===\n");

    String text("Hello World");
    echo::info("Text: '", text.str(), "'");
    echo::info("Length: ", text.length());
    echo::info("Is empty: ", text.is_empty() ? "yes" : "no");
    echo::info("Contains 'World': ", text.contains("World") ? "yes" : "no");
    echo::info("Contains 'Foo': ", text.contains("Foo") ? "yes" : "no");

    String empty("");
    echo::info("Empty string is_empty: ", empty.is_empty() ? "yes" : "no");
    std::cout << "\n";
}

void demo_chaining() {
    echo::info("=== CHAINING FORMATTING WITH COLORS ===\n");

    std::cout << String("Formatted").center(20).red().bold() << "\n";
    std::cout << String("Styled").uppercase().green().underline() << "\n";
    std::cout << String("Complex").prefix(">>> ").suffix(" <<<").blue().on_yellow() << "\n";
    std::cout << String("Badge").badge("INFO").cyan().bold() << "\n";
    std::cout << String("Progress").progress(75).green() << "\n";
    std::cout << String("Boxed").box().magenta() << "\n\n";
    std::cout << String("All Together").center(30).uppercase().bold().white().on_blue() << "\n";
    std::cout << "\n";
}

int main() {
    echo::info("╔════════════════════════════════════════╗");
    echo::info("║   Echo Format::String Text Demo       ║");
    echo::info("╚════════════════════════════════════════╝\n");

    demo_alignment();
    demo_transformation();
    demo_truncation();
    demo_borders();
    demo_indentation();
    demo_prefix_suffix();
    demo_special();
    demo_numeric();
    demo_manipulation();
    demo_inspection();
    demo_chaining();

    echo::info("Demo complete!");
    return 0;
}
