#include "cwapi_brep_adapter.hh"

#include <cstdint>
#include <cwapi3d/CwAPI3D.h>
#include <cwapi3d/CwAPI3DTypes.h>
#include <cwapi3d/ICwAPI3DElementIDList.h>
#include <cwapi3d/ICwAPI3DFacetList.h>
#include <functional>
#include <geometry/Brep.hh>
#include <geometry/Frame3D.hh>
#include <geometry/Point3D.hh>
#include <geometry/Vector3D.hh>
#include <iostream>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>

namespace {
[[nodiscard]] inline std::string toUtf8(const std::wstring_view wstr) {
    if (wstr.empty()) {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                                         nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), size,
                        nullptr, nullptr);
    return result;
}

enum class ElementType {
    Wall,
    Floor,
    Roof,
    Opening,
    Beam,
    Plate,
};

std::string_view toString(const ElementType type) {
    switch (type) {
        case ElementType::Wall:
            return "Wall";
        case ElementType::Floor:
            return "Floor";
        case ElementType::Roof:
            return "Roof";
        case ElementType::Opening:
            return "Opening";
        case ElementType::Beam:
            return "Beam";
        case ElementType::Plate:
            return "Plate";
        default:
            return "Unknown";
    }
}

struct alignas(64) ElementData {
    uint64_t id;
    std::string name;
    std::optional<ElementType> type;
    geometry::Frame3D localFrame; // Example of including a complex type in the data structure
};

std::ostream &operator<<(std::ostream &os, const ElementData &element) {
    os << "ElementData{id: " << element.id << ", name: " << element.name;
    if (element.type.has_value()) {
        os << ", type: " << toString(element.type.value());
    } else {
        os << ", type: None";
    }
    os << ", localFrame: " << element.localFrame;
    os << '}';
    return os;
}

auto toElementData(CwAPI3D::Interfaces::ICwAPI3DElementIDList *elementIDs,
                   const std::function<std::string(uint64_t)> &getNameFunc,
                   const std::function<std::optional<ElementType>(uint64_t)> &getTypeFunc,
                   const std::function<geometry::Frame3D(uint64_t)> &getFrameFunc)
    -> std::vector<ElementData> {
    std::vector<ElementData> elements;
    const auto count = elementIDs->count();
    for (int i = 0; std::cmp_less(i, count); ++i) {
        const auto elementId = elementIDs->at(i);
        std::string const name = getNameFunc(elementId);
        std::optional<ElementType> const type = getTypeFunc(elementId);
        try {
            geometry::Frame3D const localFrame = getFrameFunc(elementId);
            elements.emplace_back(
                ElementData{.id = elementId, .name = name, .type = type, .localFrame = localFrame});
        } catch (const std::invalid_argument &e) {
            std::cerr << "Error getting frame for element ID " << elementId << ": " << e.what()
                      << " This element will be skipped." << '\n';
            // Optionally
            // elements.emplace_back(ElementData{.id = elementId, .name = name, .type = type,
            // .localFrame = geometry::Frame3D::worldFrame()});
        }
    }
    return elements;
}

void print_chunks(auto view, const std::string_view separator = ", ") {
    for (auto const subrange : view) {
        std::cout << '[';
        for (std::string_view prefix; auto const &elem : subrange)
            std::cout << prefix << elem, prefix = separator;
        std::cout << "] ";
    }
    std::cout << '\n';
}

class Person {
  public:
    [[nodiscard]] int getAge() const { return age; }

  private:
    int age = 0;
};
} // namespace

CWAPI3D_PLUGIN auto plugin_x64_init(CwAPI3D::ControllerFactory *factory) -> bool {
    std::cout << "Initializing plugin with factory: " << factory << '\n';

    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::vector<int> out;
    auto [in, out_it] = std::ranges::copy(v, std::back_inserter(out));

    std::vector<std::string> words{"hello", "world", "cpp"};

    auto upper = words | std::views::transform([](std::string s) {
                     std::ranges::transform(s, s.begin(), ::toupper);
                     return s;
                 });
    std::ranges::copy(upper, std::ostream_iterator<std::string>(std::cout, " "));

    std::vector<Person> people;
    std::ranges::sort(people, std::less<>{}, &Person::getAge);

    // ========== Plugin-specific code starts here ==========
    auto *nameList = factory->getAttributeController()->getNameListItems();
    std::vector<std::wstring> names;
    names.reserve(nameList->count());
    for (uint32_t i = 0; std::cmp_less(i, nameList->count()); ++i) {
        names.emplace_back(nameList->at(i)->data());
    }
    auto namesStdString = names | std::views::transform(&toUtf8) | std::ranges::to<std::vector>();
    std::cout << "Element names: ";
    std::ranges::copy(namesStdString, std::ostream_iterator<std::string>(std::cout, ", "));
    std::cout << '\n';

    auto *elementController = factory->getElementController();
    auto *elementIDs = elementController->getActiveIdentifiableElementIDs();

    auto *attributeController = factory->getAttributeController();
    auto getNameFunc = [attributeController](const uint64_t elementId) -> std::string {
        auto *nameAttr = attributeController->getName(elementId);
        if (nameAttr) {
            std::wstring const wname = nameAttr->data();
            return toUtf8(wname);
        }
        return "Unknown";
    };

    auto getTypeFunc =
        [attributeController](const uint64_t elementId) -> std::optional<ElementType> {
        if (auto *typeAttr = attributeController->getElementType(elementId);
            typeAttr->isFramedWall()) {
            return ElementType::Wall;
        } else if (typeAttr->isFloor()) {
            return ElementType::Floor;
        } else if (typeAttr->isRoof()) {
            return ElementType::Roof;
        } else if (typeAttr->isOpening()) {
            return ElementType::Opening;
        } else if (typeAttr->isRectangularBeam()) {
            return ElementType::Beam;
        } else if (typeAttr->isPanel()) {
            return ElementType::Plate;
        }
        return std::nullopt;
    };

    auto *geometryController = factory->getGeometryController();

    auto getFrameFunc = [geometryController](const uint64_t elementId) -> geometry::Frame3D {
        const auto origin = geometryController->getP1(elementId);
        const auto axisX = geometryController->getXL(elementId);
        const auto axisY = geometryController->getYL(elementId);

        return geometry::Frame3D(geometry::Point3D(origin.mX, origin.mY, origin.mZ),
                                 geometry::Vector3D(axisX.mX, axisX.mY, axisX.mZ),
                                 geometry::Vector3D(axisY.mX, axisY.mY, axisY.mZ));
    };

    std::vector<ElementData> elements =
        toElementData(elementIDs, getNameFunc, getTypeFunc, getFrameFunc);
    // clang-format off
    const auto filteredElements =
        elements 
        | std::views::filter([](const ElementData &e) { return e.type.has_value(); })
        | std::views::filter([](const ElementData &e) { return e.type.value() == ElementType::Beam; })
        | std::views::transform([](const ElementData &e) { return e; })
        | std::ranges::to<std::vector>();
    // clang-format on

    std::cout << "Filtered beam elements: ";
    std::ranges::copy(filteredElements, std::ostream_iterator<ElementData>(std::cout, "; "));
    std::cout << '\n';

    if (!std::empty(filteredElements)) {
        const auto &firstBeam = filteredElements.front();
        const auto lToWorldPoint =
            firstBeam.localFrame.localToWorld(geometry::Point3D{0.0, 20.0, 20.0});
        // const auto lToWorldPoint = firstBeam.localFrame.localToWorld(localPoint);
        elementController->createNode(
            CwAPI3D::vector3D{lToWorldPoint.x, lToWorldPoint.y, lToWorldPoint.z});

        auto *facetList = geometryController->getElementFacets(firstBeam.id);
        const geometry::Brep brep = cwapi_adapter::toBrep(facetList);
        if (facetList != nullptr) {
            facetList->destroy();
        }
        std::cout << "Brep for element " << firstBeam.id << ": " << brep << '\n';
        for (std::size_t i = 0; i < brep.faceCount(); ++i) {
            const auto &face = brep.faceAt(i);
            std::cout << "  face " << i << ": outer=" << face.outerLoop().vertexCount()
                      << " verts, holes=" << face.innerLoopCount() << ", normal=" << face.normal()
                      << '\n';
        }
    }

    std::initializer_list v1 = {1, 2, 3, 1, 2, 3, 3, 3, 1, 2, 3};
    auto fn1 = std::ranges::less{};
    auto view1 = v1 | std::views::chunk_by(fn1);

    std::cout << "chunk_by result: ";
    print_chunks(view1);

    return true;
}
