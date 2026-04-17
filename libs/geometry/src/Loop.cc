#include "geometry/Loop.hh"

#include <stdexcept>
#include <utility>

namespace geometry {

Loop::Loop(std::vector<Point3D> vertices) : m_vertices(std::move(vertices)) {}

const Point3D &Loop::vertexAt(std::size_t index) const {
    if (index >= m_vertices.size()) {
        throw std::out_of_range("Loop::vertexAt: index out of range");
    }
    return m_vertices[index];
}

bool Loop::operator==(const Loop &other) const {
    if (m_vertices.size() != other.m_vertices.size()) {
        return false;
    }
    for (std::size_t i = 0; i < m_vertices.size(); ++i) {
        if (!(m_vertices[i] == other.m_vertices[i])) {
            return false;
        }
    }
    return true;
}

bool Loop::operator!=(const Loop &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Loop &loop) {
    os << "Loop(" << loop.m_vertices.size() << " vertices";
    if (!loop.m_vertices.empty()) {
        os << ": [";
        for (std::size_t i = 0; i < loop.m_vertices.size(); ++i) {
            if (i > 0) {
                os << ", ";
            }
            os << loop.m_vertices[i];
        }
        os << "]";
    }
    os << ")";
    return os;
}
} // namespace geometry
