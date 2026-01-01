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

    std::string pretty() const {
        return "Vector3 { x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", z: " + std::to_string(z) + " }";
    }
};

struct Color {
    int r, g, b;

    std::string to_string() const {
        return "rgb(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
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
    echo::info("Vector3 with pretty() (preferred over print): ", v);

    Color c{255, 128, 0};
    echo::info("Color with to_string(): ", c);

    echo::debug("Current log level: ", static_cast<int>(echo::current_level()));

    return 0;
}
