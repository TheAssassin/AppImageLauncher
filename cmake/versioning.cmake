set(V_MAJOR 2)
set(V_MINOR 2)
set(V_PATCH 0)
set(V_SUFFIX "")

set(APPIMAGELAUNCHER_VERSION ${V_MAJOR}.${V_MINOR}.${V_PATCH}${V_SUFFIX})

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

    if(GIT_RESULT EQUAL 0)
        message(STATUS "Storing git commit ID in cache file")
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

# add build number based on GitHub actions build number if possible
if("$ENV{GITHUB_RUN_NUMBER}" STREQUAL "")
    set(APPIMAGELAUNCHER_BUILD_NUMBER "<local dev build>")
else()
    set(APPIMAGELAUNCHER_BUILD_NUMBER "$ENV{GITHUB_RUN_NUMBER}")
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
