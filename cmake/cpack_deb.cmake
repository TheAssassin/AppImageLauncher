# required for DEB-DEFAULT to work as intended
cmake_minimum_required(VERSION 3.6)

set(PROJECT_VERSION 1.0)
set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_DEBUG ON)

# make sure to only package APPIMAGELAUNCHER component
set(CPACK_COMPONENTS_ALL APPIMAGELAUNCHER)

# global options
set(CPACK_PACKAGE_CONTACT "TheAssassin")
set(CPACK_PACKAGE_HOMEPAGE "https://github.com/TheAssassin/AppImageLauncher")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

# allow building Debian packages on non-Debian systems
if(DEFINED ENV{ARCH})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE $ENV{ARCH})
    if(CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "i686")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    endif()
endif()

# make sure to package components separately
#set(CPACK_DEB_PACKAGE_COMPONENT ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

# override default package naming
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# Debian packaging global options
set(CPACK_DEBIAN_COMPRESSION_TYPE xz)

# versioning
# it appears setting CPACK_DEBIAN_PACKAGE_VERSION doesn't work, hence setting CPACK_PACKAGE_VERSION
set(CPACK_PACKAGE_VERSION ${APPIMAGELAUNCHER_VERSION})

# use git hash as package release
set(CPACK_DEBIAN_PACKAGE_RELEASE "git${APPIMAGELAUNCHER_GIT_COMMIT}")

# append build ID, similar to AppImage naming
if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${CPACK_DEBIAN_PACKAGE_RELEASE}-travis$ENV{TRAVIS_BUILD_NUMBER}")
else()
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${CPACK_DEBIAN_PACKAGE_RELEASE}-local")
endif()

# bash is required to run install hooks
set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_PREDEPENDS bash)

# package name
set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_NAME "appimagelauncher")

# TODO: packagers watch out: you should set this to depend on a libappimage package, and avoid installing the library
# to a custom location in install.cmake
set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.2.1), libqt5gui5 (>= 5.2.1), libqt5core5a (>= 5.2.1), binfmt-support (>= 2.0)")
# improve dependency list
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# TODO: insert some useful description
set(CPACK_COMPONENT_APPIMAGELAUNCHER_PACKAGE_DESCRIPTION "AppImageLauncher")

# find more suitable section for package
set(CPACK_COMPONENT_APPIMAGELAUNCHER_PACKAGE_SECTION misc)

# add postinst and prerm hooks to Debian package
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${PROJECT_SOURCE_DIR}/cmake/debian/postinst;${PROJECT_SOURCE_DIR}/cmake/debian/prerm")

# must be the last instruction
include(CPack)
