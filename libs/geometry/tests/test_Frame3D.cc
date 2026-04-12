/**
 * @file test_Frame3D.cc
 * @brief Unit tests for geometry::Frame3D.
 *
 * A Frame3D is a right-handed 3D coordinate system consisting of:
 *   - an origin Point3D  (translation)
 *   - three orthonormal axis vectors X, Y, Z  (rotation)
 *
 * It supports two mathematically equivalent transformation methods:
 *
 *   Matrix-based  — builds the 4×4 homogeneous matrix and applies it.
 *   Dot-product   — projects the displacement onto each axis directly.
 *
 * Both should give identical results; the tests verify this equivalence.
 *
 * Coordinate transformation conventions
 * --------------------------------------
 *   worldToLocal(P)  — expresses a world-space point in frame-local coords.
 *   localToWorld(P)  — reconstructs the world position from local coords.
 *
 * The round-trip localToWorld(worldToLocal(P)) must equal P.
 *
 * Test groups
 * -----------
 * Construction
 *   Default / world frame is the identity: origin at (0,0,0), axes = world
 *   3-axis constructor normalizes the supplied axis vectors.
 *   2-axis constructor derives Z = normalize(X × Y).
 *
 * isOrthonormal
 *   The default frame always passes; a non-orthogonal set must fail.
 *
 * worldToLocal / localToWorld — identity frame
 *   Both transform functions are identity for the world frame.
 *
 * worldToLocal / localToWorld — translated frame
 *   Frame at offset O with world-aligned axes: subtracts / adds the offset.
 *
 * worldToLocal / localToWorld — rotated frame
 *   90° rotation around Z: verifies that axes and coordinates rotate correctly.
 *
 * Round-trip consistency
 *   localToWorld(worldToLocal(P)) == P for a rotated + translated frame.
 *
 * Method equivalence
 *   Matrix method == dot-product method for both points and vectors.
 *
 * Vector transformations
 *   Vectors are not affected by frame translation (only rotation).
 *
 * getTransformationMatrix / getInverseTransformationMatrix
 *   Applying M then M⁻¹ gives the identity transformation.
 *
 * Mutators
 *   setOrigin() and setAxes() update the frame state correctly.
 *
 * worldFrame() static factory
 *   Returns the identity frame.
 *
 * Stream output
 *   operator<< emits a multi-line description.
 */

#include <doctest/doctest.h>

#include "geometry/Frame3D.hh"
#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <array>
#include <cmath>
#include <numbers>
#include <sstream>

using namespace geometry;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static constexpr double EPS = 1e-9;

static void checkPoint(const Point3D& a, const Point3D& b, double eps = EPS) {
    CHECK(std::abs(a.x - b.x) < eps);
    CHECK(std::abs(a.y - b.y) < eps);
    CHECK(std::abs(a.z - b.z) < eps);
}

static void checkVector(const Vector3D& a, const Vector3D& b, double eps = EPS) {
    CHECK(std::abs(a.x - b.x) < eps);
    CHECK(std::abs(a.y - b.y) < eps);
    CHECK(std::abs(a.z - b.z) < eps);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_SUITE("Frame3D") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("Default constructor creates the world (identity) frame") {
        /**
         * The world frame has its origin at (0,0,0) and its axes aligned with
         * the global X, Y, Z axes.  Transforming any point with this frame is
         * a no-op.
         */
        const Frame3D f;
        CHECK(f.origin() == Point3D::origin());
        CHECK(f.axisX() == Vector3D::unitX());
        CHECK(f.axisY() == Vector3D::unitY());
        CHECK(f.axisZ() == Vector3D::unitZ());
    }

    TEST_CASE("worldFrame() returns the identity frame") {
        const Frame3D f = Frame3D::worldFrame();
        CHECK(f.origin() == Point3D::origin());
        CHECK(f.axisX() == Vector3D::unitX());
        CHECK(f.axisY() == Vector3D::unitY());
        CHECK(f.axisZ() == Vector3D::unitZ());
    }

    TEST_CASE("3-axis constructor normalizes non-unit axis vectors") {
        /**
         * The constructor must normalize the axes so the frame is always
         * orthonormal.  Passing vectors of length != 1 should still work.
         */
        const Frame3D f(
            Point3D(1.0, 0.0, 0.0),
            Vector3D(2.0, 0.0, 0.0), // length 2
            Vector3D(0.0, 3.0, 0.0), // length 3
            Vector3D(0.0, 0.0, 5.0)  // length 5
        );
        checkVector(f.axisX(), Vector3D::unitX());
        checkVector(f.axisY(), Vector3D::unitY());
        checkVector(f.axisZ(), Vector3D::unitZ());
    }

    TEST_CASE("2-axis constructor derives Z = normalize(X cross Y)") {
        /**
         * Providing only X and Y; Z is computed as the cross product.
         * For X=(1,0,0) and Y=(0,1,0), Z should be (0,0,1).
         */
        const Frame3D f(
            Point3D::origin(),
            Vector3D::unitX(),
            Vector3D::unitY()
        );
        checkVector(f.axisZ(), Vector3D::unitZ());
    }

    TEST_CASE("2-axis constructor with 45-degree rotation in XY plane") {
        /**
         * If X is rotated 45° around Z, the resulting frame should still
         * be orthonormal and its Z-axis should still be (0,0,1).
         */
        const double c = std::cos(std::numbers::pi / 4.0);
        const double s = std::sin(std::numbers::pi / 4.0);
        const Frame3D f(
            Point3D::origin(),
            Vector3D(c,  s, 0.0),  // X' at +45°
            Vector3D(-s, c, 0.0)   // Y' at +135°
        );
        CHECK(f.isOrthonormal());
        checkVector(f.axisZ(), Vector3D::unitZ());
    }

    // ==========================================================================
    // isOrthonormal
    // ==========================================================================

    TEST_CASE("isOrthonormal: default frame passes") {
        CHECK(Frame3D().isOrthonormal());
    }

    TEST_CASE("isOrthonormal: frame from unit axes passes") {
        const Frame3D f(Point3D(5.0, 3.0, -2.0),
                        Vector3D::unitX(), Vector3D::unitY(), Vector3D::unitZ());
        CHECK(f.isOrthonormal());
    }

    // ==========================================================================
    // worldToLocal / localToWorld — identity (world) frame
    // ==========================================================================

    TEST_CASE("worldToLocal: world frame is identity for points") {
        const Frame3D world;
        const Point3D p(3.0, 4.0, 5.0);
        checkPoint(world.worldToLocal(p), p);
    }

    TEST_CASE("localToWorld: world frame is identity for points") {
        const Frame3D world;
        const Point3D p(3.0, 4.0, 5.0);
        checkPoint(world.localToWorld(p), p);
    }

    TEST_CASE("worldToLocal: world frame is identity for vectors") {
        const Frame3D world;
        const Vector3D v(1.0, -2.0, 3.0);
        checkVector(world.worldToLocal(v), v);
    }

    TEST_CASE("localToWorld: world frame is identity for vectors") {
        const Frame3D world;
        const Vector3D v(1.0, -2.0, 3.0);
        checkVector(world.localToWorld(v), v);
    }

    // ==========================================================================
    // worldToLocal / localToWorld — translated frame (no rotation)
    // ==========================================================================

    TEST_CASE("worldToLocal: frame at (1,2,3) subtracts the offset") {
        /**
         * A frame whose origin is (1,2,3) but axes are world-aligned.
         * World point (4,6,8) is at local coordinates (3,4,5) — just the
         * displacement from the frame origin.
         */
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D::unitX(), Vector3D::unitY(), Vector3D::unitZ()
        );
        checkPoint(f.worldToLocal(Point3D(4.0, 6.0, 8.0)), Point3D(3.0, 4.0, 5.0));
    }

    TEST_CASE("localToWorld: frame at (1,2,3) adds the offset") {
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D::unitX(), Vector3D::unitY(), Vector3D::unitZ()
        );
        checkPoint(f.localToWorld(Point3D(3.0, 4.0, 5.0)), Point3D(4.0, 6.0, 8.0));
    }

    // ==========================================================================
    // worldToLocal / localToWorld — 90° rotated frame
    // ==========================================================================

    TEST_CASE("worldToLocal: 90-degree rotation — local X points in world Y") {
        /**
         * Frame where:
         *   local X = world Y  (0,1,0)
         *   local Y = world -X (-1,0,0)
         *   local Z = world Z  (0,0,1)
         *
         * A world point at (1,0,0) (one unit along world X) should appear
         * at local (0,-1,0) — one unit along local -Y.
         */
        const Frame3D f(
            Point3D::origin(),
            Vector3D(0.0,  1.0, 0.0),   // local X = world Y
            Vector3D(-1.0, 0.0, 0.0),   // local Y = world -X
            Vector3D(0.0,  0.0, 1.0)
        );
        const Point3D local = f.worldToLocal(Point3D(1.0, 0.0, 0.0));
        checkPoint(local, Point3D(0.0, -1.0, 0.0));
    }

    TEST_CASE("localToWorld: 90-degree rotation — local (0,1,0) is world (-1,0,0)") {
        const Frame3D f(
            Point3D::origin(),
            Vector3D(0.0,  1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0,  0.0, 1.0)
        );
        checkPoint(f.localToWorld(Point3D(0.0, 1.0, 0.0)), Point3D(-1.0, 0.0, 0.0));
    }

    // ==========================================================================
    // Round-trip consistency
    // ==========================================================================

    TEST_CASE("Round-trip: localToWorld(worldToLocal(P)) == P") {
        /**
         * For any invertible transformation, applying it and then its inverse
         * must return the original point.  This tests both the rotation and
         * translation components.
         */
        const double angle = std::numbers::pi / 4.0; // 45°
        const Frame3D f(
            Point3D(5.0, 3.0, -2.0),
            Vector3D(std::cos(angle), std::sin(angle), 0.0),
            Vector3D(-std::sin(angle), std::cos(angle), 0.0),
            Vector3D::unitZ()
        );
        const Point3D original(7.0, -1.0, 4.0);
        checkPoint(f.localToWorld(f.worldToLocal(original)), original);
    }

    TEST_CASE("Round-trip: worldToLocal(localToWorld(P)) == P") {
        const Frame3D f(
            Point3D(1.0, -2.0, 3.0),
            Vector3D(0.0, 0.0, 1.0),  // local X = world Z
            Vector3D(0.0, 1.0, 0.0),  // local Y = world Y
            Vector3D(-1.0, 0.0, 0.0)  // local Z = world -X
        );
        const Point3D localOrig(2.0, 3.0, 4.0);
        checkPoint(f.worldToLocal(f.localToWorld(localOrig)), localOrig);
    }

    // ==========================================================================
    // Method equivalence: matrix == dot-product
    // ==========================================================================

    TEST_CASE("Matrix and dot-product methods agree on point worldToLocal") {
        /**
         * The matrix method builds the 4×4 inverse transform; the dot-product
         * method projects the displacement onto each axis.  Both must give
         * identical results.
         */
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D(0.0,  1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0,  0.0, 1.0)
        );
        const Point3D p(4.0, 5.0, 6.0);
        checkPoint(f.worldToLocalMatrix(p), f.worldToLocalDotProduct(p));
    }

    TEST_CASE("Matrix and dot-product methods agree on point localToWorld") {
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D(0.0,  1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0,  0.0, 1.0)
        );
        const Point3D p(4.0, 5.0, 6.0);
        checkPoint(f.localToWorldMatrix(p), f.localToWorldDotProduct(p));
    }

    TEST_CASE("Matrix and dot-product methods agree on vector worldToLocal") {
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D(0.0,  1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0,  0.0, 1.0)
        );
        const Vector3D v(1.0, 2.0, 3.0);
        checkVector(f.worldToLocalMatrix(v), f.worldToLocalDotProduct(v));
    }

    TEST_CASE("Matrix and dot-product methods agree on vector localToWorld") {
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D(0.0,  1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0,  0.0, 1.0)
        );
        const Vector3D v(1.0, 2.0, 3.0);
        checkVector(f.localToWorldMatrix(v), f.localToWorldDotProduct(v));
    }

    // ==========================================================================
    // Vector transformations (no translation)
    // ==========================================================================

    TEST_CASE("Vector transformation is not affected by frame translation") {
        /**
         * Vectors represent directions, not positions.  Moving the frame
         * origin must not change how directions are expressed.
         */
        const Frame3D at_origin(
            Point3D::origin(),
            Vector3D::unitX(), Vector3D::unitY(), Vector3D::unitZ()
        );
        const Frame3D at_offset(
            Point3D(100.0, 200.0, 300.0),
            Vector3D::unitX(), Vector3D::unitY(), Vector3D::unitZ()
        );
        const Vector3D v(1.0, 2.0, 3.0);
        checkVector(at_origin.worldToLocal(v), at_offset.worldToLocal(v));
    }

    TEST_CASE("worldToLocal of a world axis vector gives a local basis vector") {
        /**
         * The frame's local X-axis expressed in world coordinates should map
         * to (1,0,0) in local coordinates.
         */
        const double angle = std::numbers::pi / 3.0; // 60°
        const Frame3D f(
            Point3D::origin(),
            Vector3D(std::cos(angle), std::sin(angle), 0.0),
            Vector3D(-std::sin(angle), std::cos(angle), 0.0),
            Vector3D::unitZ()
        );
        checkVector(f.worldToLocal(f.axisX()), Vector3D::unitX());
        checkVector(f.worldToLocal(f.axisY()), Vector3D::unitY());
        checkVector(f.worldToLocal(f.axisZ()), Vector3D::unitZ());
    }

    // ==========================================================================
    // Transformation matrices
    // ==========================================================================

    TEST_CASE("getTransformationMatrix: M * M_inv ≈ identity") {
        /**
         * Multiplying the forward matrix by the inverse should give the 4×4
         * identity matrix (within floating-point precision).
         */
        const Frame3D f(
            Point3D(1.0, 2.0, 3.0),
            Vector3D(0.0, 1.0, 0.0),
            Vector3D(-1.0, 0.0, 0.0),
            Vector3D(0.0, 0.0, 1.0)
        );
        const auto M    = f.getTransformationMatrix();
        const auto Minv = f.getInverseTransformationMatrix();

        // Row-major 4×4 multiply: result[row*4+col] = Σ M[row*4+k]*Minv[k*4+col]
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                double val = 0.0;
                for (int k = 0; k < 4; ++k) {
                    val += M[static_cast<size_t>(row * 4 + k)]
                         * Minv[static_cast<size_t>(k * 4 + col)];
                }
                const double expected = (row == col) ? 1.0 : 0.0;
                CHECK(std::abs(val - expected) < EPS);
            }
        }
    }

    // ==========================================================================
    // Mutators
    // ==========================================================================

    TEST_CASE("setOrigin: updates the origin") {
        Frame3D f;
        f.setOrigin(Point3D(5.0, 6.0, 7.0));
        CHECK(f.origin() == Point3D(5.0, 6.0, 7.0));
    }

    TEST_CASE("setOrigin: does not change the axes") {
        Frame3D f;
        f.setOrigin(Point3D(1.0, 2.0, 3.0));
        checkVector(f.axisX(), Vector3D::unitX());
        checkVector(f.axisY(), Vector3D::unitY());
        checkVector(f.axisZ(), Vector3D::unitZ());
    }

    TEST_CASE("setAxes: normalizes and stores the new axes") {
        Frame3D f;
        f.setAxes(
            Vector3D(3.0, 0.0, 0.0),  // length 3 → normalized to (1,0,0)
            Vector3D(0.0, 4.0, 0.0),  // length 4 → normalized to (0,1,0)
            Vector3D(0.0, 0.0, 5.0)   // length 5 → normalized to (0,0,1)
        );
        checkVector(f.axisX(), Vector3D::unitX());
        checkVector(f.axisY(), Vector3D::unitY());
        checkVector(f.axisZ(), Vector3D::unitZ());
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: output contains 'Frame3D'") {
        std::ostringstream oss;
        oss << Frame3D();
        CHECK(oss.str().find("Frame3D") != std::string::npos);
    }

    TEST_CASE("operator<<: output mentions Origin and axes") {
        std::ostringstream oss;
        oss << Frame3D();
        const std::string s = oss.str();
        CHECK(s.find("Origin") != std::string::npos);
        CHECK(s.find("X-Axis") != std::string::npos);
        CHECK(s.find("Y-Axis") != std::string::npos);
        CHECK(s.find("Z-Axis") != std::string::npos);
    }

} // TEST_SUITE("Frame3D")
