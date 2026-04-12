/**
 * @file test_main.cc
 * @brief doctest entry point for the geometry test suite.
 *
 * Defining DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN before including the header
 * causes doctest to emit its own main() function in this translation unit.
 * Every TEST_CASE / TEST_SUITE defined in the other .cc files is registered
 * via static constructors and discovered automatically at runtime — no manual
 * test list needed.
 *
 * Do NOT define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN in any other TU.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
