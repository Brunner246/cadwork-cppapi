#include "Vector3D.hh"
#include "Point3D.hh"

#include <stdexcept>

Vector3D::Vector3D(const Point3D& from, const Point3D& to)
    : x(to.x - from.x), y(to.y - from.y), z(to.z - from.z)
{
}

Vector3D Vector3D::operator+(const Vector3D& other) const
{
    return Vector3D(x + other.x, y + other.y, z + other.z);
}

Vector3D Vector3D::operator-(const Vector3D& other) const
{
    return Vector3D(x - other.x, y - other.y, z - other.z);
}

Vector3D Vector3D::operator-() const
{
    return Vector3D(-x, -y, -z);
}

Vector3D& Vector3D::operator+=(const Vector3D& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3D Vector3D::operator*(double scalar) const
{
    return Vector3D(x * scalar, y * scalar, z * scalar);
}

Vector3D Vector3D::operator/(double scalar) const
{
    if (std::abs(scalar) < 1e-15)
    {
        throw std::invalid_argument("Division by zero in Vector3D");
    }
    return Vector3D(x / scalar, y / scalar, z / scalar);
}

Vector3D& Vector3D::operator*=(double scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3D& Vector3D::operator/=(double scalar)
{
    if (std::abs(scalar) < 1e-15)
    {
        throw std::invalid_argument("Division by zero in Vector3D");
    }
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

Vector3D operator*(double scalar, const Vector3D& v)
{
    return v * scalar;
}

bool Vector3D::operator==(const Vector3D& other) const
{
    constexpr double epsilon = 1e-10;
    return std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon;
}

bool Vector3D::operator!=(const Vector3D& other) const
{
    return !(*this == other);
}

double Vector3D::dot(const Vector3D& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3D Vector3D::cross(const Vector3D& other) const
{
    return Vector3D(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

double Vector3D::length() const
{
    return std::sqrt(lengthSquared());
}

double Vector3D::lengthSquared() const
{
    return x * x + y * y + z * z;
}

Vector3D Vector3D::normalized() const
{
    const double len = length();
    if (len < 1e-15)
    {
        throw std::invalid_argument("Cannot normalize zero-length vector");
    }
    return *this / len;
}

void Vector3D::normalize()
{
    const double len = length();
    if (len < 1e-15)
    {
        throw std::invalid_argument("Cannot normalize zero-length vector");
    }
    *this /= len;
}

bool Vector3D::isNormalized(double tolerance) const
{
    return std::abs(lengthSquared() - 1.0) < tolerance;
}

bool Vector3D::isZero(double tolerance) const
{
    return lengthSquared() < tolerance * tolerance;
}

double Vector3D::angleTo(const Vector3D& other) const
{
    const double lenProduct = length() * other.length();
    if (lenProduct < 1e-15)
    {
        return 0.0;
    }
    
    // Clamp to handle numerical errors
    double cosAngle = dot(other) / lenProduct;
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));
    
    return std::acos(cosAngle);
}

Vector3D Vector3D::projectOnto(const Vector3D& other) const
{
    const double otherLenSq = other.lengthSquared();
    if (otherLenSq < 1e-15)
    {
        return Vector3D::zero();
    }
    
    return other * (dot(other) / otherLenSq);
}

std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{
    os << "Vector3D(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
