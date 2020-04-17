# install configs for resource files

# we depend on GNUInstallDirs, so let's make sure it's been loaded
include(GNUInstallDirs)

# debugging: should be share
message(STATUS "CMAKE_INSTALL_DATADIR: ${CMAKE_INSTALL_DATADIR}")

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/mime
    DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT APPIMAGELAUNCHER
)

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/ COMPONENT APPIMAGELAUNCHER
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/appimagelauncher.desktop
    ${CMAKE_CURRENT_SOURCE_DIR}/appimagelaunchersettings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications COMPONENT APPIMAGELAUNCHER
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fallback-icons
    DESTINATION ${CMAKE_INSTALL_DATADIR}/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
# we also need to copy the files into the binary dir to make their path predictable relative to the binary's location
file(
    COPY "${CMAKE_CURRENT_SOURCE_DIR}/fallback-icons"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)

set(INSTALL_MAINTAINER_SCRIPTS OFF CACHE BOOL "")

# install maintainer scripts that distro packagers might find useful
if (INSTALL_MAINTAINER_SCRIPTS)
    message(STATUS "Installing maintainer scripts")

    set(install_maintainer_scripts_dir ${PROJECT_BINARY_DIR}/cmake/install-maintainer-scripts/)

    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-install.in
        ${install_maintainer_scripts_dir}/post-install
        @ONLY
    )
    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/install-scripts/post-uninstall.in
        ${install_maintainer_scripts_dir}/post-uninstall
        @ONLY
    )

    install(
        FILES ${install_maintainer_scripts_dir}/post-install ${install_maintainer_scripts_dir}/post-uninstall
        DESTINATION ${CMAKE_INSTALL_DATADIR}/appimagelauncher/maintainer-scripts
    )
endif()


# install man page
set(INSTALL_MAN_PAGE ON CACHE BOOL "")

if(INSTALL_MAN_PAGE)
    set(configured_man_page_path ${PROJECT_BINARY_DIR}/cmake/man/AppImageLauncher.1)

    configure_file(
        ${PROJECT_SOURCE_DIR}/resources/AppImageLauncher.1.in
        ${configured_man_page_path}
    )

    install(
        FILES ${configured_man_page_path}
        DESTINATION share/man/man1 COMPONENT APPIMAGELAUNCHER
    )
endif()
