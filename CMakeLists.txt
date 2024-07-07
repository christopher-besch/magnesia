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
FetchContent_Declare(
    open62541pp
    GIT_REPOSITORY https://github.com/open62541pp/open62541pp.git
    GIT_TAG 6b83f185e0fbc78f98647ff4afd848b8a17cbc60 # v0.13.0
    GIT_SHALLOW ON
    SYSTEM ${EXCLUDE_FROM_ALL}
    FIND_PACKAGE_ARGS 0.13
)
message(STATUS "Fetching dependencies. This may take a while...")
FetchContent_MakeAvailable(open62541pp)

# qt6-base-dev in current debian stable (bookworm) is at least 6.4.2
find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_subdirectory(src/magnesia)
