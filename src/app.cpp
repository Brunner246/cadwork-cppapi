#include "Frame3D.hh"
#include "Point3D.hh"

#include <cstdint>
#include <cwapi3d/CwAPI3D.h>
#include <cwapi3d/CwAPI3DTypes.h>
#include <cwapi3d/ICwAPI3DElementIDList.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace {
[[nodiscard]] inline std::string toUtf8(std::wstring_view wstr) {
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

// Minimal function so the DLL has at least one symbol
void appHello() {
    std::cout << "Hello from app DLL" << std::endl;
}

enum class ElementType {
    Wall,
    Floor,
    Roof,
    Opening,
    Beam,
    Plate,
};

std::string_view toString(ElementType type) {
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
    Frame3D localFrame; // Example of including a complex type in the data structure
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
                   const std::function<Frame3D(uint64_t)> &getFrameFunc)
    -> std::vector<ElementData> {
    std::vector<ElementData> elements;
    const auto count = elementIDs->count();
    for (int i = 0; std::cmp_less(i, count); ++i) {
        const auto elementId = elementIDs->at(i);
        std::string name = getNameFunc(elementId);
        std::optional<ElementType> type = getTypeFunc(elementId);
        Frame3D localFrame = getFrameFunc(elementId);
        elements.emplace_back(
            ElementData{.id = elementId, .name = name, .type = type, .localFrame = localFrame});
    }
    return elements;
}

void print_chunks(auto view, std::string_view separator = ", ") {
    for (auto const subrange : view) {
        std::cout << '[';
        for (std::string_view prefix; auto const &elem : subrange)
            std::cout << prefix << elem, prefix = separator;
        std::cout << "] ";
    }
    std::cout << '\n';
}
} // namespace

CWAPI3D_PLUGIN auto plugin_x64_init(CwAPI3D::ControllerFactory *factory) -> bool {
    std::cout << "Initializing plugin with factory: " << factory << '\n';

    auto *elementController = factory->getElementController();
    auto *elementIDs = elementController->getActiveIdentifiableElementIDs();

    auto *attributeController = factory->getAttributeController();
    auto getNameFunc = [attributeController](uint64_t elementId) -> std::string {
        auto *nameAttr = attributeController->getName(elementId);
        if (nameAttr) {
            std::wstring wname = nameAttr->data();
            return toUtf8(wname);
        }
        return "Unknown";
    };

    auto getTypeFunc = [attributeController](uint64_t elementId) -> std::optional<ElementType> {
        auto *typeAttr = attributeController->getElementType(elementId);
        if (typeAttr->isFramedWall()) {
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

    auto getFrameFunc = [geometryController](uint64_t elementId) -> Frame3D {
        const auto origin = geometryController->getP1(elementId);
        const auto axisX = geometryController->getXL(elementId);
        const auto axisY = geometryController->getYL(elementId);

        return Frame3D(Point3D(origin.mX, origin.mY, origin.mZ),
                       Vector3D(axisX.mX, axisX.mY, axisX.mZ),
                       Vector3D(axisY.mX, axisY.mY, axisY.mZ));
    };

    std::vector<ElementData> elements =
        toElementData(elementIDs, getNameFunc, getTypeFunc, getFrameFunc);
    const auto filteredElements =
        elements | std::views::filter([](const ElementData &e) { return e.type.has_value(); })
        | std::views::filter(
            [](const ElementData &e) { return e.type.value() == ElementType::Beam; })
        | std::views::transform([](const ElementData &e) { return e; })
        | std::ranges::to<std::vector>();

    std::cout << "Filtered beam elements: ";
    std::ranges::copy(filteredElements, std::ostream_iterator<ElementData>(std::cout, "; "));
    std::cout << '\n';

    if (!std::empty(filteredElements)) {
        const auto &firstBeam = filteredElements.front();
        const auto lToWorldPoint = firstBeam.localFrame.localToWorld(Point3D{0.0, 20.0, 20.0});
        // const auto lToWorldPoint = firstBeam.localFrame.localToWorld(localPoint);
        elementController->createNode(
            CwAPI3D::vector3D{lToWorldPoint.x, lToWorldPoint.y, lToWorldPoint.z});
    }

    std::initializer_list v1 = {1, 2, 3, 1, 2, 3, 3, 3, 1, 2, 3};
    auto fn1 = std::ranges::less{};
    auto view1 = v1 | std::views::chunk_by(fn1);

    std::cout << "chunk_by result: ";
    print_chunks(view1);

    return true;
}
