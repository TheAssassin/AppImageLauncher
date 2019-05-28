# install configs for resource files

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/icons
    ${CMAKE_CURRENT_SOURCE_DIR}/mime
    DESTINATION ${CMAKE_INSTALL_DATADIR} COMPONENT APPIMAGELAUNCHER
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/appimagelauncher.desktop
    ${CMAKE_CURRENT_SOURCE_DIR}/appimagelaunchersettings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications COMPONENT APPIMAGELAUNCHER
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
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/appimagelauncher/maintainer-scripts
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
