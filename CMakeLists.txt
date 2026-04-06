cmake_minimum_required(VERSION 3.31)

project(cadwork_cppapi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add vcpkg installed prefix to CMake's search path when VCPKG_ROOT is set
# if(DEFINED ENV{VCPKG_ROOT})
#     list(PREPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
# endif()

if(MSVC)
    add_compile_options(/W4 /wd4251)  # Warning 4251 (DLL-Interface) ignore
endif()

# Add the source directory containing the DLL target "app"
add_subdirectory(src)
