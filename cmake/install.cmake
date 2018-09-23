# install libappimage.so into lib/appimagekit to avoid overwriting a libappimage potentially installed into /usr/lib
# or /usr/lib/x86_64-... or wherever the OS puts its libraries
# for some reason, using TARGETS ... doesn't work here, therefore using the absolute file path
install(
    FILES
    $<TARGET_FILE:libappimage>
    $<TARGET_LINKER_FILE:libappimage>
    $<TARGET_SONAME_FILE:libappimage>
    $<TARGET_FILE:libappimageinfo>
    $<TARGET_LINKER_FILE:libappimageinfo>
    $<TARGET_SONAME_FILE:libappimageinfo>
    $<TARGET_FILE:libappimageupdate>
    $<TARGET_LINKER_FILE:libappimageupdate>
    $<TARGET_SONAME_FILE:libappimageupdate>
    $<TARGET_FILE:libappimageupdate-qt>
    $<TARGET_LINKER_FILE:libappimageupdate-qt>
    $<TARGET_SONAME_FILE:libappimageupdate-qt>
    DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)

# also, install the runtime, which is needed to run AppImages
install(
    FILES ${PROJECT_BINARY_DIR}/lib/AppImageKit/src/runtime
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)

# TODO: find alternative to the following "workaround" (a pretty dirty hack, actually...)
# bundle update-binfmts as a fallback for distros which don't have it installed
find_program(UPDATE_BINFMTS
    NAMES update-binfmts
    PATHS /usr/sbin
)

if(NOT UPDATE_BINFMTS STREQUAL UPDATE_BINFMTS-NOTFOUND AND EXISTS ${UPDATE_BINFMTS})
    message(STATUS "Found update-binfmts, bundling: ${UPDATE_BINFMTS}")
    install(
        FILES /usr/sbin/update-binfmts
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
    )
else()
    message(WARNING "update-binfmts could not be found. Please install the binfmt-support package if you intend to build RPM packages.")
endif()

# binfmt.d config file -- used as a fallback, if update-binfmts is not available
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/binfmt.d/appimage.conf.in
    ${PROJECT_BINARY_DIR}/resources/binfmt.d/appimage.conf
    @ONLY
)

install(
    FILES ${PROJECT_BINARY_DIR}/resources/binfmt.d/appimage.conf
    DESTINATION lib/binfmt.d COMPONENT APPIMAGELAUNCHER
)

# install systemd service configuration for appimagelauncherd
configure_file(
    ${PROJECT_SOURCE_DIR}/resources/appimagelauncherd.service.in
    ${PROJECT_BINARY_DIR}/resources/appimagelauncherd.service
    @ONLY
)
install(
    FILES ${PROJECT_BINARY_DIR}/resources/appimagelauncherd.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/ COMPONENT APPIMAGELAUNCHER
)
