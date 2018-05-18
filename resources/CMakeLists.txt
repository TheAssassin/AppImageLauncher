# install configs for resource files

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/icons
    ${CMAKE_CURRENT_SOURCE_DIR}/mime
    DESTINATION share COMPONENT APPIMAGELAUNCHER
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/appimagelauncher.desktop
    DESTINATION share/applications COMPONENT APPIMAGELAUNCHER
)

# empty directories aren't tracked by Git
# therefore the directory needs to be created by CMake
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/l10n)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/l10n
    DESTINATION share/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
