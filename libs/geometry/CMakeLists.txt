# ==============================================================================
# Geometry Library - Modern CMake Configuration
# ==============================================================================
#
# This library provides 3D geometry primitives: Point3D, Vector3D, Frame3D, Plane3D
#
# Usage:
#   find_package(geometry CONFIG REQUIRED)  # If installed
#   target_link_libraries(myapp PRIVATE geometry::geometry)
#
# Or as a subdirectory:
#   add_subdirectory(libs/geometry)
#   target_link_libraries(myapp PRIVATE geometry::geometry)
# ==============================================================================

# cmake_minimum_required(VERSION 3.21)

project(geometry
    VERSION 1.0.0
    DESCRIPTION "3D Geometry primitives library"
    LANGUAGES CXX
)

# ==============================================================================
# Library Target
# ==============================================================================

add_library(${PROJECT_NAME} SHARED)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ==============================================================================
# Source Files
# ==============================================================================

target_sources(${PROJECT_NAME}
    PRIVATE
        src/Point3D.cc
        src/Vector3D.cc
        src/Frame3D.cc
        src/Plane3D.cc
)

# ==============================================================================
# Include Directories
# ==============================================================================

target_include_directories(${PROJECT_NAME}
    PUBLIC
        # Build interface: use include/ for building
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install interface: use include/ after installation
        $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# Compile Definitions
# ==============================================================================

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        # Define GEOMETRY_EXPORTS when building the library (for DLL export)
        GEOMETRY_EXPORTS
)

# ==============================================================================
# Compile Features (C++ Standard)
# ==============================================================================

target_compile_features(${PROJECT_NAME}
    PUBLIC
        cxx_std_23
)

# ==============================================================================
# Compiler Warnings (applied only to this target)
# ==============================================================================

if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            /W4           # High warning level
            /wd4251       # Suppress DLL-interface warnings for STL types
    )
else()
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

# ==============================================================================
# Symbol Visibility (for non-Windows platforms)
# ==============================================================================

if(NOT WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# ==============================================================================
# Output Directory Configuration
# ==============================================================================

set_target_properties(${PROJECT_NAME} PROPERTIES
    # Ensure consistent output directories
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    
    # Version information
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ==============================================================================
# Installation Rules (for packaging/distribution)
# ==============================================================================

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# ==============================================================================
# Package Configuration
# ==============================================================================

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
