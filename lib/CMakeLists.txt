# prevent magic bytes from being inserted into AppImage runtime
set(APPIMAGEKIT_EMBED_MAGIC_BYTES OFF CACHE BOOL "" FORCE)

# needed to build libappimage and AppImage runtime
add_subdirectory(AppImageKit EXCLUDE_FROM_ALL)

# used by the update helper
add_subdirectory(AppImageUpdate EXCLUDE_FROM_ALL)
