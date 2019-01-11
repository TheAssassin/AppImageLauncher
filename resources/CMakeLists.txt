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
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications COMPONENT APPIMAGELAUNCHER
)

# empty directories aren't tracked by Git
# therefore the directory needs to be created by CMake
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/l10n)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/l10n
    DESTINATION ${CMAKE_INSTALL_DATADIR}/appimagelauncher COMPONENT APPIMAGELAUNCHER
)


set(INSTALL_MAINTAINER_SCRIPTS OFF CACHE BOOL "")

# install maintainer scripts for
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
