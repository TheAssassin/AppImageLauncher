cmake_minimum_required(VERSION 3.5)

project(AppImageLauncher)

# versioning
include(cmake/versioning.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

include(cmake/scripts.cmake)

include(cmake/reproducible_builds.cmake)

# support for ccache
# call CMake with -DUSE_CCACHE=ON to make use of it
option(USE_CCACHE OFF)
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

# used by Debian packaging infrastructure
include(GNUInstallDirs)

# if there's a system libappimage package on the system, we can use that directly
option(USE_SYSTEM_LIBAPPIMAGE OFF)

# if the system version should be used, import globally _before_ including third-party stuff and own executables/libs
if(USE_SYSTEM_LIBAPPIMAGE)
    find_package(libappimage REQUIRED)
endif()

include(FetchContent)

# AppImageUpdate is needed for the updater UI (and libappimage)
FetchContent_Declare(AppImageUpdate
    GIT_REPOSITORY https://github.com/AppImageCommunity/AppImageUpdate.git
    GIT_TAG 2.0.0-alpha-1-20251018
    EXCLUDE_FROM_ALL
)

# work around Wimplicit-function-declaration in ancient squashfuse code
set(DEPENDENCIES_CFLAGS "-Wno-implicit-function-declaration" CACHE STRING "" FORCE)

option(ENABLE_UPDATE_HELPER "" ON)
if(ENABLE_UPDATE_HELPER)
    # instruct AppImageUpdate to build the Qt UI
    set(BUILD_QT_UI ON CACHE BOOL "" FORCE)
endif()

# note: for the time being, we require AppImageUpdate to be fetched during build, even if only to make libappimage available
FetchContent_MakeAvailable(AppImageUpdate)

# choose what init system to target (for daemon user service)
# set by adding -DSERVICE_TYPE="<type>" when calling cmake
# options: systemd, openrc
set(SERVICE_TYPE "systemd" CACHE STRING "init service provider to target")

# install resources, bundle libraries privately, etc.
# initializes important installation destination variables, therefore must be included before adding subdirectories
include(cmake/install.cmake)

add_subdirectory(src)

# contains install configs for resource files
add_subdirectory(resources)

# translation management
add_subdirectory(i18n)
