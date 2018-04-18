# install libappimage.so into lib/appimagekit to avoid overwriting a libappimage potentially installed into /usr/lib
# or /usr/lib/x86_64-... or wherever the OS puts its libraries
# for some reason, using TARGETS ... doesn't work here, therefore using the absolute file path
install(
    FILES
    ${PROJECT_BINARY_DIR}/lib/AppImageKit/src/libappimage.so
    ${PROJECT_BINARY_DIR}/lib/AppImageUpdate/src/libappimageupdate.so
    ${PROJECT_BINARY_DIR}/lib/AppImageUpdate/src/qt-ui/libappimageupdate-qt.so
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
install(
    FILES /usr/sbin/update-binfmts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
