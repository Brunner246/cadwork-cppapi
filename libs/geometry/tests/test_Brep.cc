/**
 * @file test_Brep.cc
 * @brief Unit tests for geometry::Brep.
 *
 * A Brep is a flat collection of Face instances. No topological relations
 * (shared edges/vertices) are tracked.
 *
 * Test groups
 * -----------
 * Construction
 *   Default constructor yields an empty Brep.
 *   Vector constructor stores all supplied faces.
 *
 * Accessors & mutators
 *   faceCount, isEmpty, faces, faceAt (with bounds check), addFace.
 *
 * Comparison
 *   operator== compares face lists element-wise.
 *
 * Stream output
 *   operator<< starts with "Brep(" and reports face count.
 */

#include "geometry/Brep.hh"
#include "geometry/Face.hh"
#include "geometry/Loop.hh"
#include "geometry/Plane3D.hh"
#include "geometry/Point3D.hh"
#include "geometry/Vector3D.hh"

#include <doctest/doctest.h>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace geometry;

static Face unitSquareFaceXY(double z = 0.0) {
    const Loop outer({Point3D(0.0, 0.0, z), Point3D(1.0, 0.0, z), Point3D(1.0, 1.0, z),
                      Point3D(0.0, 1.0, z)});
    return Face(outer, Plane3D::xy(z));
}

static Face unitSquareFaceXZ(double y = 0.0) {
    const Loop outer({Point3D(0.0, y, 0.0), Point3D(1.0, y, 0.0), Point3D(1.0, y, 1.0),
                      Point3D(0.0, y, 1.0)});
    return Face(outer, Plane3D::xz(y));
}

TEST_SUITE("Brep") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("default constructor: empty Brep") {
        const Brep brep;
        CHECK(brep.isEmpty());
        CHECK(brep.faceCount() == 0);
    }

    TEST_CASE("vector constructor: stores faces") {
        const std::vector<Face> faces{unitSquareFaceXY(), unitSquareFaceXZ()};
        const Brep brep(faces);
        CHECK(brep.faceCount() == 2);
    }

    // ==========================================================================
    // Accessors & mutators
    // ==========================================================================

    TEST_CASE("addFace: increases faceCount") {
        Brep brep;
        CHECK(brep.faceCount() == 0);
        brep.addFace(unitSquareFaceXY());
        CHECK(brep.faceCount() == 1);
        brep.addFace(unitSquareFaceXZ());
        CHECK(brep.faceCount() == 2);
    }

    TEST_CASE("faceAt: out-of-range throws std::out_of_range") {
        const Brep brep;
        CHECK_THROWS_AS(std::ignore = brep.faceAt(0), std::out_of_range);
    }

    TEST_CASE("faceAt: returns the face at the given index") {
        Brep brep;
        brep.addFace(unitSquareFaceXY(0.0));
        brep.addFace(unitSquareFaceXY(5.0));
        CHECK(brep.faceAt(0).supportPlane() == Plane3D::xy(0.0));
        CHECK(brep.faceAt(1).supportPlane() == Plane3D::xy(5.0));
    }

    TEST_CASE("faces(): returns all stored faces") {
        Brep brep;
        brep.addFace(unitSquareFaceXY());
        brep.addFace(unitSquareFaceXZ());
        CHECK(brep.faces().size() == 2);
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: two empty Breps are equal") {
        CHECK(Brep() == Brep());
    }

    TEST_CASE("operator==: same faces in same order") {
        Brep a;
        a.addFace(unitSquareFaceXY());
        Brep b;
        b.addFace(unitSquareFaceXY());
        CHECK(a == b);
    }

    TEST_CASE("operator!=: different face counts") {
        Brep a;
        a.addFace(unitSquareFaceXY());
        Brep b;
        CHECK(a != b);
    }

    TEST_CASE("operator!=: different faces at same index") {
        Brep a;
        a.addFace(unitSquareFaceXY());
        Brep b;
        b.addFace(unitSquareFaceXZ());
        CHECK(a != b);
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: starts with 'Brep('") {
        Brep brep;
        brep.addFace(unitSquareFaceXY());
        std::ostringstream oss;
        oss << brep;
        CHECK(oss.str().find("Brep(") != std::string::npos);
    }

    TEST_CASE("operator<<: reports face count") {
        Brep brep;
        brep.addFace(unitSquareFaceXY());
        brep.addFace(unitSquareFaceXZ());
        std::ostringstream oss;
        oss << brep;
        CHECK(oss.str().find("2 faces") != std::string::npos);
    }

} // TEST_SUITE("Brep")
