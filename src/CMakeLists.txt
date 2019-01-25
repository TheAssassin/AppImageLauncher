# in version 3.6 IMPORTED_TARGET has been added to pkg_*_modules
cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib REQUIRED glib-2.0>=2.40 IMPORTED_TARGET)

find_package(INotify REQUIRED)

# disable Qt debug messages except for debug builds
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CMake build type is ${CMAKE_BUILD_TYPE}, hence disabling debug messages in AppImageLauncher")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
    add_definitions(-DQT_NO_INFO_OUTPUT)
    add_definitions(-DQT_NO_WARNING_OUTPUT)
endif()

# this shall propagate to all targets built in the subdirs
if(ENABLE_UPDATE_HELPER)
    add_definitions(-DENABLE_UPDATE_HELPER)
endif()

# define private libraries install destionation
if(IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    set(_libdir ${CMAKE_INSTALL_LIBDIR}/appimagelauncher)
else()
    set(_libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
endif()

set(_private_libdir ${_libdir}/appimagelauncher)

# calculate relative path from binary install destination to private library install dir
if(IS_ABSOLUTE ${CMAKE_INSTALL_BINDIR})
    set(_bindir ${CMAKE_INSTALL_BINDIR})
else()
    set(_bindir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
endif()

file(RELATIVE_PATH _rpath ${_bindir} ${_private_libdir})
set(_rpath "\$ORIGIN/${_rpath}")

# utility libraries
add_subdirectory(fswatcher)
add_subdirectory(i18n)
add_subdirectory(trashbin)
add_subdirectory(shared)

# appimagelauncherd
add_subdirectory(daemon)

# main application and helper tools
add_subdirectory(ui)

# FUSE filesystem AppImageLauncherFS
add_subdirectory(fusefs)
