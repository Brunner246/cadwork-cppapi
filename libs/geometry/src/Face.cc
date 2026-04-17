#include "geometry/Face.hh"

#include <utility>

namespace geometry {

Face::Face(Loop outerLoop, Plane3D supportPlane)
    : m_outer(std::move(outerLoop)), m_plane(supportPlane) {}

Face::Face(Loop outerLoop, std::vector<Loop> innerLoops, Plane3D supportPlane)
    : m_outer(std::move(outerLoop)), m_inner(std::move(innerLoops)), m_plane(supportPlane) {}

bool Face::operator==(const Face &other) const {
    if (!(m_outer == other.m_outer)) {
        return false;
    }
    if (m_inner.size() != other.m_inner.size()) {
        return false;
    }
    for (std::size_t i = 0; i < m_inner.size(); ++i) {
        if (!(m_inner[i] == other.m_inner[i])) {
            return false;
        }
    }
    return m_plane == other.m_plane;
}

bool Face::operator!=(const Face &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Face &face) {
    os << "Face(outer=" << face.m_outer << ", holes=" << face.m_inner.size()
       << ", plane=" << face.m_plane << ")";
    return os;
}
} // namespace geometry
