/**
 * @file test_Plane3D.cc
 * @brief Unit tests for geometry::Plane3D.
 *
 * A Plane3D is an infinite plane in 3D space, described either as:
 *   - a point P0 on the plane and a unit normal n, or
 *   - the implicit equation  ax + by + cz + d = 0
 *     where (a,b,c) = n  and  d = -n·P0.
 *
 * Positive signed distance means the query point is on the same side as n;
 * negative means it is on the opposite side.
 *
 * Test groups
 * -----------
 * Factory construction
 *   fromDefault()          — XY plane at z=0; normal=(0,0,1).
 *   fromPointAndNormal()   — stores the normalized normal.
 *   fromThreePoints()      — derives normal from (P2-P1)×(P3-P1).
 *   fromCoefficients()     — back-computes a point on the plane.
 *   Error cases: collinear points, zero normal.
 *
 * Accessors
 *   point(), normal(), d(), coefficients().
 *   d() must satisfy ax+by+cz+d=0 for any point on the plane.
 *
 * Signed distance
 *   Positive above the normal, negative below, zero on the plane.
 *
 * Absolute distance
 *   Always non-negative; equal for symmetric points above/below.
 *
 * Point classification
 *   contains(), isAbove(), isBelow().
 *
 * Projection
 *   projectPoint() — closest point on the plane; must satisfy contains().
 *   projectVector() — removes the normal component; result must be in-plane.
 *
 * Plane relationships
 *   isParallelTo()      — normals are parallel (or anti-parallel).
 *   isPerpendicularTo() — normals are perpendicular.
 *   isCoplanarWith()    — parallel AND share a point.
 *   angleTo()           — dihedral angle in [0, π/2].
 *   distanceTo(Plane3D) — distance between parallel planes, nullopt otherwise.
 *
 * Line intersection
 *   intersectLine()          — returns the intersection point or nullopt.
 *   intersectLineParameter() — returns parameter t or nullopt.
 *
 * Plane transformations
 *   offset()  — parallel plane shifted along the normal.
 *   flipped() — same plane, reversed normal.
 *
 * Static planes
 *   xy(z), xz(y), yz(x) — axis-aligned planes at given offsets.
 *
 * Comparison
 *   operator== / != use isCoplanarWith(), so flipped planes compare equal.
 *
 * Stream output
 *   operator<< emits "Plane3D(ax + by + cz + d = 0)".
 */

#include "geometry/Plane3D.hh"
#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <array>
#include <cmath>
#include <doctest/doctest.h>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>

using namespace geometry;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static constexpr double EPS = 1e-9;

static void checkPoint(const Point3D &a, const Point3D &b, double eps = EPS) {
    CHECK(std::abs(a.x - b.x) < eps);
    CHECK(std::abs(a.y - b.y) < eps);
    CHECK(std::abs(a.z - b.z) < eps);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_SUITE("Plane3D") {

    // ==========================================================================
    // Factory construction
    // ==========================================================================

    TEST_CASE("fromDefault: XY plane — normal=(0,0,1), passes through origin") {
        /**
         * The default plane is the XY plane at z=0.
         * Its normal points in the +Z direction and d=0.
         */
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.normal() == Vector3D::unitZ());
        CHECK(p.contains(Point3D::origin()));
        CHECK(p.d() == doctest::Approx(0.0));
    }

    TEST_CASE("fromPointAndNormal: normalizes the supplied normal") {
        /**
         * A non-unit normal (0,0,5) must be normalized to (0,0,1).
         */
        const Plane3D p = Plane3D::fromPointAndNormal(Point3D::origin(), Vector3D(0.0, 0.0, 5.0));
        CHECK(p.normal() == Vector3D::unitZ());
    }

    TEST_CASE("fromPointAndNormal: supplied point lies on the plane") {
        const Point3D pt(1.0, 2.0, 3.0);
        const Plane3D p = Plane3D::fromPointAndNormal(pt, Vector3D::unitZ());
        CHECK(p.contains(pt));
    }

    TEST_CASE("fromThreePoints: normal computed via right-hand rule") {
        /**
         * Three points in the XY plane: (0,0,0), (1,0,0), (0,1,0).
         * (P2-P1) × (P3-P1) = (1,0,0)×(0,1,0) = (0,0,1).
         * So normal must be +Z.
         */
        const Point3D p1(0.0, 0.0, 0.0);
        const Point3D p2(1.0, 0.0, 0.0);
        const Point3D p3(0.0, 1.0, 0.0);
        const Plane3D plane = Plane3D::fromThreePoints(p1, p2, p3);
        CHECK(plane.normal() == Vector3D::unitZ());
    }

    TEST_CASE("fromThreePoints: all three points lie on the resulting plane") {
        const Point3D p1(1.0, 0.0, 0.0);
        const Point3D p2(0.0, 1.0, 0.0);
        const Point3D p3(0.0, 0.0, 1.0);
        const Plane3D plane = Plane3D::fromThreePoints(p1, p2, p3);
        CHECK(plane.contains(p1));
        CHECK(plane.contains(p2));
        CHECK(plane.contains(p3));
    }

    TEST_CASE("fromThreePoints: collinear points throw std::invalid_argument") {
        /**
         * Three points on the same line form a degenerate triangle whose
         * cross-product is the zero vector — no unique plane exists.
         */
        const Point3D p1(0.0, 0.0, 0.0);
        const Point3D p2(1.0, 0.0, 0.0);
        const Point3D p3(2.0, 0.0, 0.0); // collinear
        CHECK_THROWS_AS(std::ignore = Plane3D::fromThreePoints(p1, p2, p3), std::invalid_argument);
    }

    TEST_CASE("fromCoefficients: round-trips through coefficients()") {
        /**
         * Plane x+y+z=1 has coefficients (1,1,1,-1).
         * After constructing from those coefficients, extracting them again,
         * and building a second plane, both must be geometrically equal.
         */
        const Plane3D plane1 = Plane3D::fromCoefficients(1.0, 1.0, 1.0, -1.0);
        const auto coeff = plane1.coefficients();
        const Plane3D plane2 = Plane3D::fromCoefficients(coeff[0], coeff[1], coeff[2], coeff[3]);
        CHECK(plane1 == plane2);
    }

    TEST_CASE("fromCoefficients: zero normal throws std::invalid_argument") {
        CHECK_THROWS_AS(std::ignore = Plane3D::fromCoefficients(0.0, 0.0, 0.0, 1.0),
                        std::invalid_argument);
    }

    // ==========================================================================
    // Accessors
    // ==========================================================================

    TEST_CASE("d(): XY plane at z=5 has d=-5") {
        /**
         * Plane equation for z=5: 0*x + 0*y + 1*z - 5 = 0  →  d = -5.
         */
        const Plane3D p = Plane3D::fromPointAndNormal(Point3D(0.0, 0.0, 5.0), Vector3D::unitZ());
        CHECK(p.d() == doctest::Approx(-5.0));
    }

    TEST_CASE("coefficients(): substituting any on-plane point gives 0") {
        /**
         * For a valid plane, ax+by+cz+d must be 0 for every point on the plane.
         */
        const Plane3D p = Plane3D::fromPointAndNormal(Point3D(0.0, 0.0, 5.0), Vector3D::unitZ());
        const auto coeff = p.coefficients();
        const Point3D onPlane(3.0, 7.0, 5.0);
        const double val =
            coeff[0] * onPlane.x + coeff[1] * onPlane.y + coeff[2] * onPlane.z + coeff[3];
        CHECK(val == doctest::Approx(0.0));
    }

    TEST_CASE("normal(): is always a unit vector") {
        const Plane3D p = Plane3D::fromCoefficients(3.0, 4.0, 0.0, -5.0);
        CHECK(p.normal().length() == doctest::Approx(1.0));
    }

    // ==========================================================================
    // Signed distance
    // ==========================================================================

    TEST_CASE("signedDistanceTo: positive on the normal side") {
        /**
         * XY plane at z=0; a point at z=3 is 3 units above (normal side).
         */
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.signedDistanceTo(Point3D(0.0, 0.0, 3.0)) == doctest::Approx(3.0));
    }

    TEST_CASE("signedDistanceTo: negative on the opposite side") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.signedDistanceTo(Point3D(0.0, 0.0, -4.0)) == doctest::Approx(-4.0));
    }

    TEST_CASE("signedDistanceTo: zero for a point on the plane") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.signedDistanceTo(Point3D(5.0, -3.0, 0.0)) == doctest::Approx(0.0));
    }

    TEST_CASE("signedDistanceTo: tilted plane") {
        /**
         * Plane through (1,0,0) with normal (1,0,0) (YZ plane at x=1).
         * Point at (4,0,0) is 3 units on the positive side.
         */
        const Plane3D p = Plane3D::fromPointAndNormal(Point3D(1.0, 0.0, 0.0), Vector3D::unitX());
        CHECK(p.signedDistanceTo(Point3D(4.0, 0.0, 0.0)) == doctest::Approx(3.0));
        CHECK(p.signedDistanceTo(Point3D(-2.0, 0.0, 0.0)) == doctest::Approx(-3.0));
    }

    // ==========================================================================
    // Absolute distance
    // ==========================================================================

    TEST_CASE("distanceTo(Point3D): always non-negative") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.distanceTo(Point3D(0.0, 0.0, 3.0)) == doctest::Approx(3.0));
        CHECK(p.distanceTo(Point3D(0.0, 0.0, -3.0)) == doctest::Approx(3.0));
    }

    TEST_CASE("distanceTo(Point3D): zero for point on plane") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.distanceTo(Point3D(2.0, 7.0, 0.0)) == doctest::Approx(0.0));
    }

    // ==========================================================================
    // Point classification
    // ==========================================================================

    TEST_CASE("contains: true for a point on the plane") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.contains(Point3D(1.0, 2.0, 0.0)));
    }

    TEST_CASE("contains: false for a point slightly off the plane") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK_FALSE(p.contains(Point3D(0.0, 0.0, 0.001)));
    }

    TEST_CASE("contains: respects custom tolerance") {
        const Plane3D p = Plane3D::fromDefault();
        // 0.001 is outside the default 1e-10 tolerance but within 0.01
        CHECK(p.contains(Point3D(0.0, 0.0, 0.001), 0.01));
    }

    TEST_CASE("isAbove: true only for points on the normal side") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.isAbove(Point3D(0.0, 0.0, 1.0)));
        CHECK_FALSE(p.isAbove(Point3D(0.0, 0.0, -1.0)));
        CHECK_FALSE(p.isAbove(Point3D(0.0, 0.0, 0.0))); // on plane → not above
    }

    TEST_CASE("isBelow: true only for points on the anti-normal side") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.isBelow(Point3D(0.0, 0.0, -1.0)));
        CHECK_FALSE(p.isBelow(Point3D(0.0, 0.0, 1.0)));
        CHECK_FALSE(p.isBelow(Point3D(0.0, 0.0, 0.0))); // on plane → not below
    }

    // ==========================================================================
    // Projection
    // ==========================================================================

    TEST_CASE("projectPoint: result lies on the plane") {
        /**
         * Projecting (3,4,5) onto the XY plane (z=0) gives (3,4,0).
         * The projection must satisfy contains().
         */
        const Plane3D p = Plane3D::fromDefault();
        const Point3D proj = p.projectPoint(Point3D(3.0, 4.0, 5.0));
        checkPoint(proj, Point3D(3.0, 4.0, 0.0));
        CHECK(p.contains(proj));
    }

    TEST_CASE("projectPoint: point already on plane is unchanged") {
        const Plane3D p = Plane3D::fromDefault();
        const Point3D on(2.0, 3.0, 0.0);
        checkPoint(p.projectPoint(on), on);
    }

    TEST_CASE("projectPoint: distance to plane is 0 after projection") {
        const Plane3D p = Plane3D::fromPointAndNormal(Point3D(0.0, 0.0, 2.0), Vector3D::unitZ());
        const Point3D proj = p.projectPoint(Point3D(7.0, -5.0, 9.0));
        CHECK(p.distanceTo(proj) == doctest::Approx(0.0));
    }

    TEST_CASE("projectVector: removes the normal component") {
        /**
         * Projecting (0,0,1) onto the XY plane gives (0,0,0) because the
         * vector is entirely in the normal direction.
         */
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.projectVector(Vector3D::unitZ()).isZero());
    }

    TEST_CASE("projectVector: in-plane vector is unchanged") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.projectVector(Vector3D::unitX()) == Vector3D::unitX());
        CHECK(p.projectVector(Vector3D::unitY()) == Vector3D::unitY());
    }

    TEST_CASE("projectVector: general diagonal vector") {
        /**
         * (1,0,1) projected onto XY plane:
         *   component along Z (normal) = 1
         *   result = (1,0,1) - 1*(0,0,1) = (1,0,0)
         */
        const Plane3D p = Plane3D::fromDefault();
        const Vector3D proj = p.projectVector(Vector3D(1.0, 0.0, 1.0));
        CHECK(proj.x == doctest::Approx(1.0));
        CHECK(proj.y == doctest::Approx(0.0));
        CHECK(proj.z == doctest::Approx(0.0));
    }

    // ==========================================================================
    // Plane relationships
    // ==========================================================================

    TEST_CASE("isParallelTo: same-normal planes at different heights") {
        const Plane3D p1 = Plane3D::xy(0.0);
        const Plane3D p2 = Plane3D::xy(5.0);
        CHECK(p1.isParallelTo(p2));
    }

    TEST_CASE("isParallelTo: flipped-normal planes are still parallel") {
        /**
         * Parallel includes anti-parallel normals — the planes have the same
         * orientation even if the normal directions are opposite.
         */
        CHECK(Plane3D::xy(0.0).isParallelTo(Plane3D::xy(0.0).flipped()));
    }

    TEST_CASE("isParallelTo: perpendicular planes are not parallel") {
        CHECK_FALSE(Plane3D::xy().isParallelTo(Plane3D::xz()));
    }

    TEST_CASE("isPerpendicularTo: XY and XZ planes") {
        /**
         * The XY plane (normal Z) and XZ plane (normal Y) have perpendicular
         * normals: Z·Y = 0.
         */
        CHECK(Plane3D::xy().isPerpendicularTo(Plane3D::xz()));
        CHECK(Plane3D::xy().isPerpendicularTo(Plane3D::yz()));
        CHECK(Plane3D::xz().isPerpendicularTo(Plane3D::yz()));
    }

    TEST_CASE("isPerpendicularTo: parallel planes are not perpendicular") {
        CHECK_FALSE(Plane3D::xy(0.0).isPerpendicularTo(Plane3D::xy(5.0)));
    }

    TEST_CASE("isCoplanarWith: a plane is coplanar with itself") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.isCoplanarWith(p));
    }

    TEST_CASE("isCoplanarWith: parallel but distinct planes are not coplanar") {
        CHECK_FALSE(Plane3D::xy(0.0).isCoplanarWith(Plane3D::xy(1.0)));
    }

    TEST_CASE("isCoplanarWith: flipped plane is still coplanar") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.isCoplanarWith(p.flipped()));
    }

    TEST_CASE("angleTo: perpendicular planes → π/2") {
        /**
         * The dihedral angle between the XY and XZ planes is 90°.
         * angleTo() returns the angle in [0, π/2].
         */
        CHECK(Plane3D::xy().angleTo(Plane3D::xz()) == doctest::Approx(std::numbers::pi / 2.0));
    }

    TEST_CASE("angleTo: parallel planes → 0") {
        CHECK(Plane3D::xy(0.0).angleTo(Plane3D::xy(5.0)) == doctest::Approx(0.0));
    }

    TEST_CASE("angleTo: anti-parallel normals → 0 (same geometric angle)") {
        /**
         * angleTo() returns |arccos(|n1·n2|)| which clamps anti-parallel to 0.
         */
        CHECK(Plane3D::xy(0.0).angleTo(Plane3D::xy(0.0).flipped()) == doctest::Approx(0.0));
    }

    TEST_CASE("distanceTo(Plane3D): parallel planes at distance 3") {
        const Plane3D p1 = Plane3D::xy(0.0);
        const Plane3D p2 = Plane3D::xy(3.0);
        const auto dist = p1.distanceTo(p2);
        REQUIRE(dist.has_value());
        CHECK(dist.value() == doctest::Approx(3.0));
    }

    TEST_CASE("distanceTo(Plane3D): coplanar planes have distance 0") {
        const Plane3D p = Plane3D::fromDefault();
        const auto dist = p.distanceTo(p);
        REQUIRE(dist.has_value());
        CHECK(dist.value() == doctest::Approx(0.0));
    }

    TEST_CASE("distanceTo(Plane3D): non-parallel planes return nullopt") {
        const auto dist = Plane3D::xy().distanceTo(Plane3D::xz());
        CHECK_FALSE(dist.has_value());
    }

    // ==========================================================================
    // Line intersection
    // ==========================================================================

    TEST_CASE("intersectLine: vertical ray hits XY plane at origin") {
        /**
         * Ray from (0,0,5) in direction (0,0,-1).
         * Should hit the XY plane at (0,0,0).
         */
        const Plane3D plane = Plane3D::fromDefault();
        const auto hit = plane.intersectLine(Point3D(0.0, 0.0, 5.0), Vector3D(0.0, 0.0, -1.0));
        REQUIRE(hit.has_value());
        checkPoint(hit.value(), Point3D(0.0, 0.0, 0.0));
    }

    TEST_CASE("intersectLine: tilted line hits at expected point") {
        /**
         * Line: P=(0,0,1), dir=(1,0,-1).  XY plane at z=0.
         * Parameter t: 1 - t = 0  →  t = 1.
         * Hit point: (0+1, 0, 1-1) = (1,0,0).
         */
        const Plane3D plane = Plane3D::fromDefault();
        const auto hit = plane.intersectLine(Point3D(0.0, 0.0, 1.0), Vector3D(1.0, 0.0, -1.0));
        REQUIRE(hit.has_value());
        checkPoint(hit.value(), Point3D(1.0, 0.0, 0.0));
    }

    TEST_CASE("intersectLine: hit point lies on the plane") {
        const Plane3D plane =
            Plane3D::fromPointAndNormal(Point3D(0.0, 0.0, 3.0), Vector3D::unitZ());
        const auto hit = plane.intersectLine(Point3D(0.0, 0.0, 0.0), Vector3D(1.0, 1.0, 1.0));
        REQUIRE(hit.has_value());
        CHECK(plane.contains(hit.value()));
    }

    TEST_CASE("intersectLine: parallel line returns nullopt") {
        /**
         * A line traveling in the XY plane (direction has no Z component)
         * never intersects the XY plane (unless it lies within it, but
         * the implementation treats that as parallel → nullopt).
         */
        const Plane3D plane = Plane3D::fromDefault();
        const auto hit = plane.intersectLine(Point3D(0.0, 0.0, 1.0), Vector3D(1.0, 0.0, 0.0));
        CHECK_FALSE(hit.has_value());
    }

    TEST_CASE("intersectLineParameter: correct t for vertical ray") {
        /**
         * Ray from (0,0,2) in direction (0,0,-1).
         * t = n·(P0-linePoint) / n·dir = (0,0,1)·(0,0,-2) / (0,0,1)·(0,0,-1) = -2 / -1 = 2.
         */
        const Plane3D plane = Plane3D::fromDefault();
        const auto t =
            plane.intersectLineParameter(Point3D(0.0, 0.0, 2.0), Vector3D(0.0, 0.0, -1.0));
        REQUIRE(t.has_value());
        CHECK(t.value() == doctest::Approx(2.0));
    }

    TEST_CASE("intersectLineParameter: parallel line returns nullopt") {
        const Plane3D plane = Plane3D::fromDefault();
        const auto t =
            plane.intersectLineParameter(Point3D(0.0, 0.0, 1.0), Vector3D(1.0, 0.0, 0.0));
        CHECK_FALSE(t.has_value());
    }

    // ==========================================================================
    // Plane transformations
    // ==========================================================================

    TEST_CASE("offset: moves the plane by the given distance along normal") {
        /**
         * XY plane at z=0 offset by 3 units should pass through (0,0,3).
         */
        const Plane3D p = Plane3D::fromDefault();
        const Plane3D q = p.offset(3.0);
        CHECK(q.contains(Point3D(0.0, 0.0, 3.0)));
        CHECK(q.normal() == Vector3D::unitZ());
    }

    TEST_CASE("offset: negative distance moves in opposite direction") {
        const Plane3D p = Plane3D::fromDefault();
        const Plane3D q = p.offset(-2.0);
        CHECK(q.contains(Point3D(0.0, 0.0, -2.0)));
    }

    TEST_CASE("offset then negative offset recovers original plane") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p == p.offset(5.0).offset(-5.0));
    }

    TEST_CASE("flipped: reverses the normal direction") {
        const Plane3D p = Plane3D::fromDefault();
        const Plane3D f = p.flipped();
        CHECK(f.normal() == -Vector3D::unitZ());
    }

    TEST_CASE("flipped: resulting plane is geometrically the same plane") {
        /**
         * operator== uses isCoplanarWith(), which only checks whether the
         * planes share all points — not whether the normals match.
         */
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p == p.flipped());
    }

    TEST_CASE("flipped: double flip recovers original orientation") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p.flipped().flipped().normal() == p.normal());
    }

    // ==========================================================================
    // Static axis-aligned planes
    // ==========================================================================

    TEST_CASE("Plane3D::xy: contains points with the specified z coordinate") {
        const Plane3D p = Plane3D::xy(2.0);
        CHECK(p.contains(Point3D(3.0, 4.0, 2.0)));
        CHECK_FALSE(p.contains(Point3D(0.0, 0.0, 0.0)));
        CHECK(p.normal() == Vector3D::unitZ());
    }

    TEST_CASE("Plane3D::xz: contains points with the specified y coordinate") {
        const Plane3D p = Plane3D::xz(1.0);
        CHECK(p.contains(Point3D(3.0, 1.0, 4.0)));
        CHECK(p.normal() == Vector3D::unitY());
    }

    TEST_CASE("Plane3D::yz: contains points with the specified x coordinate") {
        const Plane3D p = Plane3D::yz(2.0);
        CHECK(p.contains(Point3D(2.0, 5.0, 6.0)));
        CHECK(p.normal() == Vector3D::unitX());
    }

    TEST_CASE("Plane3D::xy, xz, yz are mutually perpendicular") {
        CHECK(Plane3D::xy().isPerpendicularTo(Plane3D::xz()));
        CHECK(Plane3D::xy().isPerpendicularTo(Plane3D::yz()));
        CHECK(Plane3D::xz().isPerpendicularTo(Plane3D::yz()));
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: same plane from different factories") {
        /**
         * The XY plane at z=0 created with fromDefault() and with
         * fromCoefficients(0,0,1,0) must compare equal.
         */
        const Plane3D p1 = Plane3D::fromDefault();
        const Plane3D p2 = Plane3D::fromCoefficients(0.0, 0.0, 1.0, 0.0);
        CHECK(p1 == p2);
    }

    TEST_CASE("operator!=: parallel planes at different heights") {
        CHECK(Plane3D::xy(0.0) != Plane3D::xy(1.0));
    }

    TEST_CASE("operator==: a plane equals itself") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p == p);
    }

    TEST_CASE("operator==: flipped plane equals original") {
        const Plane3D p = Plane3D::fromDefault();
        CHECK(p == p.flipped());
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: output starts with 'Plane3D('") {
        std::ostringstream oss;
        oss << Plane3D::fromDefault();
        CHECK(oss.str().find("Plane3D(") != std::string::npos);
    }

    TEST_CASE("operator<<: output contains '= 0'") {
        std::ostringstream oss;
        oss << Plane3D::fromDefault();
        CHECK(oss.str().find("= 0") != std::string::npos);
    }

} // TEST_SUITE("Plane3D")
