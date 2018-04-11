cmake_minimum_required(VERSION 3.2)

project(AppImageLauncher)

set(APPIMAGELAUNCHER_VERSION 1-alpha)

# support for ccache
# call CMake with -DUSE_CCACHE=ON to make use of it
set(USE_CCACHE OFF CACHE BOOL "")
if(USE_CCACHE)
    find_program(CCACHE ccache)
    if(CCACHE)
        message(STATUS "Using ccache")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
    else()
        message(WARNING "USE_CCACHE set, but could not find ccache")
    endif()
endif()

# read Git revision ID
# WARNING: this value will be stored in the CMake cache
# to update it, you will have to reset the CMake cache
# (doesn't matter for CI builds like Travis for instance, where there's no permanent CMake cache)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE APPIMAGELAUNCHER_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# set version and build number
set(VERSION 1-alpha)
if("$ENV{TRAVIS_BUILD_NUMBER}" STREQUAL "")
    set(APPIMAGELAUNCHER_BUILD_NUMBER "<local dev build>")
else()
    set(APPIMAGELAUNCHER_BUILD_NUMBER "$ENV{TRAVIS_BUILD_NUMBER}")
endif()

# get current date
execute_process(
    COMMAND env LC_ALL=C date -u "+%Y-%m-%d %H:%M:%S %Z"
    OUTPUT_VARIABLE APPIMAGELAUNCHER_BUILD_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions(-DAPPIMAGELAUNCHER_VERSION="${APPIMAGELAUNCHER_VERSION}")
add_definitions(-DAPPIMAGELAUNCHER_GIT_COMMIT="${APPIMAGELAUNCHER_GIT_COMMIT}")
add_definitions(-DAPPIMAGELAUNCHER_BUILD_NUMBER="${APPIMAGELAUNCHER_BUILD_NUMBER}")
add_definitions(-DAPPIMAGELAUNCHER_BUILD_DATE="${APPIMAGELAUNCHER_BUILD_DATE}")

add_subdirectory(lib)
add_subdirectory(src)

# contains global options
include(cmake/install.cmake)

# contains install configs for resource files
add_subdirectory(resources)

# packaging
include(cmake/cpack_general.cmake)
include(cmake/cpack_deb.cmake)

# must be the last instruction
include(CPack)
