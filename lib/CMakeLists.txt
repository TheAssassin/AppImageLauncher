# prevent magic bytes from being inserted into AppImage runtime
set(APPIMAGEKIT_EMBED_MAGIC_BYTES OFF CACHE BOOL "" FORCE)
# enable libbsd powered setting of proc title in the runtime when using $TARGET_APPIMAGE (which we do) in order to
# allow users to distinguish between the AppImages run with the AppImageLauncher runtime
set(APPIMAGEKIT_RUNTIME_ENABLE_SETPROCTITLE ON CACHE BOOL "" FORCE)

# needed to build libappimage and AppImage runtime
add_subdirectory(AppImageKit EXCLUDE_FROM_ALL)

# used by the update helper
add_subdirectory(AppImageUpdate EXCLUDE_FROM_ALL)
