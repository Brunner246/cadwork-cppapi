#ifndef GEOMETRY_FACE_HH
#define GEOMETRY_FACE_HH

#include "geometry/Loop.hh"
#include "geometry/Plane3D.hh"
#include "geometry/Vector3D.hh"
#include "geometry/export.hh"

#include <cstddef>
#include <ostream>
#include <vector>

namespace geometry {

/**
 * @brief A planar face described by an outer loop, optional holes, and a support plane.
 *
 * Faces are the building blocks of a Brep. Each face is assumed to be planar
 * (all vertices of all loops lie on the support plane within tolerance). No
 * topological relationship is maintained between faces — shared edges or
 * vertices between faces are implicit.
 */
class GEOMETRY_API Face {
  public:
    Face(Loop outerLoop, Plane3D supportPlane);
    Face(Loop outerLoop, std::vector<Loop> innerLoops, Plane3D supportPlane);

    // Copy and move constructors/assignments (defaulted)
    Face(const Face &) = default;
    Face(Face &&) noexcept = default;
    Face &operator=(const Face &) = default;
    Face &operator=(Face &&) noexcept = default;
    ~Face() = default;

    // Accessors
    [[nodiscard]] const Loop &outerLoop() const { return m_outer; }
    [[nodiscard]] const std::vector<Loop> &innerLoops() const { return m_inner; }
    [[nodiscard]] const Plane3D &supportPlane() const { return m_plane; }
    [[nodiscard]] const Vector3D &normal() const { return m_plane.normal(); }
    [[nodiscard]] std::size_t innerLoopCount() const { return m_inner.size(); }
    [[nodiscard]] bool hasHoles() const { return !m_inner.empty(); }

    // Comparison operators
    bool operator==(const Face &other) const;
    bool operator!=(const Face &other) const;

    // Stream output
    friend GEOMETRY_API std::ostream &operator<<(std::ostream &os, const Face &face);

  private:
    Loop m_outer;
    std::vector<Loop> m_inner;
    Plane3D m_plane;
};
} // namespace geometry
#endif // GEOMETRY_FACE_HH
