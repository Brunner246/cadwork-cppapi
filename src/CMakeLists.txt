project(app)
add_library(${PROJECT_NAME} SHARED)

find_package(CwAPI3D CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE
    app.cpp
    Point3D.hh
    Point3D.cc
    Vector3D.hh
    Vector3D.cc
    Frame3D.hh
    Frame3D.cc
    Plane3D.hh
    Plane3D.cc
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        CwAPI3D::CwAPI3D
)


# Public include directory for consumers of this DLL
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        CWAPI3D_PLUGIN_NAME=L"Cadwork Test Plugin"
        CWAPI3D_AUTHOR_NAME=L"Your Name"
        CWAPI3D_AUTHOR_EMAIL=L"your_name@something.swiss"
)