#ifndef POINT3D_HH
#define POINT3D_HH

#include <cmath>
#include <ostream>

class Vector3D;

/**
 * @brief Represents a point in 3D space.
 */
class Point3D
{
public:
    double x;
    double y;
    double z;

    // Constructors
    constexpr Point3D() noexcept : x(0.0), y(0.0), z(0.0) {}
    constexpr Point3D(double x, double y, double z) noexcept : x(x), y(y), z(z) {}

    // Copy and move constructors/assignments (defaulted)
    Point3D(const Point3D&) = default;
    Point3D(Point3D&&) noexcept = default;
    Point3D& operator=(const Point3D&) = default;
    Point3D& operator=(Point3D&&) noexcept = default;
    ~Point3D() = default;

    // Arithmetic operations with vectors
    Point3D operator+(const Vector3D& v) const;
    Point3D operator-(const Vector3D& v) const;
    Point3D& operator+=(const Vector3D& v);
    Point3D& operator-=(const Vector3D& v);

    // Difference between two points yields a vector
    Vector3D operator-(const Point3D& other) const;

    // Comparison operators
    bool operator==(const Point3D& other) const;
    bool operator!=(const Point3D& other) const;

    // Utility methods
    [[nodiscard]] double distanceTo(const Point3D& other) const;
    [[nodiscard]] double distanceSquaredTo(const Point3D& other) const;

    // Static factory methods
    static constexpr Point3D origin() noexcept { return Point3D(0.0, 0.0, 0.0); }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const Point3D& p);
};

#endif // POINT3D_HH
