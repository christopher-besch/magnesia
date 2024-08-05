cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

project(
    magnesia
    LANGUAGES CXX
    VERSION 0.0.1
)

include(PreventInSourceBuilds)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.28")
    set(EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
else()
    set(EXCLUDE_FROM_ALL)
endif()

include(FetchContent)
set(UA_ENABLE_ENCRYPTION
    OPENSSL
    CACHE INTERNAL ""
)
FetchContent_Declare(
    open62541pp
    GIT_REPOSITORY https://github.com/open62541pp/open62541pp.git
    GIT_TAG 6b83f185e0fbc78f98647ff4afd848b8a17cbc60 # v0.13.0
    GIT_SHALLOW ON
    SYSTEM
    ${EXCLUDE_FROM_ALL}
    # cmake-format: off
    FIND_PACKAGE_ARGS 0.13
    # cmake-format: on
)
message(STATUS "Fetching dependencies. This may take a while...")
FetchContent_MakeAvailable(open62541pp)

# qt6-base-dev in current debian stable (bookworm) is at least 6.4.2
find_package(
    Qt6 6.4 REQUIRED
    COMPONENTS Core
               Widgets
               Sql
               Network
)
qt_standard_project_setup()

message(STATUS "Using cmake ${CMAKE_VERSION}")
message(STATUS "Using open62541 ${open62541_VERSION}")
message(STATUS "Using open62541pp ${open62541pp_VERSION}")
message(STATUS "Using qt ${Qt6_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(MAGNESIA_IS_RELEASE ON)
    set(MAGNESIA_IS_NOT_RELEASE OFF)
else()
    set(MAGNESIA_IS_RELEASE OFF)
    set(MAGNESIA_IS_NOT_RELEASE ON)
endif()

add_library(magnesia_project_options INTERFACE)
add_library(magnesia::project_options ALIAS magnesia_project_options)

option(MAGNESIA_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ${MAGNESIA_IS_RELEASE})
include(CompilerWarnings)
# cmake-format: off
magnesia_set_project_warnings(magnesia_project_options ${MAGNESIA_WARNINGS_AS_ERRORS} "" "" "" "")
# cmake-format: on

option(MAGNESIA_ENABLE_SANITIZERS "Build with asan and ubsan" ${MAGNESIA_IS_NOT_RELEASE})
if(MAGNESIA_ENABLE_SANITIZERS)
    include(Sanitizers)
    # cmake-format: off
    magnesia_enable_sanitizers(magnesia_project_options ON ON ON OFF OFF)
    # cmake-format: on
endif()

option(MAGNESIA_BUILD_DOCS "Whether to build code documentation with doxygen" ON)
if(MAGNESIA_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
endif()

add_subdirectory(src/magnesia)
