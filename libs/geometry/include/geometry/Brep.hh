#ifndef GEOMETRY_BREP_HH
#define GEOMETRY_BREP_HH

#include "geometry/Face.hh"
#include "geometry/export.hh"

#include <cstddef>
#include <ostream>
#include <vector>

namespace geometry {

/**
 * @brief A boundary representation: a collection of planar faces.
 *
 * This lightweight Brep does not track shared edges or vertices between
 * faces. Each Face carries its own outer/inner loops and support plane.
 */
class GEOMETRY_API Brep {
  public:
    Brep() = default;
    explicit Brep(std::vector<Face> faces);

    // Copy and move constructors/assignments (defaulted)
    Brep(const Brep &) = default;
    Brep(Brep &&) noexcept = default;
    Brep &operator=(const Brep &) = default;
    Brep &operator=(Brep &&) noexcept = default;
    ~Brep() = default;

    // Accessors
    [[nodiscard]] const std::vector<Face> &faces() const { return m_faces; }
    [[nodiscard]] std::size_t faceCount() const { return m_faces.size(); }
    [[nodiscard]] bool isEmpty() const { return m_faces.empty(); }

    /**
     * @brief Return the face at the given index.
     * @throws std::out_of_range if index is beyond faceCount().
     */
    [[nodiscard]] const Face &faceAt(std::size_t index) const;

    // Mutators
    void addFace(Face face);

    // Comparison operators
    bool operator==(const Brep &other) const;
    bool operator!=(const Brep &other) const;

    // Stream output
    friend GEOMETRY_API std::ostream &operator<<(std::ostream &os, const Brep &brep);

  private:
    std::vector<Face> m_faces;
};
} // namespace geometry
#endif // GEOMETRY_BREP_HH
