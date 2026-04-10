#ifndef PLANE3D_HH
#define PLANE3D_HH

#include "Point3D.hh"
#include "Vector3D.hh"

#include <optional>
#include <ostream>

/**
 * @brief Represents an infinite plane in 3D space.
 *
 * A plane is defined by a point on the plane and a normal vector.
 * The plane equation is: n · (P - P0) = 0, or equivalently: ax + by + cz + d = 0
 * where (a, b, c) is the normal vector and d = -n · P0.
 */
class Plane3D {
  public:
    [[nodiscard]] static Plane3D fromDefault() { return {}; }

    [[nodiscard]] static Plane3D fromPointAndNormal(const Point3D &point, const Vector3D &normal) {
        return {point, normal};
    }

    [[nodiscard]] static Plane3D fromThreePoints(const Point3D &p1, const Point3D &p2,
                                                 const Point3D &p3) {
        return {p1, p2, p3};
    }

    [[nodiscard]] static Plane3D fromCoefficients(double a, double b, double c, double d) {
        return {a, b, c, d};
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    [[nodiscard]] const Point3D &point() const { return m_point; }
    [[nodiscard]] const Vector3D &normal() const { return m_normal; }

    /**
     * @brief Get the d coefficient in plane equation ax + by + cz + d = 0.
     */
    [[nodiscard]] double d() const;

    /**
     * @brief Get coefficients (a, b, c, d) of plane equation.
     */
    [[nodiscard]] std::array<double, 4> coefficients() const;

    // ========================================================================
    // Distance Operations
    // ========================================================================

    /**
     * @brief Compute signed distance from a point to the plane.
     *
     * Positive distance means the point is on the side of the normal.
     * Negative distance means the point is on the opposite side.
     *
     * Formula: distance = n · (P - P0) = n · P + d
     *
     * @param point The point to measure distance from.
     * @return Signed distance (positive = same side as normal).
     */
    [[nodiscard]] double signedDistanceTo(const Point3D &point) const;

    /**
     * @brief Compute absolute distance from a point to the plane.
     * @param point The point to measure distance from.
     * @return Absolute (unsigned) distance.
     */
    [[nodiscard]] double distanceTo(const Point3D &point) const;

    // ========================================================================
    // Point Classification
    // ========================================================================

    /**
     * @brief Check if a point lies on the plane (within tolerance).
     */
    [[nodiscard]] bool contains(const Point3D &point, double tolerance = 1e-10) const;

    /**
     * @brief Check if a point is on the positive side of the plane (normal side).
     */
    [[nodiscard]] bool isAbove(const Point3D &point) const;

    /**
     * @brief Check if a point is on the negative side of the plane.
     */
    [[nodiscard]] bool isBelow(const Point3D &point) const;

    // ========================================================================
    // Projection Operations
    // ========================================================================

    /**
     * @brief Project a point onto the plane.
     *
     * Returns the closest point on the plane to the given point.
     * Formula: P_projected = P - (n · (P - P0)) * n
     *
     * @param point The point to project.
     * @return The projected point on the plane.
     */
    [[nodiscard]] Point3D projectPoint(const Point3D &point) const;

    /**
     * @brief Project a vector onto the plane.
     *
     * Returns the component of the vector that lies in the plane.
     * Formula: V_projected = V - (V · n) * n
     *
     * @param vector The vector to project.
     * @return The projected vector (lies in the plane).
     */
    [[nodiscard]] Vector3D projectVector(const Vector3D &vector) const;

    // ========================================================================
    // Plane Relationships
    // ========================================================================

    /**
     * @brief Check if two planes are parallel (within tolerance).
     *
     * Two planes are parallel if their normals are parallel (or anti-parallel).
     */
    [[nodiscard]] bool isParallelTo(const Plane3D &other, double tolerance = 1e-10) const;

    /**
     * @brief Check if two planes are perpendicular (within tolerance).
     */
    [[nodiscard]] bool isPerpendicularTo(const Plane3D &other, double tolerance = 1e-10) const;

    /**
     * @brief Check if two planes are coplanar (same plane, within tolerance).
     */
    [[nodiscard]] bool isCoplanarWith(const Plane3D &other, double tolerance = 1e-10) const;

    /**
     * @brief Compute the angle between two planes (dihedral angle).
     *
     * Returns the angle in radians between 0 and π/2.
     */
    [[nodiscard]] double angleTo(const Plane3D &other) const;

    /**
     * @brief Compute the distance between two parallel planes.
     *
     * @return Distance if planes are parallel, std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<double> distanceTo(const Plane3D &other,
                                                   double tolerance = 1e-10) const;

    // ========================================================================
    // Line/Ray Intersection
    // ========================================================================

    /**
     * @brief Find intersection point of a line with the plane.
     *
     * A line is defined by a point and direction: L(t) = linePoint + t * lineDir
     *
     * @param linePoint A point on the line.
     * @param lineDir Direction of the line (does not need to be normalized).
     * @return Intersection point, or std::nullopt if line is parallel to plane.
     */
    [[nodiscard]] std::optional<Point3D> intersectLine(const Point3D &linePoint,
                                                       const Vector3D &lineDir) const;

    /**
     * @brief Find intersection parameter t where line intersects plane.
     *
     * The intersection point is: linePoint + t * lineDir
     *
     * @return Parameter t, or std::nullopt if line is parallel to plane.
     */
    [[nodiscard]] std::optional<double> intersectLineParameter(const Point3D &linePoint,
                                                               const Vector3D &lineDir) const;

    // ========================================================================
    // Plane Transformations
    // ========================================================================

    /**
     * @brief Create a plane offset by a distance along its normal.
     * @param distance Offset distance (positive = in normal direction).
     */
    [[nodiscard]] Plane3D offset(double distance) const;

    /**
     * @brief Create a plane with flipped normal.
     */
    [[nodiscard]] Plane3D flipped() const;

    // ========================================================================
    // Static Factory Methods
    // ========================================================================

    static Plane3D xy(double z = 0.0); ///< XY plane at given z
    static Plane3D xz(double y = 0.0); ///< XZ plane at given y
    static Plane3D yz(double x = 0.0); ///< YZ plane at given x

    // ========================================================================
    // Comparison
    // ========================================================================

    bool operator==(const Plane3D &other) const;
    bool operator!=(const Plane3D &other) const;

    // Stream output
    friend std::ostream &operator<<(std::ostream &os, const Plane3D &plane);

  private:
    Point3D m_point;   ///< A point on the plane
    Vector3D m_normal; ///< Unit normal vector

    /**
     * @brief Default constructor creates the XY plane (z = 0).
     */
    Plane3D();

    /**
     * @brief Construct a plane from a point and normal vector.
     * @param point A point on the plane.
     * @param normal The normal vector (will be normalized).
     */
    Plane3D(const Point3D &point, const Vector3D &normal);

    /**
     * @brief Construct a plane from three non-collinear points.
     *
     * The normal is computed as (p2 - p1) × (p3 - p1), following right-hand rule.
     *
     * @param p1 First point on the plane.
     * @param p2 Second point on the plane.
     * @param p3 Third point on the plane.
     * @throws std::invalid_argument if points are collinear.
     */
    Plane3D(const Point3D &p1, const Point3D &p2, const Point3D &p3);

    /**
     * @brief Construct a plane from coefficients ax + by + cz + d = 0.
     * @param a X coefficient of normal.
     * @param b Y coefficient of normal.
     * @param c Z coefficient of normal.
     * @param d Distance coefficient.
     */
    Plane3D(double a, double b, double c, double d);

    // Copy and move (defaulted)
    Plane3D(const Plane3D &) = default;
    Plane3D(Plane3D &&) noexcept = default;
    Plane3D &operator=(const Plane3D &) = default;
    Plane3D &operator=(Plane3D &&) noexcept = default;
    ~Plane3D() = default;
};

#endif // PLANE3D_HH
