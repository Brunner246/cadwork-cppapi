/**
 * @file test_Face.cc
 * @brief Unit tests for geometry::Face.
 *
 * A Face wraps an outer Loop (boundary), zero or more inner Loops (holes),
 * and a support Plane3D. The normal() accessor delegates to the plane.
 *
 * Test groups
 * -----------
 * Construction
 *   Two-argument ctor (outer + plane) leaves holes empty.
 *   Three-argument ctor stores outer, inner loops, and plane.
 *
 * Accessors
 *   outerLoop, innerLoops, supportPlane, normal, innerLoopCount, hasHoles.
 *
 * Comparison
 *   operator== requires matching outer, inner (order-sensitive), and plane.
 *
 * Stream output
 *   operator<< starts with "Face(".
 */

#include "geometry/Face.hh"
#include "geometry/Loop.hh"
#include "geometry/Plane3D.hh"
#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <doctest/doctest.h>
#include <sstream>
#include <vector>

using namespace geometry;

static Loop squareLoop() {
    return Loop({Point3D(0.0, 0.0, 0.0), Point3D(1.0, 0.0, 0.0), Point3D(1.0, 1.0, 0.0),
                 Point3D(0.0, 1.0, 0.0)});
}

static Loop squareHole() {
    return Loop({Point3D(0.25, 0.25, 0.0), Point3D(0.75, 0.25, 0.0), Point3D(0.75, 0.75, 0.0),
                 Point3D(0.25, 0.75, 0.0)});
}

TEST_SUITE("Face") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("two-arg constructor: outer + plane, no holes") {
        const Face face(squareLoop(), Plane3D::xy());
        CHECK(face.innerLoopCount() == 0);
        CHECK_FALSE(face.hasHoles());
        CHECK(face.outerLoop().vertexCount() == 4);
    }

    TEST_CASE("three-arg constructor: outer + holes + plane") {
        const std::vector<Loop> holes{squareHole()};
        const Face face(squareLoop(), holes, Plane3D::xy());
        CHECK(face.innerLoopCount() == 1);
        CHECK(face.hasHoles());
    }

    // ==========================================================================
    // Accessors
    // ==========================================================================

    TEST_CASE("normal(): delegates to supportPlane().normal()") {
        const Face face(squareLoop(), Plane3D::xy());
        CHECK(face.normal() == Vector3D::unitZ());
    }

    TEST_CASE("supportPlane(): returns the stored plane") {
        const Plane3D plane = Plane3D::xy(2.0);
        const Face face(squareLoop(), plane);
        CHECK(face.supportPlane() == plane);
    }

    TEST_CASE("innerLoops(): returns all hole loops") {
        const std::vector<Loop> holes{squareHole(), squareHole()};
        const Face face(squareLoop(), holes, Plane3D::xy());
        CHECK(face.innerLoops().size() == 2);
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: equal outer + same plane + no holes") {
        const Face a(squareLoop(), Plane3D::xy());
        const Face b(squareLoop(), Plane3D::xy());
        CHECK(a == b);
    }

    TEST_CASE("operator!=: differing outer loop") {
        const Face a(squareLoop(), Plane3D::xy());
        const Face b(Loop({Point3D(0.0, 0.0, 0.0)}), Plane3D::xy());
        CHECK(a != b);
    }

    TEST_CASE("operator!=: differing hole count") {
        const Face a(squareLoop(), Plane3D::xy());
        const Face b(squareLoop(), {squareHole()}, Plane3D::xy());
        CHECK(a != b);
    }

    TEST_CASE("operator==: same hole in both faces") {
        const Face a(squareLoop(), {squareHole()}, Plane3D::xy());
        const Face b(squareLoop(), {squareHole()}, Plane3D::xy());
        CHECK(a == b);
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: starts with 'Face('") {
        std::ostringstream oss;
        oss << Face(squareLoop(), Plane3D::xy());
        CHECK(oss.str().find("Face(") != std::string::npos);
    }

    TEST_CASE("operator<<: reports number of holes") {
        std::ostringstream oss;
        oss << Face(squareLoop(), {squareHole(), squareHole()}, Plane3D::xy());
        CHECK(oss.str().find("holes=2") != std::string::npos);
    }

} // TEST_SUITE("Face")
