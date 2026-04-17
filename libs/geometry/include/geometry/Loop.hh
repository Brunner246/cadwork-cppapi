#ifndef GEOMETRY_LOOP_HH
#define GEOMETRY_LOOP_HH

#include "geometry/Point3D.hh"
#include "geometry/export.hh"

#include <cstddef>
#include <ostream>
#include <vector>

namespace geometry {

/**
 * @brief An ordered, closed polyline of 3D points.
 *
 * A Loop stores a sequence of vertices describing either the outer boundary
 * or an inner hole of a planar face. The closing edge between the last and
 * first vertex is implicit — it is not stored.
 */
class GEOMETRY_API Loop {
  public:
    Loop() = default;
    explicit Loop(std::vector<Point3D> vertices);

    // Copy and move constructors/assignments (defaulted)
    Loop(const Loop &) = default;
    Loop(Loop &&) noexcept = default;
    Loop &operator=(const Loop &) = default;
    Loop &operator=(Loop &&) noexcept = default;
    ~Loop() = default;

    // Accessors
    [[nodiscard]] const std::vector<Point3D> &vertices() const { return m_vertices; }
    [[nodiscard]] std::size_t vertexCount() const { return m_vertices.size(); }
    [[nodiscard]] bool isEmpty() const { return m_vertices.empty(); }

    /**
     * @brief Return the vertex at the given index.
     * @throws std::out_of_range if index is beyond vertexCount().
     */
    [[nodiscard]] const Point3D &vertexAt(std::size_t index) const;

    // Comparison operators
    bool operator==(const Loop &other) const;
    bool operator!=(const Loop &other) const;

    // Stream output
    friend GEOMETRY_API std::ostream &operator<<(std::ostream &os, const Loop &loop);

  private:
    std::vector<Point3D> m_vertices;
};
} // namespace geometry
#endif // GEOMETRY_LOOP_HH
