#include "geometry/Plane3D.hh"

#include <array>
#include <cmath>
#include <stdexcept>

namespace geometry {
// ============================================================================
// Constructors
// ============================================================================

Plane3D::Plane3D() : m_point(Point3D::origin()), m_normal(Vector3D::unitZ()) {}

Plane3D::Plane3D(const Point3D &point, const Vector3D &normal)
    : m_point(point), m_normal(normal.normalized()) {}

Plane3D::Plane3D(const Point3D &p1, const Point3D &p2, const Point3D &p3) : m_point(p1) {
    const Vector3D v1 = p2 - p1;
    const Vector3D v2 = p3 - p1;
    const Vector3D cross = v1.cross(v2);

    if (cross.isZero()) {
        throw std::invalid_argument("Cannot create plane from collinear points");
    }

    m_normal = cross.normalized();
}

Plane3D::Plane3D(double a, double b, double c, double d) {
    const Vector3D normal(a, b, c);
    if (normal.isZero()) {
        throw std::invalid_argument("Plane normal cannot be zero");
    }

    m_normal = normal.normalized();

    // Find a point on the plane: ax + by + cz + d = 0
    // Choose the simplest point by setting two coordinates to 0
    if (std::abs(c) > 1e-10) {
        m_point = Point3D(0.0, 0.0, -d / c);
    } else if (std::abs(b) > 1e-10) {
        m_point = Point3D(0.0, -d / b, 0.0);
    } else {
        m_point = Point3D(-d / a, 0.0, 0.0);
    }
}

// ============================================================================
// Accessors
// ============================================================================

double Plane3D::d() const {
    // d = -n · P0
    return -(m_normal.x * m_point.x + m_normal.y * m_point.y + m_normal.z * m_point.z);
}

std::array<double, 4> Plane3D::coefficients() const {
    return {m_normal.x, m_normal.y, m_normal.z, d()};
}

// ============================================================================
// Distance Operations
// ============================================================================

double Plane3D::signedDistanceTo(const Point3D &point) const {
    // Signed distance = n · (P - P0) = n · P - n · P0 = n · P + d
    const Vector3D toPoint = point - m_point;
    return m_normal.dot(toPoint);
}

double Plane3D::distanceTo(const Point3D &point) const {
    return std::abs(signedDistanceTo(point));
}

// ============================================================================
// Point Classification
// ============================================================================

bool Plane3D::contains(const Point3D &point, double tolerance) const {
    return std::abs(signedDistanceTo(point)) < tolerance;
}

bool Plane3D::isAbove(const Point3D &point) const {
    return signedDistanceTo(point) > 0.0;
}

bool Plane3D::isBelow(const Point3D &point) const {
    return signedDistanceTo(point) < 0.0;
}

// ============================================================================
// Projection Operations
// ============================================================================

Point3D Plane3D::projectPoint(const Point3D &point) const {
    // P_projected = P - (signed_distance) * n
    const double dist = signedDistanceTo(point);
    return Point3D(point.x - dist * m_normal.x, point.y - dist * m_normal.y,
                   point.z - dist * m_normal.z);
}

Vector3D Plane3D::projectVector(const Vector3D &vector) const {
    // V_projected = V - (V · n) * n
    const double dotProduct = vector.dot(m_normal);
    return Vector3D(vector.x - dotProduct * m_normal.x, vector.y - dotProduct * m_normal.y,
                    vector.z - dotProduct * m_normal.z);
}

// ============================================================================
// Plane Relationships
// ============================================================================

bool Plane3D::isParallelTo(const Plane3D &other, double tolerance) const {
    // Planes are parallel if their normals are parallel (cross product is zero)
    const Vector3D cross = m_normal.cross(other.m_normal);
    return cross.lengthSquared() < tolerance * tolerance;
}

bool Plane3D::isPerpendicularTo(const Plane3D &other, double tolerance) const {
    // Planes are perpendicular if their normals are perpendicular
    return std::abs(m_normal.dot(other.m_normal)) < tolerance;
}

bool Plane3D::isCoplanarWith(const Plane3D &other, double tolerance) const {
    // Planes are coplanar if they are parallel and share a point
    if (!isParallelTo(other, tolerance)) {
        return false;
    }

    // Check if the other plane's point lies on this plane
    return contains(other.m_point, tolerance);
}

double Plane3D::angleTo(const Plane3D &other) const {
    // The angle between planes is the angle between their normals
    // (or its supplement, we take the smaller angle)
    double cosAngle = m_normal.dot(other.m_normal);

    // Clamp for numerical stability
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));

    // Return angle between 0 and π/2
    return std::acos(std::abs(cosAngle));
}

std::optional<double> Plane3D::distanceTo(const Plane3D &other, double tolerance) const {
    if (!isParallelTo(other, tolerance)) {
        return std::nullopt;
    }

    // Distance between parallel planes = |signed distance from other's point to this plane|
    return std::abs(signedDistanceTo(other.m_point));
}

// ============================================================================
// Line/Ray Intersection
// ============================================================================

std::optional<double> Plane3D::intersectLineParameter(const Point3D &linePoint,
                                                      const Vector3D &lineDir) const {
    // Line: P(t) = linePoint + t * lineDir
    // Plane: n · (P - p0) = 0
    // Substituting: n · (linePoint + t * lineDir - p0) = 0
    // t = n · (p0 - linePoint) / (n · lineDir)

    const double denominator = m_normal.dot(lineDir);

    if (std::abs(denominator) < 1e-15) {
        // Line is parallel to plane
        return std::nullopt;
    }

    const Vector3D toPlane = m_point - linePoint;
    const double t = m_normal.dot(toPlane) / denominator;

    return t;
}

std::optional<Point3D> Plane3D::intersectLine(const Point3D &linePoint,
                                              const Vector3D &lineDir) const {
    const auto t = intersectLineParameter(linePoint, lineDir);

    if (!t.has_value()) {
        return std::nullopt;
    }

    return Point3D(linePoint.x + t.value() * lineDir.x, linePoint.y + t.value() * lineDir.y,
                   linePoint.z + t.value() * lineDir.z);
}

// ============================================================================
// Plane Transformations
// ============================================================================

Plane3D Plane3D::offset(double distance) const {
    const Point3D newPoint(m_point.x + distance * m_normal.x, m_point.y + distance * m_normal.y,
                           m_point.z + distance * m_normal.z);
    return Plane3D(newPoint, m_normal);
}

Plane3D Plane3D::flipped() const {
    return Plane3D(m_point, -m_normal);
}

// ============================================================================
// Static Factory Methods
// ============================================================================

Plane3D Plane3D::xy(double z) {
    return Plane3D(Point3D(0.0, 0.0, z), Vector3D::unitZ());
}

Plane3D Plane3D::xz(double y) {
    return Plane3D(Point3D(0.0, y, 0.0), Vector3D::unitY());
}

Plane3D Plane3D::yz(double x) {
    return Plane3D(Point3D(x, 0.0, 0.0), Vector3D::unitX());
}

// ============================================================================
// Comparison
// ============================================================================

bool Plane3D::operator==(const Plane3D &other) const {
    return isCoplanarWith(other);
}

bool Plane3D::operator!=(const Plane3D &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Plane3D &plane) {
    const auto coeff = plane.coefficients();
    os << "Plane3D(" << coeff[0] << "x + " << coeff[1] << "y + " << coeff[2] << "z + " << coeff[3]
       << " = 0)";
    return os;
}
} // namespace geometry
