#include "geometry/Brep.hh"

#include <stdexcept>
#include <utility>

namespace geometry {

Brep::Brep(std::vector<Face> faces) : m_faces(std::move(faces)) {}

const Face &Brep::faceAt(std::size_t index) const {
    if (index >= m_faces.size()) {
        throw std::out_of_range("Brep::faceAt: index out of range");
    }
    return m_faces[index];
}

void Brep::addFace(Face face) {
    m_faces.push_back(std::move(face));
}

bool Brep::operator==(const Brep &other) const {
    if (m_faces.size() != other.m_faces.size()) {
        return false;
    }
    for (std::size_t i = 0; i < m_faces.size(); ++i) {
        if (!(m_faces[i] == other.m_faces[i])) {
            return false;
        }
    }
    return true;
}

bool Brep::operator!=(const Brep &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Brep &brep) {
    os << "Brep(" << brep.m_faces.size() << " faces)";
    return os;
}
} // namespace geometry
