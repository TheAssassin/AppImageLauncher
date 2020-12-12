# required for DEB-DEFAULT to work as intended
cmake_minimum_required(VERSION 3.6)

# Fix for https://github.com/TheAssassin/AppImageLauncher/issues/28
execute_process(
    COMMAND lsb_release -c
    OUTPUT_VARIABLE _lsb_release_output
)

if(_lsb_release_output MATCHES bionic)
    message(STATUS "platform is bionic, enabling compatibility mode for CPack Debian packaging")
    set(_compatibility_level bionic)
elseif(_lsb_release_output MATCHES cosmic)
    message(STATUS "platform is cosmic, enabling compatibility mode for CPack Debian packaging")
    set(_compatibility_level cosmic)
elseif(_lsb_release_output MATCHES disco)
    message(STATUS "platform is disco, enabling compatibility mode for CPack Debian packaging")
    set(_compatibility_level disco)
elseif(_lsb_release_output MATCHES eoan)
    message(STATUS "platform is eoan, enabling compatibility mode for CPack Debian packaging")
    set(_compatibility_level eoan)
else()
    set(_compatibility_level "")
endif()

set(CPACK_DEBIAN_COMPATIBILITY_LEVEL ${_compatibility_level} CACHE STRING "Available values: bionic (Ubuntu 18.04 LTS), cosmic (Ubuntu 18.10), disco (Ubuntu 19.04), eoan (Ubuntu 19.10)")

unset(_lsb_release_output)
unset(_compatibility_level)

# allow building Debian packages on non-Debian systems
if(DEFINED ENV{ARCH})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE $ENV{ARCH})
    if(CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "i686")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    elseif(CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "x86_64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    endif()
endif()

# make sure to package components separately
#set(CPACK_DEB_PACKAGE_COMPONENT ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

# override default package naming
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# Debian packaging global options
set(CPACK_DEBIAN_COMPRESSION_TYPE xz)

# use git hash as package release
set(CPACK_DEBIAN_PACKAGE_RELEASE "${APPIMAGELAUNCHER_GIT_COMMIT}")

# append build ID, similar to AppImage naming
if(DEFINED ENV{GITHUB_RUN_NUMBER})
    set(CPACK_DEBIAN_PACKAGE_RELEASE "gha$ENV{GITHUB_RUN_NUMBER}~${CPACK_DEBIAN_PACKAGE_RELEASE}")
else()
    set(CPACK_DEBIAN_PACKAGE_RELEASE "local~${CPACK_DEBIAN_PACKAGE_RELEASE}")
endif()

if(CPACK_DEBIAN_COMPATIBILITY_LEVEL)
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${CPACK_DEBIAN_PACKAGE_RELEASE}+${CPACK_DEBIAN_COMPATIBILITY_LEVEL}")
endif()

# bash is required to run install hooks
set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_PREDEPENDS bash)

# package name
set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_NAME "appimagelauncher")

# TODO: packagers watch out: you should set this to depend on a libappimage package, and avoid installing the library
# to a custom location in install.cmake

if(CPACK_DEBIAN_COMPATIBILITY_LEVEL STREQUAL "bionic" OR CPACK_DEBIAN_COMPATIBILITY_LEVEL STREQUAL "cosmic" OR CPACK_DEBIAN_COMPATIBILITY_LEVEL STREQUAL "disco" OR CPACK_DEBIAN_COMPATIBILITY_LEVEL STREQUAL "eoan")
    set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.2.1), libqt5gui5 (>= 5.2.1), libqt5core5a (>= 5.2.1), binfmt-support (>= 2.0), systemd, libcurl4")
else()
    set(CPACK_DEBIAN_APPIMAGELAUNCHER_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.2.1), libqt5gui5 (>= 5.2.1), libqt5core5a (>= 5.2.1), binfmt-support (>= 2.0), systemd, libcurl3")
endif()

# improve dependency list
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# add postinst and prerm hooks to Debian package
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-install.in
    ${PROJECT_BINARY_DIR}/cmake/debian/postinst
    @ONLY
)
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-uninstall.in
    ${PROJECT_BINARY_DIR}/cmake/debian/postrm
    @ONLY
)

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${PROJECT_BINARY_DIR}/cmake/debian/postinst"
    "${PROJECT_BINARY_DIR}/cmake/debian/postrm"
)
