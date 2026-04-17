# ==============================================================================
# Geometry Library - Test Suite
# ==============================================================================
#
# Test runner for the geometry library using doctest.
#
# All four classes are covered in separate translation units that are linked
# into a single test executable (geometry_tests).  The test executable is
# placed in the same output directory as the geometry shared library so that
# the DLL/SO is found at runtime without extra PATH manipulation.
#
# Running the tests:
#   ctest --test-dir <build_dir>                   # all tests
#   ctest --test-dir <build_dir> -V                # verbose
#   <build_dir>/bin/geometry_tests --help          # doctest CLI options
# ==============================================================================

find_package(doctest CONFIG REQUIRED)

add_executable(geometry_tests
    test_main.cc        # provides main() via DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    test_Vector3D.cc
    test_Point3D.cc
    test_Frame3D.cc
    test_Plane3D.cc
    test_Loop.cc
    test_Face.cc
    test_Brep.cc
)

target_link_libraries(geometry_tests
    PRIVATE
        geometry::geometry
        doctest::doctest
)

target_compile_features(geometry_tests PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(geometry_tests PRIVATE /W4 /wd4251)
else()
    target_compile_options(geometry_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Place the executable next to the geometry DLL so Windows finds it at runtime.
set_target_properties(geometry_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register with CTest so `ctest` picks up the tests automatically.
add_test(NAME geometry_tests COMMAND geometry_tests)
