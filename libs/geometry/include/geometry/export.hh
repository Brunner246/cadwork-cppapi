#ifndef GEOMETRY_EXPORT_HH
#define GEOMETRY_EXPORT_HH

/**
 * @file export.hh
 * @brief DLL export/import macros for the geometry library.
 *
 * These macros handle symbol visibility for shared library builds on Windows and other platforms.
 * On Windows, symbols must be explicitly exported from DLLs using __declspec(dllexport/dllimport).
 * On Unix-like systems with GCC/Clang, we use visibility attributes.
 */

#if defined(_WIN32) || defined(_WIN64)
    #ifdef GEOMETRY_EXPORTS
        #define GEOMETRY_API __declspec(dllexport)
    #else
        #define GEOMETRY_API __declspec(dllimport)
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define GEOMETRY_API __attribute__((visibility("default")))
    #else
        #define GEOMETRY_API
    #endif
#endif

#endif // GEOMETRY_EXPORT_HH
