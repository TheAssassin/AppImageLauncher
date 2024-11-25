# required by file(CHMOD ...)
cmake_minimum_required(VERSION 3.19)

# define private libraries install destination
include(GNUInstallDirs)

# debugging: libdir should be lib/<triplet>
message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")

if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    set(_libdir ${CMAKE_INSTALL_LIBDIR})
else()
    file(RELATIVE_PATH _libdir ${CMAKE_INSTALL_LIBDIR} ${CMAKE_INSTALL_PREFIX})
endif()

set(_private_libdir ${_libdir}/appimagelauncher)

# calculate relative path from binary install destination to private library install dir
if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_BINDIR})
    set(_bindir ${CMAKE_INSTALL_BINDIR})
else()
    file(RELATIVE_PATH _bindir ${CMAKE_INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})
    #set(_bindir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
endif()

set(_abs_bindir ${CMAKE_INSTALL_PREFIX}/${_bindir})
set(_abs_private_libdir ${CMAKE_INSTALL_PREFIX}/${_private_libdir})

file(RELATIVE_PATH _rpath ${_abs_bindir} ${_abs_private_libdir})
set(_rpath "\$ORIGIN/${_rpath}")

# install libappimage.so into lib/appimagekit to avoid overwriting a libappimage potentially installed into /usr/lib
# or /usr/lib/x86_64-... or wherever the OS puts its libraries
install(
    TARGETS libappimage
    LIBRARY DESTINATION "${_private_libdir}" COMPONENT APPIMAGELAUNCHER
)

if(ENABLE_UPDATE_HELPER)
    install(
        TARGETS libappimageupdate libappimageupdate-qt
        LIBRARY DESTINATION "${_private_libdir}" COMPONENT APPIMAGELAUNCHER
    )
endif()

if(NOT BUILD_LITE)
    # unfortunately, due to a cyclic dependency, we need to hardcode parts of this variable, which is included in the
    # install scripts and the binfmt.d config
    set(BINFMT_INTERPRETER_PATH ${CMAKE_INSTALL_PREFIX}/${_private_libdir}/binfmt-interpreter)

    # according to https://www.kernel.org/doc/html/latest/admin-guide/binfmt-misc.html, we must make sure the
    # interpreter string does not exceed 127 characters
    set(BINFMT_INTERPRETER_PATH_LENGTH_MAX 127)
    string(LENGTH BINFMT_INTERPRETER_PATH BINFMT_INTERPRETER_PATH_LENGTH)

    if(BINFMT_INTERPRETER_PATH_LENGTH GREATER BINFMT_INTERPRETER_PATH_LENGTH_MAX)
        message(FATAL_ERROR "interpreter path exceeds maximum length of ${BINFMT_INTERPRETER_PATH_LENGTH_MAX}")
    endif()

    # binfmt.d config file -- used as a fallback, if update-binfmts is not available
    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/binfmt.d/appimagelauncher.conf.in
        ${PROJECT_BINARY_DIR}/resources/binfmt.d/appimagelauncher.conf
        @ONLY
    )
    # caution: don't use ${CMAKE_INSTALL_LIBDIR} here, it's really just lib/binfmt.d
    install(
        FILES ${PROJECT_BINARY_DIR}/resources/binfmt.d/appimagelauncher.conf
        DESTINATION lib/binfmt.d COMPONENT APPIMAGELAUNCHER
    )

    # prepare postinst and prerm hooks to Debian package
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
endif()

# install systemd service configuration for appimagelauncherd
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/appimagelauncherd.service.in
    ${PROJECT_BINARY_DIR}/resources/appimagelauncherd.service
    @ONLY
)
# caution: don't use ${CMAKE_INSTALL_LIBDIR} here, it's really just lib/systemd/user
install(
    FILES ${PROJECT_BINARY_DIR}/resources/appimagelauncherd.service
    DESTINATION lib/systemd/user/ COMPONENT APPIMAGELAUNCHER
)
