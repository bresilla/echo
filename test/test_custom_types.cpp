/**
 * @file test_custom_types.cpp
 * @brief Tests for custom type formatting with echo
 *
 * This test ensures that echo can handle various custom types with different
 * conversion methods: .pretty(), .print(), .to_string(), and operator<<
 */

#include <doctest/doctest.h>

#include <echo/echo.hpp>
#include <sstream>

// Test struct with to_string()
struct TypeWithToString {
    int value;
    std::string to_string() const { return "TypeWithToString(" + std::to_string(value) + ")"; }
};

// Test struct with print()
struct TypeWithPrint {
    int value;
    std::string print() const { return "TypeWithPrint(" + std::to_string(value) + ")"; }
};

// Test struct with pretty()
struct TypeWithPretty {
    int value;
    std::string pretty() const { return "TypeWithPretty(" + std::to_string(value) + ")"; }
};

// Test struct with operator<<
struct TypeWithStreamOperator {
    int value;
    friend std::ostream &operator<<(std::ostream &os, const TypeWithStreamOperator &obj) {
        return os << "TypeWithStreamOperator(" << obj.value << ")";
    }
};

// Test struct with implicit conversion to std::string
struct TypeWithStringConversion {
    int value;
    operator std::string() const { return "TypeWithStringConversion(" + std::to_string(value) + ")"; }
};

// Test struct with std::string_view conversion (like dp::String)
struct TypeWithStringViewConversion {
    std::string data_;
    TypeWithStringViewConversion(const char *s) : data_(s) {}
    operator std::string_view() const { return data_; }
    const char *c_str() const { return data_.c_str(); }

    // Add operator<< like dp::String
    friend std::ostream &operator<<(std::ostream &os, const TypeWithStringViewConversion &obj) {
        return os << std::string_view(obj.data_);
    }
};

TEST_CASE("Custom types with different conversion methods") {
    // Redirect output to capture it
    std::ostringstream oss;
    std::streambuf *old_cout = std::cout.rdbuf(oss.rdbuf());

    SUBCASE("Type with to_string()") {
        TypeWithToString obj{42};
        echo::info("Value: ", obj);
        std::string output = oss.str();
        CHECK(output.find("TypeWithToString(42)") != std::string::npos);
    }

    SUBCASE("Type with print()") {
        oss.str("");
        TypeWithPrint obj{42};
        echo::info("Value: ", obj);
        std::string output = oss.str();
        CHECK(output.find("TypeWithPrint(42)") != std::string::npos);
    }

    SUBCASE("Type with pretty()") {
        oss.str("");
        TypeWithPretty obj{42};
        echo::info("Value: ", obj);
        std::string output = oss.str();
        CHECK(output.find("TypeWithPretty(42)") != std::string::npos);
    }

    SUBCASE("Type with operator<<") {
        oss.str("");
        TypeWithStreamOperator obj{42};
        echo::info("Value: ", obj);
        std::string output = oss.str();
        CHECK(output.find("TypeWithStreamOperator(42)") != std::string::npos);
    }

    SUBCASE("Type with string_view conversion (like dp::String)") {
        oss.str("");
        TypeWithStringViewConversion obj("Hello from custom string");
        echo::info("Value: ", obj);
        std::string output = oss.str();
        CHECK(output.find("Hello from custom string") != std::string::npos);
    }

    SUBCASE("Mixed types") {
        oss.str("");
        TypeWithToString t1{10};
        TypeWithPrint t2{20};
        TypeWithPretty t3{30};
        echo::info("Values: ", t1, " ", t2, " ", t3);
        std::string output = oss.str();
        CHECK(output.find("TypeWithToString(10)") != std::string::npos);
        CHECK(output.find("TypeWithPrint(20)") != std::string::npos);
        CHECK(output.find("TypeWithPretty(30)") != std::string::npos);
    }

    // Restore cout
    std::cout.rdbuf(old_cout);
}

// Type with multiple conversion methods - pretty() should take priority
struct TypeWithAll {
    std::string pretty() const { return "pretty"; }
    std::string print() const { return "print"; }
    std::string to_string() const { return "to_string"; }
    friend std::ostream &operator<<(std::ostream &os, const TypeWithAll &) { return os << "operator<<"; }
};

// Type with print() and to_string() - print() should take priority
struct TypeWithPrintAndToString {
    std::string print() const { return "print"; }
    std::string to_string() const { return "to_string"; }
};

TEST_CASE("Priority order of conversion methods") {
    std::ostringstream oss;
    std::streambuf *old_cout = std::cout.rdbuf(oss.rdbuf());

    SUBCASE("pretty() has highest priority") {
        TypeWithAll obj;
        echo::info(obj);
        std::string output = oss.str();
        CHECK(output.find("pretty") != std::string::npos);
        CHECK(output.find("print") == std::string::npos);
        CHECK(output.find("to_string") == std::string::npos);
        CHECK(output.find("operator<<") == std::string::npos);
    }

    SUBCASE("print() has priority over to_string()") {
        oss.str("");
        TypeWithPrintAndToString obj;
        echo::info(obj);
        std::string output = oss.str();
        CHECK(output.find("print") != std::string::npos);
        CHECK(output.find("to_string") == std::string::npos);
    }

    std::cout.rdbuf(old_cout);
}

TEST_CASE("Standard types still work") {
    std::ostringstream oss;
    std::streambuf *old_cout = std::cout.rdbuf(oss.rdbuf());

    SUBCASE("Integers") {
        echo::info("Value: ", 42);
        std::string output = oss.str();
        CHECK(output.find("42") != std::string::npos);
    }

    SUBCASE("Floating point") {
        oss.str("");
        echo::info("Value: ", 3.14);
        std::string output = oss.str();
        CHECK(output.find("3.14") != std::string::npos);
    }

    SUBCASE("Booleans") {
        oss.str("");
        echo::info("Value: ", true);
        std::string output = oss.str();
        // Note: C++17 prints "1", C++20 with std::format prints "true"
        CHECK((output.find("true") != std::string::npos || output.find("1") != std::string::npos));
    }

    SUBCASE("C-strings") {
        oss.str("");
        echo::info("Value: ", "hello");
        std::string output = oss.str();
        CHECK(output.find("hello") != std::string::npos);
    }

    SUBCASE("std::string") {
        oss.str("");
        std::string str = "world";
        echo::info("Value: ", str);
        std::string output = oss.str();
        CHECK(output.find("world") != std::string::npos);
    }

    std::cout.rdbuf(old_cout);
}
