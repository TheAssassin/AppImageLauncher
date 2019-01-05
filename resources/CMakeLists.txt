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
