cmake_minimum_required(VERSION 3.2)

project(AppImageLauncher)

set(V_MAJOR 1)
set(V_MINOR 1)
set(V_PATCH 0)
set(V_SUFFIX "")

set(APPIMAGELAUNCHER_VERSION ${V_MAJOR}.${V_MINOR}.${V_PATCH}${V_SUFFIX})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

include(cmake/scripts.cmake)

include(cmake/reproducible_builds.cmake)

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

# check whether git is available
find_program(GIT git)
set(GIT_COMMIT_CACHE_FILE "${PROJECT_SOURCE_DIR}/cmake/GIT_COMMIT")

if(NOT GIT STREQUAL "GIT-NOTFOUND")
    # read Git revision ID
    # WARNING: this value will be stored in the CMake cache
    # to update it, you will have to reset the CMake cache
    # (doesn't matter for CI builds like Travis for instance, where there's no permanent CMake cache)
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE APPIMAGELAUNCHER_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_RESULT
    )

    if(GIT_RESULT AND GIT_RESULT EQUAL 0)
        message("Storing git commit ID in cache file")
        file(WRITE "${GIT_COMMIT_CACHE_FILE}" "${APPIMAGELAUNCHER_GIT_COMMIT}")
    endif()
endif()

if(NOT GIT_RESULT EQUAL 0)
    # git call failed or git hasn't been found, might happen when calling CMake in an extracted source tarball
    # therefore we try to find the git commit cache file
    # if it doesn't exist, refuse to configure

    message(WARNING "Failed to gather commit ID via git command, trying to read cache file")
    if(EXISTS "${GIT_COMMIT_CACHE_FILE}")
        file(READ "${GIT_COMMIT_CACHE_FILE}" APPIMAGELAUNCHER_CACHED_GIT_COMMIT)
        mark_as_advanced(FORCE APPIMAGELAUNCHER_CACHED_GIT_COMMIT)
        string(REPLACE "\n" "" APPIMAGELAUNCHER_GIT_COMMIT "${APPIMAGELAUNCHER_CACHED_GIT_COMMIT}")
    else()
        message(FATAL_ERROR "Could not find git commit cache file, git commit ID not available for versioning")
    endif()
endif()

if("${APPIMAGELAUNCHER_GIT_COMMIT}" STREQUAL "")
    message(FATAL_ERROR "Invalid git commit ID: ${APPIMAGELAUNCHER_GIT_COMMIT}")
endif()

message(STATUS "Git commit: ${APPIMAGELAUNCHER_GIT_COMMIT}")
mark_as_advanced(FORCE APPIMAGELAUNCHER_GIT_COMMIT)

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

# get current date, short form
execute_process(
    COMMAND env LC_ALL=C git show -s --format=%ci ${APPIMAGELAUNCHER_GIT_COMMIT}
    OUTPUT_VARIABLE APPIMAGELAUNCHER_GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
execute_process(
    COMMAND env LC_ALL=C date -u "+%Y%m%d" -d ${APPIMAGELAUNCHER_GIT_COMMIT_DATE}
    OUTPUT_VARIABLE APPIMAGELAUNCHER_GIT_COMMIT_DATE_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_definitions(-DAPPIMAGELAUNCHER_VERSION="${APPIMAGELAUNCHER_VERSION}")
add_definitions(-DAPPIMAGELAUNCHER_GIT_COMMIT="${APPIMAGELAUNCHER_GIT_COMMIT}")
add_definitions(-DAPPIMAGELAUNCHER_BUILD_NUMBER="${APPIMAGELAUNCHER_BUILD_NUMBER}")
add_definitions(-DAPPIMAGELAUNCHER_BUILD_DATE="${APPIMAGELAUNCHER_BUILD_DATE}")

# used by Debian packaging infrastructure
include(GNUInstallDirs)

# if there's a system libappimage package on the system, we can use that directly
set(USE_SYSTEM_LIBAPPIMAGE OFF CACHE BOOL "")

# if the system version should be used, import globally _before_ including third-party stuff and own executables/libs
if(USE_SYSTEM_LIBAPPIMAGE)
    find_package(libappimage REQUIRED)
endif()

# optional; if AppImageUpdate dependency is not viable, the update helper can be disabled
set(ENABLE_UPDATE_HELPER ON CACHE BOOL "")

# install resources, bundle libraries privately, etc.
# initializes important installation destination variables, therefore must be included before adding subdirectories
include(cmake/install.cmake)

add_subdirectory(lib)
add_subdirectory(src)

# contains install configs for resource files
add_subdirectory(resources)

# translation management
add_subdirectory(i18n)

# packaging
include(cmake/cpack_general.cmake)
include(cmake/cpack_source.cmake)
include(cmake/cpack_deb.cmake)
include(cmake/cpack_rpm.cmake)

# must be the last instruction
include(CPack)
