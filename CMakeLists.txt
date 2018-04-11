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
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE GIT_RESULT
    )

    if(GIT_RESULT AND GIT_RESULT EQUAL 0)
        message("Storing git commit ID in cache file")
        file(WRITE "${GIT_COMMIT_CACHE_FILE}" "${APPIMAGELAUNCHER_GIT_COMMIT}")
    endif()
endif()

if(NOT GIT_RESULT OR NOT GIT_RESULT EQUAL 0)
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
    message(FATAL_ERROR "Git commit ID cache file empty")
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
include(cmake/cpack_source.cmake)
include(cmake/cpack_deb.cmake)

# must be the last instruction
include(CPack)
