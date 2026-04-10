project(app)
add_library(${PROJECT_NAME} SHARED)

find_package(CwAPI3D CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE
    app.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CwAPI3D::CwAPI3D
        geometry::geometry
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        CWAPI3D_PLUGIN_NAME=L"Cadwork Test Plugin"
        CWAPI3D_AUTHOR_NAME=L"Your Name"
        CWAPI3D_AUTHOR_EMAIL=L"your_name@something.swiss"
)