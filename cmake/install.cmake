# install libappimage.so into lib/appimagekit to avoid overwriting a libappimage potentially installed into /usr/lib
# or /usr/lib/x86_64-... or wherever the OS puts its libraries
install(FILES
    ${PROJECT_BINARY_DIR}/lib/AppImageKit/src/libappimage.so
    DESTINATION lib/appimagekit
    COMPONENT AppImageLauncher
)
