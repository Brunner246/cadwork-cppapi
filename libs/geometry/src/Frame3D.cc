#include "geometry/Frame3D.hh"

#include <cmath>
#include <stdexcept>

namespace geometry {
// ============================================================================
// Constructors
// ============================================================================

Frame3D::Frame3D()
    : m_origin(Point3D::origin()), m_axisX(Vector3D::unitX()), m_axisY(Vector3D::unitY()),
      m_axisZ(Vector3D::unitZ()) {}

Frame3D::Frame3D(const Point3D &origin, const Vector3D &axisX, const Vector3D &axisY,
                 const Vector3D &axisZ)
    : m_origin(origin), m_axisX(axisX.normalized()), m_axisY(axisY.normalized()),
      m_axisZ(axisZ.normalized()) {}

Frame3D::Frame3D(const Point3D &origin, const Vector3D &axisX, const Vector3D &axisY)
    : m_origin(origin), m_axisX(axisX.normalized()), m_axisY(axisY.normalized()),
      m_axisZ(axisX.cross(axisY).normalized()) {}

// ============================================================================
// Accessors
// ============================================================================

void Frame3D::setOrigin(const Point3D &origin) {
    m_origin = origin;
}

void Frame3D::setAxes(const Vector3D &axisX, const Vector3D &axisY, const Vector3D &axisZ) {
    m_axisX = axisX.normalized();
    m_axisY = axisY.normalized();
    m_axisZ = axisZ.normalized();
}

// ============================================================================
// Matrix-Based Transformation Methods
// ============================================================================

/*
 * MATRIX TRANSFORMATION THEORY:
 * ============================
 *
 * A 4x4 homogeneous transformation matrix combines rotation and translation:
 *
 *     | Xx  Yx  Zx  Ox |   | px |   | Xx*px + Yx*py + Zx*pz + Ox |
 * M = | Xy  Yy  Zy  Oy | * | py | = | Xy*px + Yy*py + Zy*pz + Oy |
 *     | Xz  Yz  Zz  Oz |   | pz |   | Xz*px + Yz*py + Zz*pz + Oz |
 *     | 0   0   0   1  |   | 1  |   | 1                          |
 *
 * Where:
 * - (Xx, Xy, Xz) = X-axis of local frame in world coordinates
 * - (Yx, Yy, Yz) = Y-axis of local frame in world coordinates
 * - (Zx, Zy, Zz) = Z-axis of local frame in world coordinates
 * - (Ox, Oy, Oz) = Origin of local frame in world coordinates
 *
 * The inverse (world to local) is efficient for orthonormal matrices:
 * - Rotation inverse = transpose
 * - Translation needs to be rotated by inverse rotation
 */

Point3D Frame3D::worldToLocalMatrix(const Point3D &worldPoint) const {
    // Get the inverse transformation matrix
    const auto invMatrix = getInverseTransformationMatrix();

    // Apply transformation: P_local = M^(-1) * P_world
    // Using homogeneous coordinates (w = 1)
    const double lx = invMatrix[0] * worldPoint.x + invMatrix[1] * worldPoint.y
                    + invMatrix[2] * worldPoint.z + invMatrix[3];
    const double ly = invMatrix[4] * worldPoint.x + invMatrix[5] * worldPoint.y
                    + invMatrix[6] * worldPoint.z + invMatrix[7];
    const double lz = invMatrix[8] * worldPoint.x + invMatrix[9] * worldPoint.y
                    + invMatrix[10] * worldPoint.z + invMatrix[11];

    return {lx, ly, lz};
}

Point3D Frame3D::localToWorldMatrix(const Point3D &localPoint) const {
    // Get the transformation matrix
    const auto matrix = getTransformationMatrix();

    // Apply transformation: P_world = M * P_local
    const double wx =
        matrix[0] * localPoint.x + matrix[1] * localPoint.y + matrix[2] * localPoint.z + matrix[3];
    const double wy =
        matrix[4] * localPoint.x + matrix[5] * localPoint.y + matrix[6] * localPoint.z + matrix[7];
    const double wz = matrix[8] * localPoint.x + matrix[9] * localPoint.y
                    + matrix[10] * localPoint.z + matrix[11];

    return {wx, wy, wz};
}

Vector3D Frame3D::worldToLocalMatrix(const Vector3D &worldVector) const {
    // Vectors only use the rotation part (no translation)
    // For orthonormal frame, inverse rotation = transpose
    const double lx =
        m_axisX.x * worldVector.x + m_axisX.y * worldVector.y + m_axisX.z * worldVector.z;
    const double ly =
        m_axisY.x * worldVector.x + m_axisY.y * worldVector.y + m_axisY.z * worldVector.z;
    const double lz =
        m_axisZ.x * worldVector.x + m_axisZ.y * worldVector.y + m_axisZ.z * worldVector.z;

    return {lx, ly, lz};
}

Vector3D Frame3D::localToWorldMatrix(const Vector3D &localVector) const {
    // V_world = R * V_local (rotation matrix times local vector)
    const double wx =
        m_axisX.x * localVector.x + m_axisY.x * localVector.y + m_axisZ.x * localVector.z;
    const double wy =
        m_axisX.y * localVector.x + m_axisY.y * localVector.y + m_axisZ.y * localVector.z;
    const double wz =
        m_axisX.z * localVector.x + m_axisY.z * localVector.y + m_axisZ.z * localVector.z;

    return {wx, wy, wz};
}

std::array<double, 16> Frame3D::getTransformationMatrix() const {
    /*
     * Row-major 4x4 matrix:
     *
     * | m[0]  m[1]  m[2]  m[3]  |   | Xx  Yx  Zx  Ox |
     * | m[4]  m[5]  m[6]  m[7]  | = | Xy  Yy  Zy  Oy |
     * | m[8]  m[9]  m[10] m[11] |   | Xz  Yz  Zz  Oz |
     * | m[12] m[13] m[14] m[15] |   | 0   0   0   1  |
     */
    return {
        m_axisX.x, m_axisY.x, m_axisZ.x, m_origin.x, // Row 0
        m_axisX.y, m_axisY.y, m_axisZ.y, m_origin.y, // Row 1
        m_axisX.z, m_axisY.z, m_axisZ.z, m_origin.z, // Row 2
        0.0,       0.0,       0.0,       1.0         // Row 3
    };
}

std::array<double, 16> Frame3D::getInverseTransformationMatrix() const {
    /*
     * For orthonormal frame, the inverse is:
     *
     * | Xx  Xy  Xz  -X·O |
     * | Yx  Yy  Yz  -Y·O |
     * | Zx  Zy  Zz  -Z·O |
     * | 0   0   0    1   |
     *
     * The 3x3 rotation part is transposed.
     * The translation part is: -R^T * O = -(axis · origin)
     */
    const Vector3D originVec(m_origin.x, m_origin.y, m_origin.z);

    return {
        m_axisX.x, m_axisX.y, m_axisX.z, -m_axisX.dot(originVec), // Row 0
        m_axisY.x, m_axisY.y, m_axisY.z, -m_axisY.dot(originVec), // Row 1
        m_axisZ.x, m_axisZ.y, m_axisZ.z, -m_axisZ.dot(originVec), // Row 2
        0.0,       0.0,       0.0,       1.0                      // Row 3
    };
}

// ============================================================================
// Dot Product-Based Transformation Methods
// ============================================================================

/*
 * DOT PRODUCT TRANSFORMATION THEORY:
 * ==================================
 *
 * This approach uses the geometric interpretation of dot product:
 *   a · b = |a| * |b| * cos(θ) = projection of a onto b (if b is unit)
 *
 * For orthonormal basis vectors, the dot product gives the component
 * of a vector along each axis directly.
 *
 * WORLD TO LOCAL:
 * ---------------
 * Given point P in world coordinates:
 * 1. Compute displacement from frame origin: V = P - Origin
 * 2. Project V onto each axis:
 *    local.x = V · axisX  (how far along X?)
 *    local.y = V · axisY  (how far along Y?)
 *    local.z = V · axisZ  (how far along Z?)
 *
 * LOCAL TO WORLD:
 * ---------------
 * Given local coordinates (lx, ly, lz):
 *    P_world = Origin + lx*axisX + ly*axisY + lz*axisZ
 *
 * This reconstructs the world position by starting at the frame
 * origin and moving along each axis by the local coordinate amount.
 */

Point3D Frame3D::worldToLocalDotProduct(const Point3D &worldPoint) const {
    // Step 1: Get displacement vector from frame origin to world point
    const Vector3D displacement = worldPoint - m_origin;

    // Step 2: Project displacement onto each axis using dot product
    // For unit vectors: component = displacement · axis
    const double localX = displacement.dot(m_axisX);
    const double localY = displacement.dot(m_axisY);
    const double localZ = displacement.dot(m_axisZ);

    return {localX, localY, localZ};
}

Point3D Frame3D::localToWorldDotProduct(const Point3D &localPoint) const {
    // Start at frame origin
    // Add contribution along each axis: local_coord * axis_vector
    const double worldX =
        m_origin.x + localPoint.x * m_axisX.x + localPoint.y * m_axisY.x + localPoint.z * m_axisZ.x;

    const double worldY =
        m_origin.y + localPoint.x * m_axisX.y + localPoint.y * m_axisY.y + localPoint.z * m_axisZ.y;

    const double worldZ =
        m_origin.z + localPoint.x * m_axisX.z + localPoint.y * m_axisY.z + localPoint.z * m_axisZ.z;

    return {worldX, worldY, worldZ};
}

Vector3D Frame3D::worldToLocalDotProduct(const Vector3D &worldVector) const {
    // For vectors, no translation needed - just project onto axes
    const double localX = worldVector.dot(m_axisX);
    const double localY = worldVector.dot(m_axisY);
    const double localZ = worldVector.dot(m_axisZ);

    return {localX, localY, localZ};
}

Vector3D Frame3D::localToWorldDotProduct(const Vector3D &localVector) const {
    // Reconstruct world vector from local components
    const double worldX =
        localVector.x * m_axisX.x + localVector.y * m_axisY.x + localVector.z * m_axisZ.x;

    const double worldY =
        localVector.x * m_axisX.y + localVector.y * m_axisY.y + localVector.z * m_axisZ.y;

    const double worldZ =
        localVector.x * m_axisX.z + localVector.y * m_axisY.z + localVector.z * m_axisZ.z;

    return {worldX, worldY, worldZ};
}

// ============================================================================
// Convenience Methods
// ============================================================================

Point3D Frame3D::worldToLocal(const Point3D &worldPoint) const {
    // Use dot product method - slightly more efficient (no matrix construction)
    return worldToLocalDotProduct(worldPoint);
}

Point3D Frame3D::localToWorld(const Point3D &localPoint) const {
    return localToWorldDotProduct(localPoint);
}

Vector3D Frame3D::worldToLocal(const Vector3D &worldVector) const {
    return worldToLocalDotProduct(worldVector);
}

Vector3D Frame3D::localToWorld(const Vector3D &localVector) const {
    return localToWorldDotProduct(localVector);
}

// ============================================================================
// Frame Operations
// ============================================================================

bool Frame3D::isOrthonormal(double tolerance) const {
    // Check that axes are unit length
    if (std::abs(m_axisX.lengthSquared() - 1.0) > tolerance)
        return false;
    if (std::abs(m_axisY.lengthSquared() - 1.0) > tolerance)
        return false;
    if (std::abs(m_axisZ.lengthSquared() - 1.0) > tolerance)
        return false;

    // Check that axes are orthogonal (dot products are zero)
    if (std::abs(m_axisX.dot(m_axisY)) > tolerance)
        return false;
    if (std::abs(m_axisY.dot(m_axisZ)) > tolerance)
        return false;
    if (std::abs(m_axisZ.dot(m_axisX)) > tolerance)
        return false;

    return true;
}

Frame3D Frame3D::worldFrame() {
    return {};
}

std::ostream &operator<<(std::ostream &os, const Frame3D &frame) {
    os << "Frame3D{\n"
       << "  Origin: " << frame.m_origin << "\n"
       << "  X-Axis: " << frame.m_axisX << "\n"
       << "  Y-Axis: " << frame.m_axisY << "\n"
       << "  Z-Axis: " << frame.m_axisZ << "\n"
       << "}";
    return os;
}
} // namespace geometry
