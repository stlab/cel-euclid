cmake_minimum_required(VERSION 3.24)

include(cmake/CPM.cmake)
CPMAddPackage("gh:stlab/cpp-library@6.1.0")
include("${cpp-library_SOURCE_DIR}/cpp-library.cmake")
cpp_library_enable_dependency_tracking()

project(euclid LANGUAGES CXX)
include(CTest)

if(MSVC)
    set(floating_point_tests fast_math_fail.cpp)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    set(floating_point_tests fast_math_fail.cpp finite_math_fail.cpp)
endif()

cpp_library_setup(
    DESCRIPTION "CEL Euclid geometry"
    NAMESPACE cel
    HEADERS euclid.hpp
    SOURCES euclid.cpp geometry.cpp records.cpp transforms.cpp
    TESTS euclid_test.cpp transforms_test.cpp ${floating_point_tests}
    EXAMPLES example.cpp
    LICENSE_FILES licenses/GLM-LICENSE.txt licenses/CPM-MIT.txt
    DOCS_INPUTS README.md docs/tutorial.md docs/reference.md
    DOCS_EXCLUDE_SYMBOLS cel::op_lookup CEL_EUCLID_ADAPTER
    DOCS_OPTIONS
        "USE_MDFILE_AS_MAINPAGE = ${CMAKE_CURRENT_SOURCE_DIR}/README.md"
)
# Workaround until stlab/cpp-library#58 is resolved: add dependencies after
# setup so CEL_INSTALL defaults to top-level, and register GLM's export set.
# Select the direct runtime before the parser's transitive dependency.
CPMAddPackage("gh:stlab/cel-runtime@1.1.2")
CPMAddPackage("gh:stlab/cel-parser@1.1.1")
CPMAddPackage(
    URI "gh:g-truc/glm@1.0.3#8d1fd52e5ab5590e2c81768ace50c72bae28f2ed"
    OPTIONS "GLM_BUILD_INSTALL ${CEL_INSTALL}"
)

target_link_libraries(euclid PUBLIC cel::parser cel::runtime glm::glm)
if(MSVC)
    target_compile_options(euclid PRIVATE /fp:strict)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    target_compile_options(euclid PRIVATE -ffp-contract=off)
endif()
if(TARGET fast_math_fail)
    if(MSVC)
        target_compile_options(fast_math_fail PRIVATE /fp:fast)
    else()
        target_compile_options(fast_math_fail PRIVATE -ffast-math)
        target_compile_options(finite_math_fail PRIVATE -ffinite-math-only)
    endif()
endif()
