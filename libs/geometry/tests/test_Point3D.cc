/**
 * @file test_Point3D.cc
 * @brief Unit tests for geometry::Point3D.
 *
 * A Point3D represents an absolute position in 3D space.  Unlike Vector3D,
 * a point does not have a magnitude; instead it participates in these
 * fundamental operations:
 *
 *   Point + Vector  →  Point   (translate a position by a displacement)
 *   Point - Vector  →  Point   (translate in the opposite direction)
 *   Point - Point   →  Vector  (displacement between two positions)
 *
 * Test groups
 * -----------
 * Construction
 *   Default constructor yields the origin (0,0,0).
 *   Component-wise constructor stores x/y/z exactly.
 *   origin() factory returns Point3D(0,0,0).
 *
 * Arithmetic with Vector3D
 *   operator+ / operator- translate the point; verify component results.
 *   operator+= / operator-= modify the point in place.
 *   These operations must NOT change the vector operand.
 *
 * Point difference → Vector
 *   P1 - P2 produces the displacement vector from P2 to P1.
 *   A point minus itself yields the zero vector.
 *
 * Comparison
 *   == / != use an internal epsilon of 1e-10 per component.
 *
 * Distance
 *   distanceTo() is the Euclidean distance, always non-negative.
 *   distanceSquaredTo() avoids the sqrt — faster for comparisons.
 *   Both are symmetric and return 0 for the same point.
 *   The 3-4-0 Pythagorean triple gives distance 5.
 *
 * Stream output
 *   operator<< emits "Point3D(x, y, z)".
 */

#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <doctest/doctest.h>
#include <sstream>


using namespace geometry;

TEST_SUITE("Point3D") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("Default constructor is the origin (0,0,0)") {
        const Point3D p;
        CHECK(p.x == 0.0);
        CHECK(p.y == 0.0);
        CHECK(p.z == 0.0);
    }

    TEST_CASE("Component constructor stores values") {
        const Point3D p(1.0, 2.0, 3.0);
        CHECK(p.x == 1.0);
        CHECK(p.y == 2.0);
        CHECK(p.z == 3.0);
    }

    TEST_CASE("origin() factory returns (0,0,0)") {
        CHECK(Point3D::origin() == Point3D(0.0, 0.0, 0.0));
    }

    // ==========================================================================
    // Arithmetic with Vector3D
    // ==========================================================================

    TEST_CASE("Point + Vector displaces the point") {
        /**
         * Adding a displacement vector to a point shifts it by that amount.
         * P(1,2,3) + V(1,1,1) = P(2,3,4).
         */
        const Point3D p(1.0, 2.0, 3.0);
        const Vector3D v(1.0, 1.0, 1.0);
        const Point3D q = p + v;
        CHECK(q.x == doctest::Approx(2.0));
        CHECK(q.y == doctest::Approx(3.0));
        CHECK(q.z == doctest::Approx(4.0));
    }

    TEST_CASE("Point - Vector displaces in the opposite direction") {
        const Point3D p(3.0, 3.0, 3.0);
        const Vector3D v(1.0, 2.0, 3.0);
        const Point3D q = p - v;
        CHECK(q.x == doctest::Approx(2.0));
        CHECK(q.y == doctest::Approx(1.0));
        CHECK(q.z == doctest::Approx(0.0));
    }

    TEST_CASE("operator+= modifies the point in place") {
        Point3D p(1.0, 1.0, 1.0);
        p += Vector3D(2.0, 3.0, 4.0);
        CHECK(p.x == doctest::Approx(3.0));
        CHECK(p.y == doctest::Approx(4.0));
        CHECK(p.z == doctest::Approx(5.0));
    }

    TEST_CASE("operator-= modifies the point in place") {
        Point3D p(5.0, 5.0, 5.0);
        p -= Vector3D(1.0, 2.0, 3.0);
        CHECK(p.x == doctest::Approx(4.0));
        CHECK(p.y == doctest::Approx(3.0));
        CHECK(p.z == doctest::Approx(2.0));
    }

    TEST_CASE("Arithmetic does not modify the vector operand") {
        const Point3D p(0.0, 0.0, 0.0);
        Vector3D v(1.0, 2.0, 3.0);
        (void)(p + v);
        (void)(p - v);
        // v must be unchanged
        CHECK(v.x == 1.0);
        CHECK(v.y == 2.0);
        CHECK(v.z == 3.0);
    }

    // ==========================================================================
    // Point difference → Vector
    // ==========================================================================

    TEST_CASE("Point - Point yields the displacement vector (P2 - P1)") {
        /**
         * The vector from P1=(1,2,3) to P2=(4,6,8) is (3,4,5).
         * Written as P2 - P1, not P1 - P2.
         */
        const Point3D p1(1.0, 2.0, 3.0);
        const Point3D p2(4.0, 6.0, 8.0);
        const Vector3D v = p2 - p1;
        CHECK(v.x == doctest::Approx(3.0));
        CHECK(v.y == doctest::Approx(4.0));
        CHECK(v.z == doctest::Approx(5.0));
    }

    TEST_CASE("P - P yields the zero vector") {
        const Point3D p(7.0, 8.0, 9.0);
        const Vector3D v = p - p;
        CHECK(v.isZero());
    }

    TEST_CASE("Displacement round-trip: P1 + (P2 - P1) == P2") {
        /**
         * Adding the displacement vector from P1 to P2 back onto P1 must
         * recover P2 exactly (within floating-point precision).
         */
        const Point3D p1(1.0, 2.0, 3.0);
        const Point3D p2(5.0, -1.0, 7.0);
        const Point3D recovered = p1 + (p2 - p1);
        CHECK(recovered == p2);
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: identical points compare equal") {
        CHECK(Point3D(1.0, 2.0, 3.0) == Point3D(1.0, 2.0, 3.0));
    }

    TEST_CASE("operator==: points within 1e-10 compare equal") {
        /**
         * The implementation compares each component with |a - b| < 1e-10.
         */
        constexpr double tiny = 5e-11;
        CHECK(Point3D(0.0, 0.0, 0.0) == Point3D(tiny, tiny, tiny));
    }

    TEST_CASE("operator==: points beyond epsilon compare unequal") {
        constexpr double just_over = 2e-10;
        CHECK_FALSE(Point3D(0.0, 0.0, 0.0) == Point3D(just_over, 0.0, 0.0));
    }

    TEST_CASE("operator!=: clearly different points") {
        CHECK(Point3D(1.0, 0.0, 0.0) != Point3D(0.0, 1.0, 0.0));
    }

    // ==========================================================================
    // Distance
    // ==========================================================================

    TEST_CASE("distanceTo: 3-4-0 Pythagorean triple → 5") {
        /**
         * |(0,0,0) - (3,4,0)| = √(9+16+0) = √25 = 5.
         */
        const Point3D a(0.0, 0.0, 0.0);
        const Point3D b(3.0, 4.0, 0.0);
        CHECK(a.distanceTo(b) == doctest::Approx(5.0));
    }

    TEST_CASE("distanceTo: 3D Pythagorean triple (1,2,2) → 3") {
        /**
         * |(0,0,0) - (1,2,2)| = √(1+4+4) = √9 = 3.
         */
        CHECK(Point3D::origin().distanceTo(Point3D(1.0, 2.0, 2.0)) == doctest::Approx(3.0));
    }

    TEST_CASE("distanceTo: symmetric — a.distanceTo(b) == b.distanceTo(a)") {
        const Point3D a(1.0, 2.0, 3.0);
        const Point3D b(4.0, 5.0, 6.0);
        CHECK(a.distanceTo(b) == doctest::Approx(b.distanceTo(a)));
    }

    TEST_CASE("distanceTo: a point to itself is zero") {
        const Point3D p(5.0, 6.0, 7.0);
        CHECK(p.distanceTo(p) == doctest::Approx(0.0));
    }

    TEST_CASE("distanceSquaredTo: skips sqrt — (1,2,3)→(4,5,6) = 27") {
        /**
         * |Δ|² = 3² + 3² + 3² = 9 + 9 + 9 = 27.
         * Useful for performance-sensitive comparisons where the actual
         * distance is not needed.
         */
        const Point3D a(1.0, 2.0, 3.0);
        const Point3D b(4.0, 5.0, 6.0);
        CHECK(a.distanceSquaredTo(b) == doctest::Approx(27.0));
    }

    TEST_CASE("distanceSquaredTo: consistent with distanceTo") {
        const Point3D a(2.0, 3.0, 4.0);
        const Point3D b(6.0, 6.0, 4.0);
        const double d = a.distanceTo(b);
        CHECK(a.distanceSquaredTo(b) == doctest::Approx(d * d));
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: output starts with 'Point3D('") {
        std::ostringstream oss;
        oss << Point3D(1.0, 2.0, 3.0);
        CHECK(oss.str().find("Point3D(") != std::string::npos);
    }

    TEST_CASE("operator<<: output contains all three component values") {
        std::ostringstream oss;
        oss << Point3D(1.0, 2.0, 3.0);
        const std::string s = oss.str();
        CHECK(s.find('1') != std::string::npos);
        CHECK(s.find('2') != std::string::npos);
        CHECK(s.find('3') != std::string::npos);
    }

} // TEST_SUITE("Point3D")
