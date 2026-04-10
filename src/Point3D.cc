#include "Point3D.hh"
#include "Vector3D.hh"

Point3D Point3D::operator+(const Vector3D& v) const
{
    return Point3D(x + v.x, y + v.y, z + v.z);
}

Point3D Point3D::operator-(const Vector3D& v) const
{
    return Point3D(x - v.x, y - v.y, z - v.z);
}

Point3D& Point3D::operator+=(const Vector3D& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Point3D& Point3D::operator-=(const Vector3D& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector3D Point3D::operator-(const Point3D& other) const
{
    return Vector3D(x - other.x, y - other.y, z - other.z);
}

bool Point3D::operator==(const Point3D& other) const
{
    constexpr double epsilon = 1e-10;
    return std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon;
}

bool Point3D::operator!=(const Point3D& other) const
{
    return !(*this == other);
}

double Point3D::distanceTo(const Point3D& other) const
{
    return std::sqrt(distanceSquaredTo(other));
}

double Point3D::distanceSquaredTo(const Point3D& other) const
{
    const double dx = x - other.x;
    const double dy = y - other.y;
    const double dz = z - other.z;
    return dx * dx + dy * dy + dz * dz;
}

std::ostream& operator<<(std::ostream& os, const Point3D& p)
{
    os << "Point3D(" << p.x << ", " << p.y << ", " << p.z << ")";
    return os;
}
