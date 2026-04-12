/**
 * @file test_Vector3D.cc
 * @brief Unit tests for geometry::Vector3D.
 *
 * A Vector3D has direction and magnitude.  It differs from Point3D in that
 * translation does not apply to vectors — only rotation/scaling.
 *
 * Test groups
 * -----------
 * Construction
 *   Default constructor yields the zero vector (0,0,0).
 *   Component-wise constructor stores x/y/z exactly.
 *   Two-point constructor computes the displacement vector P2 - P1.
 *
 * Static factory helpers
 *   zero(), unitX(), unitY(), unitZ() return the expected constant vectors.
 *
 * Arithmetic operators
 *   +, -, unary -, *, / (scalar on both sides), compound assignments.
 *   Division by a near-zero scalar must throw std::invalid_argument.
 *
 * Comparison
 *   == / != use an internal epsilon of 1e-10.
 *
 * Dot product  (a · b = |a||b|cos θ)
 *   Perpendicular unit vectors → 0.
 *   Parallel unit vectors → 1.
 *   General case verified by hand.
 *
 * Cross product  (a × b = |a||b|sin θ · n̂)
 *   Right-hand rule: unitX × unitY = unitZ.
 *   Anti-commutativity: a × b = -(b × a).
 *   Parallel vectors → zero vector.
 *   General case verified by hand.
 *
 * Length / magnitude
 *   length() of a unit vector is 1.
 *   3-4-0 Pythagorean triple → length 5.
 *   lengthSquared() skips the sqrt.
 *
 * Normalisation
 *   normalized() returns a unit vector without modifying the original.
 *   normalize() modifies in place.
 *   Both throw std::invalid_argument for the zero vector.
 *   isNormalized() / isZero() query helpers.
 *
 * Angle
 *   angleTo() uses arccos(a·b / |a||b|), clamped for numerical safety.
 *   Perpendicular → π/2, parallel → 0, anti-parallel → π.
 *
 * Projection
 *   projectOnto() decomposes this vector along another.
 *   Projecting onto a perpendicular axis gives the zero vector.
 *
 * Stream output
 *   operator<< emits "Vector3D(x, y, z)".
 */

#include <doctest/doctest.h>

#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <cmath>
#include <numbers>
#include <sstream>
#include <stdexcept>

using namespace geometry;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/** Approximate equality for doubles. */
static bool approxEq(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_SUITE("Vector3D") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("Default constructor produces the zero vector") {
        /**
         * The zero vector is the additive identity: v + zero = v.
         * It is also the only vector with length 0.
         */
        const Vector3D v;
        CHECK(v.x == 0.0);
        CHECK(v.y == 0.0);
        CHECK(v.z == 0.0);
    }

    TEST_CASE("Component constructor stores values") {
        const Vector3D v(1.0, 2.0, 3.0);
        CHECK(v.x == 1.0);
        CHECK(v.y == 2.0);
        CHECK(v.z == 3.0);
    }

    TEST_CASE("Two-point constructor computes displacement P2 - P1") {
        /**
         * The vector from P1=(1,2,3) to P2=(4,6,9) is (3,4,6).
         * This is equivalent to P2 - P1 using the Point3D subtraction operator.
         */
        const Point3D from(1.0, 2.0, 3.0);
        const Point3D to(4.0, 6.0, 9.0);
        const Vector3D v(from, to);
        CHECK(v.x == doctest::Approx(3.0));
        CHECK(v.y == doctest::Approx(4.0));
        CHECK(v.z == doctest::Approx(6.0));
    }

    // ==========================================================================
    // Static factory helpers
    // ==========================================================================

    TEST_CASE("zero() is the zero vector") {
        const Vector3D v = Vector3D::zero();
        CHECK(v.x == 0.0);
        CHECK(v.y == 0.0);
        CHECK(v.z == 0.0);
    }

    TEST_CASE("unitX() is (1,0,0)") {
        const Vector3D v = Vector3D::unitX();
        CHECK(v.x == 1.0);
        CHECK(v.y == 0.0);
        CHECK(v.z == 0.0);
    }

    TEST_CASE("unitY() is (0,1,0)") {
        const Vector3D v = Vector3D::unitY();
        CHECK(v.x == 0.0);
        CHECK(v.y == 1.0);
        CHECK(v.z == 0.0);
    }

    TEST_CASE("unitZ() is (0,0,1)") {
        const Vector3D v = Vector3D::unitZ();
        CHECK(v.x == 0.0);
        CHECK(v.y == 0.0);
        CHECK(v.z == 1.0);
    }

    // ==========================================================================
    // Arithmetic operators
    // ==========================================================================

    TEST_CASE("operator+: component-wise addition") {
        const Vector3D a(1.0, 2.0, 3.0);
        const Vector3D b(4.0, 5.0, 6.0);
        const Vector3D c = a + b;
        CHECK(c.x == doctest::Approx(5.0));
        CHECK(c.y == doctest::Approx(7.0));
        CHECK(c.z == doctest::Approx(9.0));
    }

    TEST_CASE("operator-: component-wise subtraction") {
        const Vector3D a(4.0, 5.0, 6.0);
        const Vector3D b(1.0, 2.0, 3.0);
        const Vector3D c = a - b;
        CHECK(c.x == doctest::Approx(3.0));
        CHECK(c.y == doctest::Approx(3.0));
        CHECK(c.z == doctest::Approx(3.0));
    }

    TEST_CASE("unary operator-: negation flips all components") {
        const Vector3D a(1.0, -2.0, 3.0);
        const Vector3D b = -a;
        CHECK(b.x == doctest::Approx(-1.0));
        CHECK(b.y == doctest::Approx(2.0));
        CHECK(b.z == doctest::Approx(-3.0));
    }

    TEST_CASE("operator* (vector * scalar): scales all components") {
        const Vector3D a(1.0, 2.0, 3.0);
        const Vector3D b = a * 3.0;
        CHECK(b.x == doctest::Approx(3.0));
        CHECK(b.y == doctest::Approx(6.0));
        CHECK(b.z == doctest::Approx(9.0));
    }

    TEST_CASE("operator* (scalar * vector): commutative friend overload") {
        const Vector3D a(1.0, 2.0, 3.0);
        const Vector3D b = 3.0 * a;
        CHECK(b.x == doctest::Approx(3.0));
        CHECK(b.y == doctest::Approx(6.0));
        CHECK(b.z == doctest::Approx(9.0));
    }

    TEST_CASE("operator/: divides all components by scalar") {
        const Vector3D a(3.0, 6.0, 9.0);
        const Vector3D b = a / 3.0;
        CHECK(b.x == doctest::Approx(1.0));
        CHECK(b.y == doctest::Approx(2.0));
        CHECK(b.z == doctest::Approx(3.0));
    }

    TEST_CASE("operator/ by near-zero throws std::invalid_argument") {
        /**
         * Division by a value with |scalar| < 1e-15 is treated as division
         * by zero to avoid producing Inf/NaN components.
         */
        const Vector3D a(1.0, 0.0, 0.0);
        CHECK_THROWS_AS(a / 1e-16, std::invalid_argument);
    }

    TEST_CASE("operator+=: modifies in place") {
        Vector3D a(1.0, 2.0, 3.0);
        a += Vector3D(1.0, 1.0, 1.0);
        CHECK(a.x == doctest::Approx(2.0));
        CHECK(a.y == doctest::Approx(3.0));
        CHECK(a.z == doctest::Approx(4.0));
    }

    TEST_CASE("operator-=: modifies in place") {
        Vector3D a(3.0, 3.0, 3.0);
        a -= Vector3D(1.0, 2.0, 3.0);
        CHECK(a.x == doctest::Approx(2.0));
        CHECK(a.y == doctest::Approx(1.0));
        CHECK(a.z == doctest::Approx(0.0));
    }

    TEST_CASE("operator*=: modifies in place") {
        Vector3D a(1.0, 2.0, 3.0);
        a *= 2.0;
        CHECK(a.x == doctest::Approx(2.0));
        CHECK(a.y == doctest::Approx(4.0));
        CHECK(a.z == doctest::Approx(6.0));
    }

    TEST_CASE("operator/=: modifies in place") {
        Vector3D a(2.0, 4.0, 6.0);
        a /= 2.0;
        CHECK(a.x == doctest::Approx(1.0));
        CHECK(a.y == doctest::Approx(2.0));
        CHECK(a.z == doctest::Approx(3.0));
    }

    TEST_CASE("operator/= by near-zero throws std::invalid_argument") {
        Vector3D a(1.0, 0.0, 0.0);
        CHECK_THROWS_AS(a /= 0.0, std::invalid_argument);
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: identical vectors compare equal") {
        CHECK(Vector3D(1.0, 2.0, 3.0) == Vector3D(1.0, 2.0, 3.0));
    }

    TEST_CASE("operator==: vectors within 1e-10 compare equal") {
        /**
         * The implementation uses component-wise |a - b| < 1e-10.
         * Values just inside the epsilon should still compare equal.
         */
        constexpr double tiny = 5e-11;
        CHECK(Vector3D(1.0, 1.0, 1.0) == Vector3D(1.0 + tiny, 1.0 + tiny, 1.0 + tiny));
    }

    TEST_CASE("operator!=: clearly different vectors") {
        CHECK(Vector3D(1.0, 0.0, 0.0) != Vector3D(0.0, 1.0, 0.0));
    }

    // ==========================================================================
    // Dot product
    // ==========================================================================

    TEST_CASE("dot: perpendicular unit vectors → 0") {
        /**
         * a · b = |a||b|cos(θ).  For θ = 90°, cos(90°) = 0.
         */
        CHECK(Vector3D::unitX().dot(Vector3D::unitY()) == doctest::Approx(0.0));
        CHECK(Vector3D::unitY().dot(Vector3D::unitZ()) == doctest::Approx(0.0));
        CHECK(Vector3D::unitZ().dot(Vector3D::unitX()) == doctest::Approx(0.0));
    }

    TEST_CASE("dot: unit vector with itself → 1") {
        CHECK(Vector3D::unitX().dot(Vector3D::unitX()) == doctest::Approx(1.0));
    }

    TEST_CASE("dot: general case (1,2,3)·(4,5,6) = 32") {
        /**
         * Manual: 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32.
         */
        CHECK(Vector3D(1.0, 2.0, 3.0).dot(Vector3D(4.0, 5.0, 6.0)) == doctest::Approx(32.0));
    }

    TEST_CASE("dot: commutative — a·b == b·a") {
        const Vector3D a(1.0, 2.0, 3.0);
        const Vector3D b(7.0, -3.0, 0.5);
        CHECK(a.dot(b) == doctest::Approx(b.dot(a)));
    }

    // ==========================================================================
    // Cross product
    // ==========================================================================

    TEST_CASE("cross: unitX × unitY = unitZ  (right-hand rule)") {
        /**
         * In a right-handed coordinate system: X × Y = Z, Y × Z = X, Z × X = Y.
         */
        CHECK(Vector3D::unitX().cross(Vector3D::unitY()) == Vector3D::unitZ());
        CHECK(Vector3D::unitY().cross(Vector3D::unitZ()) == Vector3D::unitX());
        CHECK(Vector3D::unitZ().cross(Vector3D::unitX()) == Vector3D::unitY());
    }

    TEST_CASE("cross: anti-commutative — a×b = -(b×a)") {
        const Vector3D a(1.0, 2.0, 3.0);
        const Vector3D b(4.0, 5.0, 6.0);
        CHECK(a.cross(b) == -(b.cross(a)));
    }

    TEST_CASE("cross: parallel vectors produce zero vector") {
        /**
         * Parallel vectors span a degenerate plane — the cross product
         * has no well-defined perpendicular, so it is zero.
         */
        const Vector3D a(2.0, 0.0, 0.0);
        const Vector3D b(5.0, 0.0, 0.0);
        CHECK(a.cross(b).isZero());
    }

    TEST_CASE("cross: general case (1,2,3)×(4,5,6) = (-3,6,-3)") {
        /**
         * Manual:
         *   x = 2*6 - 3*5 =  12 - 15 = -3
         *   y = 3*4 - 1*6 =  12 -  6 =  6
         *   z = 1*5 - 2*4 =   5 -  8 = -3
         */
        const Vector3D c = Vector3D(1.0, 2.0, 3.0).cross(Vector3D(4.0, 5.0, 6.0));
        CHECK(c.x == doctest::Approx(-3.0));
        CHECK(c.y == doctest::Approx(6.0));
        CHECK(c.z == doctest::Approx(-3.0));
    }

    TEST_CASE("cross result is perpendicular to both operands") {
        const Vector3D a(1.0, 2.0, 0.0);
        const Vector3D b(3.0, 0.0, 4.0);
        const Vector3D c = a.cross(b);
        CHECK(approxEq(c.dot(a), 0.0));
        CHECK(approxEq(c.dot(b), 0.0));
    }

    // ==========================================================================
    // Length / magnitude
    // ==========================================================================

    TEST_CASE("length: unit vector has length 1") {
        CHECK(Vector3D::unitX().length() == doctest::Approx(1.0));
    }

    TEST_CASE("length: 3-4-0 Pythagorean triple → 5") {
        CHECK(Vector3D(3.0, 4.0, 0.0).length() == doctest::Approx(5.0));
    }

    TEST_CASE("length: zero vector has length 0") {
        CHECK(Vector3D::zero().length() == doctest::Approx(0.0));
    }

    TEST_CASE("lengthSquared: (1,2,3) → 14") {
        /**
         * 1² + 2² + 3² = 1 + 4 + 9 = 14.
         * lengthSquared() avoids the sqrt, useful for comparisons.
         */
        CHECK(Vector3D(1.0, 2.0, 3.0).lengthSquared() == doctest::Approx(14.0));
    }

    // ==========================================================================
    // Normalisation
    // ==========================================================================

    TEST_CASE("normalized: produces unit vector, original unchanged") {
        const Vector3D v(3.0, 4.0, 0.0); // length 5
        const Vector3D n = v.normalized();
        CHECK(n.length() == doctest::Approx(1.0));
        CHECK(n.x == doctest::Approx(0.6));
        CHECK(n.y == doctest::Approx(0.8));
        CHECK(n.z == doctest::Approx(0.0));
        // Original must be unchanged
        CHECK(v.x == 3.0);
    }

    TEST_CASE("normalize: modifies vector in place") {
        Vector3D v(0.0, 0.0, 5.0);
        v.normalize();
        CHECK(v == Vector3D::unitZ());
    }

    TEST_CASE("normalized: throws for zero vector") {
        CHECK_THROWS_AS(Vector3D::zero().normalized(), std::invalid_argument);
    }

    TEST_CASE("normalize: throws for zero vector") {
        Vector3D v;
        CHECK_THROWS_AS(v.normalize(), std::invalid_argument);
    }

    TEST_CASE("isNormalized: true for unit vectors") {
        CHECK(Vector3D::unitX().isNormalized());
        CHECK(Vector3D::unitY().isNormalized());
        CHECK(Vector3D::unitZ().isNormalized());
    }

    TEST_CASE("isNormalized: false for non-unit vectors") {
        CHECK_FALSE(Vector3D(2.0, 0.0, 0.0).isNormalized());
        CHECK_FALSE(Vector3D::zero().isNormalized());
    }

    // ==========================================================================
    // isZero
    // ==========================================================================

    TEST_CASE("isZero: true for zero vector") {
        CHECK(Vector3D::zero().isZero());
    }

    TEST_CASE("isZero: true for near-zero vector (within default tolerance)") {
        CHECK(Vector3D(1e-11, 1e-11, 1e-11).isZero());
    }

    TEST_CASE("isZero: false for non-zero vector") {
        CHECK_FALSE(Vector3D::unitX().isZero());
        CHECK_FALSE(Vector3D(1e-5, 0.0, 0.0).isZero());
    }

    // ==========================================================================
    // Angle between vectors
    // ==========================================================================

    TEST_CASE("angleTo: perpendicular vectors → π/2") {
        /**
         * θ = arccos(a·b / |a||b|).  For perpendicular unit vectors the dot
         * product is 0, so arccos(0) = π/2.
         */
        const double angle = Vector3D::unitX().angleTo(Vector3D::unitY());
        CHECK(angle == doctest::Approx(std::numbers::pi / 2.0));
    }

    TEST_CASE("angleTo: parallel vectors → 0") {
        CHECK(Vector3D::unitX().angleTo(Vector3D::unitX()) == doctest::Approx(0.0));
    }

    TEST_CASE("angleTo: anti-parallel vectors → π") {
        const double angle = Vector3D::unitX().angleTo(-Vector3D::unitX());
        CHECK(angle == doctest::Approx(std::numbers::pi));
    }

    TEST_CASE("angleTo: symmetric — a.angleTo(b) == b.angleTo(a)") {
        const Vector3D a(1.0, 2.0, 0.0);
        const Vector3D b(3.0, -1.0, 0.0);
        CHECK(a.angleTo(b) == doctest::Approx(b.angleTo(a)));
    }

    // ==========================================================================
    // Projection
    // ==========================================================================

    TEST_CASE("projectOnto: projecting onto self returns same vector") {
        const Vector3D v(3.0, 0.0, 0.0);
        CHECK(v.projectOnto(Vector3D::unitX()) == v);
    }

    TEST_CASE("projectOnto: projecting onto perpendicular axis returns zero") {
        /**
         * (0,1,0) has no component along (1,0,0), so the projection is zero.
         */
        const Vector3D result = Vector3D::unitY().projectOnto(Vector3D::unitX());
        CHECK(result.isZero());
    }

    TEST_CASE("projectOnto: general case") {
        /**
         * (1,1,0) projected onto unitX:
         *   proj = ((1,1,0)·(1,0,0) / |(1,0,0)|²) * (1,0,0) = 1 * (1,0,0) = (1,0,0)
         */
        const Vector3D proj = Vector3D(1.0, 1.0, 0.0).projectOnto(Vector3D::unitX());
        CHECK(proj.x == doctest::Approx(1.0));
        CHECK(proj.y == doctest::Approx(0.0));
        CHECK(proj.z == doctest::Approx(0.0));
    }

    TEST_CASE("projectOnto: non-unit target vector still gives correct result") {
        /**
         * Projecting (2,0,0) onto (3,0,0):
         *   scalar = (2,0,0)·(3,0,0) / |(3,0,0)|² = 6/9 = 2/3
         *   proj = (2/3)*(3,0,0) = (2,0,0)
         */
        const Vector3D v(2.0, 0.0, 0.0);
        const Vector3D onto(3.0, 0.0, 0.0);
        const Vector3D proj = v.projectOnto(onto);
        CHECK(proj.x == doctest::Approx(2.0));
        CHECK(proj.y == doctest::Approx(0.0));
        CHECK(proj.z == doctest::Approx(0.0));
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: output starts with 'Vector3D('") {
        std::ostringstream oss;
        oss << Vector3D(1.0, 2.0, 3.0);
        const std::string s = oss.str();
        CHECK(s.find("Vector3D(") != std::string::npos);
    }

    TEST_CASE("operator<<: output contains all three component values") {
        std::ostringstream oss;
        oss << Vector3D(1.0, 2.0, 3.0);
        const std::string s = oss.str();
        // Default float formatting: 1.0 → "1", 2.0 → "2", 3.0 → "3"
        CHECK(s.find('1') != std::string::npos);
        CHECK(s.find('2') != std::string::npos);
        CHECK(s.find('3') != std::string::npos);
    }

} // TEST_SUITE("Vector3D")
