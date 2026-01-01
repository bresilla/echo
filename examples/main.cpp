// Override log level in-file (before including echo.hpp)
// This takes precedence over -DLOGLEVEL from build system
#define LOGLEVEL Trace

#include <echo/echo.hpp>

struct Point {
    int x, y;

    std::string print() const { return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }
};

struct Vector3 {
    float x, y, z;

    std::string print() const {
        return "Vec3[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
    }

    std::string pretty_print() const {
        return "Vector3 { x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", z: " + std::to_string(z) + " }";
    }
};

int main() {
    echo::trace("This is a trace message");
    echo::debug("This is a debug message");
    echo::info("This is an info message");
    echo::warn("This is a warning message");
    echo::error("This is an error message");
    echo::critical("This is a critical message");

    echo::info("Multiple args: ", 42, " and ", 3.14, " and ", "hello");

    Point p{10, 20};
    echo::info("Point with print(): ", p);

    Vector3 v{1.0f, 2.0f, 3.0f};
    echo::info("Vector3 with pretty_print() (preferred): ", v);

    echo::debug("Current log level: ", static_cast<int>(echo::current_level()));

    return 0;
}
