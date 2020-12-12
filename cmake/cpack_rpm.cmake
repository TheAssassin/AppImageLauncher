# required for RPM-DEFAULT to work as intended
cmake_minimum_required(VERSION 3.6)

# allow building RPM packages on non-RPM systems
if(DEFINED ENV{ARCH})
    set(CPACK_RPM_PACKAGE_ARCHITECTURE $ENV{ARCH})
    if(CPACK_RPM_PACKAGE_ARCHITECTURE MATCHES "i686")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "i386")
    elseif(CPACK_RPM_PACKAGE_ARCHITECTURE MATCHES "amd64")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    endif()
endif()

# override default package naming
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# make sure to package components separately
#set(CPACK_DEB_PACKAGE_COMPONENT ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

# RPM packaging global options
set(CPACK_RPM_COMPRESSION_TYPE xz)

# use git hash as package release
set(CPACK_RPM_PACKAGE_RELEASE "${APPIMAGELAUNCHER_GIT_COMMIT}")

# append build ID, similar to AppImage naming
if(DEFINED ENV{GITHUB_RUN_NUMBER})
    set(CPACK_RPM_PACKAGE_RELEASE "gha$ENV{GITHUB_RUN_NUMBER}~${CPACK_RPM_PACKAGE_RELEASE}")
else()
    set(CPACK_RPM_PACKAGE_RELEASE "local~${CPACK_RPM_PACKAGE_RELEASE}")
endif()

# bash is required to run install hooks
set(CPACK_RPM_APPIMAGELAUNCHER_REQUIRES_PRE "bash")

# package name
set(CPACK_RPM_APPIMAGELAUNCHER_PACKAGE_NAME "appimagelauncher")

# TODO: packagers watch out: you should set this to depend on a libappimage package, and avoid installing the library
# to a custom location in install.cmake
set(CPACK_RPM_APPIMAGELAUNCHER_PACKAGE_DREQUIRES "pkgconfig(Qt5Widgets) (>= 5.2.1), pkgconfig(Qt5Gui) (>= 5.2.1), pkgconfig(Qt5Core) (>= 5.2.1), libcurl4")
# deactivate automatic generation of dependency list
# it wants to add libappimage.so to the list of dependencies, which is actually shipped by the package
set(CPACK_RPM_PACKAGE_AUTOREQ OFF)

# avoid conflicts with the filesystem package
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    /usr/share/applications
    /usr/lib/binfmt.d
    /usr/lib/systemd
    /usr/lib/systemd/user
    /usr/share/man
    /usr/share/man/man1
)

# add postinst and prerm hooks to RPM package
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-install.in
    ${PROJECT_BINARY_DIR}/cmake/rpm/post-install
    @ONLY
)
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-uninstall.in
    ${PROJECT_BINARY_DIR}/cmake/rpm/post-uninstall
    @ONLY
)

set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${PROJECT_BINARY_DIR}/cmake/rpm/post-install")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${PROJECT_BINARY_DIR}/cmake/rpm/post-uninstall")
