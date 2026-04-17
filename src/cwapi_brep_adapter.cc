#include "cwapi_brep_adapter.hh"

#include <cstdint>
#include <cwapi3d/ICwAPI3DPolygonList.h>
#include <cwapi3d/ICwAPI3DVertexList.h>
#include <geometry/Face.hh>
#include <geometry/Loop.hh>
#include <geometry/Plane3D.hh>
#include <geometry/Point3D.hh>
#include <geometry/Vector3D.hh>
#include <vector>

namespace cwapi_adapter {

namespace {

geometry::Loop loopFromVertexList(CwAPI3D::Interfaces::ICwAPI3DVertexList *verts) {
    std::vector<geometry::Point3D> points;
    if (verts == nullptr) {
        return geometry::Loop(std::move(points));
    }
    const uint32_t n = verts->count();
    points.reserve(n);
    for (uint32_t i = 0; i < n; ++i) {
        const auto v = verts->at(i);
        points.emplace_back(v.mX, v.mY, v.mZ);
    }
    return geometry::Loop(std::move(points));
}

} // namespace

geometry::Brep toBrep(CwAPI3D::Interfaces::ICwAPI3DFacetList *facetList) {
    geometry::Brep brep;
    if (facetList == nullptr) {
        return brep;
    }

    const uint32_t facetCount = facetList->count();
    for (uint32_t i = 0; i < facetCount; ++i) {
        geometry::Loop outer = loopFromVertexList(facetList->getExternalPolygon(i));

        std::vector<geometry::Loop> inner;
        if (auto *holes = facetList->getInternalPolygons(i); holes != nullptr) {
            const uint32_t holeCount = holes->count();
            inner.reserve(holeCount);
            for (uint32_t h = 0; h < holeCount; ++h) {
                inner.emplace_back(loopFromVertexList(holes->at(h)));
            }
        }

        const auto n = facetList->getNormalVector(i);
        const geometry::Vector3D normal(n.mX, n.mY, n.mZ);

        // Prefer a real outer vertex as the point-on-plane over reconstructing
        // from distanceToOrigin, which divides by a possibly-small normal
        // component. Fall back to the origin when the outer loop is empty.
        const geometry::Point3D pointOnPlane =
            outer.isEmpty() ? geometry::Point3D::origin() : outer.vertexAt(0);

        const geometry::Plane3D plane = geometry::Plane3D::fromPointAndNormal(pointOnPlane, normal);

        brep.addFace(geometry::Face(std::move(outer), std::move(inner), plane));
    }
    return brep;
}

} // namespace cwapi_adapter
