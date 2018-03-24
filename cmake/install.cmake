# install libappimage.so into lib/appimagekit to avoid overwriting a libappimage potentially installed into /usr/lib
# or /usr/lib/x86_64-... or wherever the OS puts its libraries
# also, install the runtime, which is needed to run AppImages
install(FILES
    ${PROJECT_BINARY_DIR}/lib/AppImageKit/src/libappimage.so
    DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
install(FILES
    ${PROJECT_BINARY_DIR}/lib/AppImageKit/src/runtime
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
