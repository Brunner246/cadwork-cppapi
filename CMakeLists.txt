cmake_minimum_required(VERSION 3.31)

project(cadwork_cppapi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy compile_commands.json to project root for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

# Add vcpkg installed prefix to CMake's search path when VCPKG_ROOT is set
# if(DEFINED ENV{VCPKG_ROOT})
#     list(PREPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
# endif()

if(MSVC)
    add_compile_options(/W4 /wd4251)  # Warning 4251 (DLL-Interface) ignore
endif()

# Add the source directory containing the DLL target "app"
add_subdirectory(src)
