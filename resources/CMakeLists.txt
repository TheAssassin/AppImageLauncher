# install configs for resource files

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/icons
    ${CMAKE_CURRENT_SOURCE_DIR}/mime
    DESTINATION share COMPONENT APPIMAGELAUNCHER
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/AppImageLauncher.desktop
    DESTINATION share/applications COMPONENT APPIMAGELAUNCHER
)
