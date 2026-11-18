cmake_minimum_required(VERSION 3.24)

include(cmake/CPM.cmake)
CPMAddPackage(NAME cpp-library GITHUB_REPOSITORY stlab/cpp-library
    GIT_TAG 13be83b3d92aeb1969fb6f8d13e81b372bb57d43)
include("${cpp-library_SOURCE_DIR}/cpp-library.cmake")
cpp_library_enable_dependency_tracking()

project(euclid LANGUAGES CXX)
include(CTest)
option(CEL_INSTALL "Enable installation for CEL libraries" ${PROJECT_IS_TOP_LEVEL})

CPMAddPackage("gh:stlab/cel-runtime@1.1.0")
CPMAddPackage(NAME cel-parser GITHUB_REPOSITORY stlab/cel-parser
    GIT_TAG e090f994c11a0023aee9b894050be62394ebcce9)
CPMAddPackage(NAME glm GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 8d1fd52e5ab5590e2c81768ace50c72bae28f2ed
    OPTIONS "GLM_BUILD_INSTALL ${CEL_INSTALL}")
if(BUILD_TESTING)
    CPMAddPackage(NAME cel-std GITHUB_REPOSITORY stlab/cel-std
        GIT_TAG 493acb275cc59cdcf802175d07c806aeefed8686)
endif()

cpp_library_setup(
    DESCRIPTION "CEL Euclid geometry"
    NAMESPACE cel
    REQUIRES_CPP_VERSION 20
    HEADERS euclid.hpp
    SOURCES euclid.cpp geometry.cpp transforms.cpp
    TESTS euclid_test.cpp transforms_test.cpp
    EXAMPLES example.cpp
    DOCS_INPUTS README.md docs/tutorial.md docs/reference.md
    DOCS_OPTIONS
        "USE_MDFILE_AS_MAINPAGE = ${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "EXCLUDE_SYMBOLS = cel::op_lookup CEL_EUCLID_ADAPTER"
        "WARN_AS_ERROR = YES"
        "FULL_PATH_NAMES = NO"
        "STRIP_FROM_INC_PATH = ${CMAKE_CURRENT_SOURCE_DIR}/include"
)
cpp_library_map_dependency("cel::parser" "cel-parser CONFIG")
cpp_library_map_dependency("glm::glm" "glm CONFIG")
target_link_libraries(euclid PUBLIC cel::parser glm::glm)
if(MSVC)
    target_compile_options(euclid PRIVATE /fp:strict)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    target_compile_options(euclid PRIVATE -ffp-contract=off)
endif()
if(CEL_INSTALL)
    install(FILES LICENSE licenses/GLM-LICENSE.txt licenses/CPM-MIT.txt
        DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/cel-euclid")
endif()
if(TARGET euclid_test)
    target_link_libraries(euclid_test PRIVATE cel::std)
endif()
if(TARGET transforms_test)
    target_link_libraries(transforms_test PRIVATE cel::std)
endif()
