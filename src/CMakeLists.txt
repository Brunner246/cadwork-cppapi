project(app)
add_library(${PROJECT_NAME} SHARED)

find_package(CwAPI3D CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE
    app.cpp
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
