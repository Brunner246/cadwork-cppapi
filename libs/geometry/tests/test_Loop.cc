/**
 * @file test_Loop.cc
 * @brief Unit tests for geometry::Loop.
 *
 * A Loop is an ordered, closed polyline of Point3D vertices. The closing
 * edge between the last and first vertex is implicit.
 *
 * Test groups
 * -----------
 * Construction
 *   Default constructor yields an empty loop.
 *   Vector constructor stores vertices verbatim.
 *
 * Accessors
 *   vertexCount(), vertices(), isEmpty(), vertexAt().
 *   vertexAt() with out-of-range index must throw std::out_of_range.
 *
 * Comparison
 *   operator== / != compare element-wise on vertices.
 *
 * Stream output
 *   operator<< starts with "Loop(" and reports vertex count.
 */

#include "geometry/Loop.hh"
#include "geometry/Point3D.hh"

#include <doctest/doctest.h>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace geometry;

TEST_SUITE("Loop") {

    // ==========================================================================
    // Construction
    // ==========================================================================

    TEST_CASE("default constructor: empty loop") {
        const Loop loop;
        CHECK(loop.isEmpty());
        CHECK(loop.vertexCount() == 0);
    }

    TEST_CASE("vector constructor: stores the supplied vertices") {
        const std::vector<Point3D> verts{Point3D(0.0, 0.0, 0.0), Point3D(1.0, 0.0, 0.0),
                                         Point3D(0.0, 1.0, 0.0)};
        const Loop loop(verts);
        CHECK(loop.vertexCount() == 3);
        CHECK_FALSE(loop.isEmpty());
    }

    TEST_CASE("vector constructor: preserves vertex order") {
        const Loop loop({Point3D(1.0, 2.0, 3.0), Point3D(4.0, 5.0, 6.0)});
        CHECK(loop.vertexAt(0) == Point3D(1.0, 2.0, 3.0));
        CHECK(loop.vertexAt(1) == Point3D(4.0, 5.0, 6.0));
    }

    // ==========================================================================
    // Accessors
    // ==========================================================================

    TEST_CASE("vertices(): returns all stored vertices") {
        const Loop loop({Point3D(0.0, 0.0, 0.0), Point3D(1.0, 1.0, 1.0)});
        CHECK(loop.vertices().size() == 2);
    }

    TEST_CASE("vertexAt: out-of-range index throws std::out_of_range") {
        const Loop loop({Point3D(0.0, 0.0, 0.0)});
        CHECK_THROWS_AS(std::ignore = loop.vertexAt(1), std::out_of_range);
    }

    TEST_CASE("vertexAt: out-of-range on empty loop throws") {
        const Loop empty;
        CHECK_THROWS_AS(std::ignore = empty.vertexAt(0), std::out_of_range);
    }

    // ==========================================================================
    // Comparison
    // ==========================================================================

    TEST_CASE("operator==: two loops with identical vertices compare equal") {
        const Loop a({Point3D(0.0, 0.0, 0.0), Point3D(1.0, 0.0, 0.0)});
        const Loop b({Point3D(0.0, 0.0, 0.0), Point3D(1.0, 0.0, 0.0)});
        CHECK(a == b);
    }

    TEST_CASE("operator==: two empty loops compare equal") {
        CHECK(Loop() == Loop());
    }

    TEST_CASE("operator!=: different vertex counts are unequal") {
        const Loop a({Point3D(0.0, 0.0, 0.0)});
        const Loop b({Point3D(0.0, 0.0, 0.0), Point3D(1.0, 0.0, 0.0)});
        CHECK(a != b);
    }

    TEST_CASE("operator!=: different vertex values are unequal") {
        const Loop a({Point3D(0.0, 0.0, 0.0)});
        const Loop b({Point3D(1.0, 0.0, 0.0)});
        CHECK(a != b);
    }

    // ==========================================================================
    // Stream output
    // ==========================================================================

    TEST_CASE("operator<<: starts with 'Loop('") {
        std::ostringstream oss;
        oss << Loop({Point3D(0.0, 0.0, 0.0)});
        CHECK(oss.str().find("Loop(") != std::string::npos);
    }

    TEST_CASE("operator<<: empty loop reports 0 vertices") {
        std::ostringstream oss;
        oss << Loop();
        CHECK(oss.str().find("0 vertices") != std::string::npos);
    }

} // TEST_SUITE("Loop")
