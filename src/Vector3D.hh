#ifndef VECTOR3D_HH
#define VECTOR3D_HH

#include <cmath>
#include <ostream>

class Point3D;

/**
 * @brief Represents a vector in 3D space.
 *
 * A vector has direction and magnitude, unlike a point which represents a location.
 */
class Vector3D {
  public:
    double x;
    double y;
    double z;

    // Constructors
    constexpr Vector3D() noexcept : x(0.0), y(0.0), z(0.0) {}
    constexpr Vector3D(double x, double y, double z) noexcept : x(x), y(y), z(z) {}

    // Create vector from two points (from p1 to p2)
    Vector3D(const Point3D &from, const Point3D &to);

    // Copy and move constructors/assignments (defaulted)
    Vector3D(const Vector3D &) = default;
    Vector3D(Vector3D &&) noexcept = default;
    Vector3D &operator=(const Vector3D &) = default;
    Vector3D &operator=(Vector3D &&) noexcept = default;
    ~Vector3D() = default;

    // Vector arithmetic
    Vector3D operator+(const Vector3D &other) const;
    Vector3D operator-(const Vector3D &other) const;
    Vector3D operator-() const; // Negation
    Vector3D &operator+=(const Vector3D &other);
    Vector3D &operator-=(const Vector3D &other);

    // Scalar operations
    Vector3D operator*(double scalar) const;
    Vector3D operator/(double scalar) const;
    Vector3D &operator*=(double scalar);
    Vector3D &operator/=(double scalar);
    friend Vector3D operator*(double scalar, const Vector3D &v);

    // Comparison operators
    bool operator==(const Vector3D &other) const;
    bool operator!=(const Vector3D &other) const;

    // Vector operations
    [[nodiscard]] double dot(const Vector3D &other) const;
    [[nodiscard]] Vector3D cross(const Vector3D &other) const;

    // Length/magnitude
    [[nodiscard]] double length() const;
    [[nodiscard]] double lengthSquared() const;

    // Normalization
    [[nodiscard]] Vector3D normalized() const;
    void normalize();
    [[nodiscard]] bool isNormalized(double tolerance = 1e-10) const;
    [[nodiscard]] bool isZero(double tolerance = 1e-10) const;

    // Angle operations
    [[nodiscard]] double angleTo(const Vector3D &other) const;

    // Projection
    [[nodiscard]] Vector3D projectOnto(const Vector3D &other) const;

    // Static factory methods for common vectors
    static constexpr Vector3D zero() noexcept { return Vector3D(0.0, 0.0, 0.0); }
    static constexpr Vector3D unitX() noexcept { return Vector3D(1.0, 0.0, 0.0); }
    static constexpr Vector3D unitY() noexcept { return Vector3D(0.0, 1.0, 0.0); }
    static constexpr Vector3D unitZ() noexcept { return Vector3D(0.0, 0.0, 1.0); }

    // Stream output
    friend std::ostream &operator<<(std::ostream &os, const Vector3D &v);
};

#endif // VECTOR3D_HH
