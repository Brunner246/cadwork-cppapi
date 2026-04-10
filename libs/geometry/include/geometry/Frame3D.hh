#ifndef GEOMETRY_FRAME3D_HH
#define GEOMETRY_FRAME3D_HH

#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"
#include "geometry/export.hh"

#include <array>
#include <ostream>


namespace geometry {
/**
 * @brief Represents a 3D coordinate frame (coordinate system).
 *
 * A Frame3D defines a local coordinate system with an origin and three
 * orthonormal axis vectors (X, Y, Z). It can be used to transform points
 * between world coordinates and local coordinates.
 *
 * This class demonstrates two mathematically equivalent transformation methods:
 * 1. Matrix-based transformation (using 4x4 homogeneous transformation matrix)
 * 2. Dot product-based transformation (direct vector operations)
 *
 * Both methods produce identical results but illustrate different approaches
 * to the same mathematical problem.
 */
class GEOMETRY_API Frame3D {
  public:
    // ========================================================================
    // Constructors
    // ========================================================================

    /**
     * @brief Default constructor creates the world frame (identity).
     */
    Frame3D();

    /**
     * @brief Construct a frame from origin and axis vectors.
     * @param origin The origin point of the coordinate system.
     * @param axisX The X-axis direction (will be normalized).
     * @param axisY The Y-axis direction (will be normalized).
     * @param axisZ The Z-axis direction (will be normalized).
     * @note The axes should be orthogonal. If not, results may be unexpected.
     */
    explicit Frame3D(const Point3D &origin, const Vector3D &axisX, const Vector3D &axisY,
                     const Vector3D &axisZ);

    /**
     * @brief Construct a frame from origin and two axis vectors.
     *
     * The Z-axis is computed as the cross product of X and Y.
     *
     * @param origin The origin point of the coordinate system.
     * @param axisX The X-axis direction (will be normalized).
     * @param axisY The Y-axis direction (will be normalized).
     */
    explicit Frame3D(const Point3D &origin, const Vector3D &axisX, const Vector3D &axisY);

    // Copy and move (defaulted)
    Frame3D(const Frame3D &) = default;
    Frame3D(Frame3D &&) noexcept = default;
    Frame3D &operator=(const Frame3D &) = default;
    Frame3D &operator=(Frame3D &&) noexcept = default;
    ~Frame3D() = default;

    // ========================================================================
    // Accessors
    // ========================================================================

    [[nodiscard]] const Point3D &origin() const { return m_origin; }
    [[nodiscard]] const Vector3D &axisX() const { return m_axisX; }
    [[nodiscard]] const Vector3D &axisY() const { return m_axisY; }
    [[nodiscard]] const Vector3D &axisZ() const { return m_axisZ; }

    void setOrigin(const Point3D &origin);
    void setAxes(const Vector3D &axisX, const Vector3D &axisY, const Vector3D &axisZ);

    // ========================================================================
    // Matrix-Based Transformation Methods
    // ========================================================================

    /**
     * @brief Transform a point from world coordinates to local coordinates using matrix.
     *
     * MATHEMATICAL EXPLANATION:
     * The transformation matrix from local to world is:
     *
     *     | Xx  Yx  Zx  Ox |
     * M = | Xy  Yy  Zy  Oy |
     *     | Xz  Yz  Zz  Oz |
     *     | 0   0   0   1  |
     *
     * where (Xx, Xy, Xz) is the X-axis, etc., and (Ox, Oy, Oz) is the origin.
     *
     * To transform from world to local, we need M^(-1):
     *
     *         | Xx  Xy  Xz  -X·O |
     * M^(-1) = | Yx  Yy  Yz  -Y·O |
     *         | Zx  Zy  Zz  -Z·O |
     *         | 0   0   0    1   |
     *
     * The inverse is simple because the rotation part is orthonormal (transpose = inverse).
     *
     * @param worldPoint Point in world coordinates.
     * @return Point in local coordinates.
     */
    [[nodiscard]] Point3D worldToLocalMatrix(const Point3D &worldPoint) const;

    /**
     * @brief Transform a point from local coordinates to world coordinates using matrix.
     *
     * Uses the transformation matrix directly:
     *     P_world = M * P_local
     *
     * @param localPoint Point in local coordinates.
     * @return Point in world coordinates.
     */
    [[nodiscard]] Point3D localToWorldMatrix(const Point3D &localPoint) const;

    /**
     * @brief Transform a vector from world coordinates to local coordinates using matrix.
     *
     * Vectors are transformed using only the rotation part of the matrix
     * (translation does not affect direction).
     */
    [[nodiscard]] Vector3D worldToLocalMatrix(const Vector3D &worldVector) const;

    /**
     * @brief Transform a vector from local coordinates to world coordinates using matrix.
     */
    [[nodiscard]] Vector3D localToWorldMatrix(const Vector3D &localVector) const;

    /**
     * @brief Get the 4x4 transformation matrix (local to world).
     *
     * Matrix layout (row-major, indices 0-15):
     *     | m[0]  m[1]  m[2]  m[3]  |   | Xx  Yx  Zx  Ox |
     *     | m[4]  m[5]  m[6]  m[7]  | = | Xy  Yy  Zy  Oy |
     *     | m[8]  m[9]  m[10] m[11] |   | Xz  Yz  Zz  Oz |
     *     | m[12] m[13] m[14] m[15] |   | 0   0   0   1  |
     */
    [[nodiscard]] std::array<double, 16> getTransformationMatrix() const;

    /**
     * @brief Get the inverse 4x4 transformation matrix (world to local).
     */
    [[nodiscard]] std::array<double, 16> getInverseTransformationMatrix() const;

    // ========================================================================
    // Dot Product-Based Transformation Methods
    // ========================================================================

    /**
     * @brief Transform a point from world coordinates to local coordinates using dot product.
     *
     * MATHEMATICAL EXPLANATION:
     * Given a point P in world coordinates and frame F with origin O and axes X, Y, Z:
     *
     * The local coordinates (lx, ly, lz) are found by:
     * 1. Translate: V = P - O  (vector from frame origin to point)
     * 2. Project onto each axis using dot product:
     *    lx = V · X  (component along X-axis)
     *    ly = V · Y  (component along Y-axis)
     *    lz = V · Z  (component along Z-axis)
     *
     * This works because for orthonormal axes:
     *    V = lx*X + ly*Y + lz*Z
     * And taking dot product with X: V·X = lx*(X·X) + ly*(Y·X) + lz*(Z·X)
     * Since X·X = 1 and Y·X = Z·X = 0 (orthonormal), we get: V·X = lx
     *
     * @param worldPoint Point in world coordinates.
     * @return Point in local coordinates.
     */
    [[nodiscard]] Point3D worldToLocalDotProduct(const Point3D &worldPoint) const;

    /**
     * @brief Transform a point from local coordinates to world coordinates using dot product.
     *
     * MATHEMATICAL EXPLANATION:
     * Given local coordinates (lx, ly, lz) and frame F with origin O and axes X, Y, Z:
     *
     * The world point P is:
     *    P = O + lx*X + ly*Y + lz*Z
     *
     * This is just the parametric equation of a point in 3D space,
     * expressing the point as origin plus linear combination of basis vectors.
     *
     * @param localPoint Point in local coordinates.
     * @return Point in world coordinates.
     */
    [[nodiscard]] Point3D localToWorldDotProduct(const Point3D &localPoint) const;

    /**
     * @brief Transform a vector from world to local coordinates using dot product.
     *
     * Similar to point transformation, but without translation:
     *    lx = V · X
     *    ly = V · Y
     *    lz = V · Z
     */
    [[nodiscard]] Vector3D worldToLocalDotProduct(const Vector3D &worldVector) const;

    /**
     * @brief Transform a vector from local to world coordinates using dot product.
     *
     *    V_world = lx*X + ly*Y + lz*Z
     */
    [[nodiscard]] Vector3D localToWorldDotProduct(const Vector3D &localVector) const;

    // ========================================================================
    // Convenience Methods (use dot product internally - slightly more efficient)
    // ========================================================================

    /**
     * @brief Transform point from world to local coordinates.
     */
    [[nodiscard]] Point3D worldToLocal(const Point3D &worldPoint) const;

    /**
     * @brief Transform point from local to world coordinates.
     */
    [[nodiscard]] Point3D localToWorld(const Point3D &localPoint) const;

    /**
     * @brief Transform vector from world to local coordinates.
     */
    [[nodiscard]] Vector3D worldToLocal(const Vector3D &worldVector) const;

    /**
     * @brief Transform vector from local to world coordinates.
     */
    [[nodiscard]] Vector3D localToWorld(const Vector3D &localVector) const;

    // ========================================================================
    // Frame Operations
    // ========================================================================

    /**
     * @brief Check if the frame axes are orthonormal within tolerance.
     */
    [[nodiscard]] bool isOrthonormal(double tolerance = 1e-10) const;

    /**
     * @brief Create the world coordinate frame (identity frame).
     */
    static Frame3D worldFrame();

    // Stream output
    friend GEOMETRY_API std::ostream &operator<<(std::ostream &os, const Frame3D &frame);

  private:
    Point3D m_origin; ///< Origin of the coordinate system
    Vector3D m_axisX; ///< X-axis (unit vector)
    Vector3D m_axisY; ///< Y-axis (unit vector)
    Vector3D m_axisZ; ///< Z-axis (unit vector)
};
} // namespace geometry
#endif // GEOMETRY_FRAME3D_HH
