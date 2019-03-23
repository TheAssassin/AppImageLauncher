cmake_minimum_required(VERSION 3.2)

project(AppImageLauncher)

# versioning
include(cmake/versioning.cmake)

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
