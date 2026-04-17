#ifndef CWAPI_BREP_ADAPTER_HH
#define CWAPI_BREP_ADAPTER_HH

#include <cwapi3d/ICwAPI3DFacetList.h>
#include <geometry/Brep.hh>

namespace cwapi_adapter {

/**
 * @brief Copy all facets from an ICwAPI3DFacetList into a geometry::Brep.
 *
 * The returned Brep contains one Face per facet. Each Face carries:
 *   - an outer Loop built from getExternalPolygon(i)
 *   - zero or more inner Loops built from getInternalPolygons(i)
 *   - a support Plane3D built from getNormalVector(i) and the first vertex
 *     of the outer loop (a real point on the plane, avoiding numerical
 *     issues from reconstructing via getDistanceToOrigin()).
 *
 * Ownership: the caller retains ownership of @p facetList (no destroy()).
 * A null @p facetList yields an empty Brep.
 */
[[nodiscard]] geometry::Brep toBrep(CwAPI3D::Interfaces::ICwAPI3DFacetList *facetList);

} // namespace cwapi_adapter

#endif // CWAPI_BREP_ADAPTER_HH
