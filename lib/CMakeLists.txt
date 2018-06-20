# prevent magic bytes from being inserted into AppImage runtime
set(APPIMAGEKIT_EMBED_MAGIC_BYTES OFF CACHE BOOL "" FORCE)
# enable libbsd powered setting of proc title in the runtime when using $TARGET_APPIMAGE (which we do) in order to
# allow users to distinguish between the AppImages run with the AppImageLauncher runtime
set(APPIMAGEKIT_RUNTIME_ENABLE_SETPROCTITLE ON CACHE BOOL "" FORCE)

# needed to build libappimage and AppImage runtime
add_subdirectory(AppImageKit EXCLUDE_FROM_ALL)

# used by the update helper
add_subdirectory(AppImageUpdate EXCLUDE_FROM_ALL)

if(TRAVIS_BUILD)
    # TODO: use latest version of CMake once it contains the following change:
    # https://gitlab.kitware.com/cmake/cmake/commit/00a9d133fb2838ebb756d684659c5d51f577ede3
    # when this is available, the workaround here isn't necessary any more, instead install(TARGETS ...) can be used
    # and the BUILD_WITH_INSTALL_RPATH property doesn't need to be set any more
    message(WARNING "TRAVIS_BUILD is set to ON, binaries can only be tested after installation!")
    set(DEPS_RPATH "\$ORIGIN")
    set_property(TARGET libappimageupdate PROPERTY INSTALL_RPATH ${DEPS_RPATH})
    set_property(TARGET libappimageupdate PROPERTY BUILD_WITH_INSTALL_RPATH ON)
    set_property(TARGET libappimageupdate-qt PROPERTY INSTALL_RPATH ${DEPS_RPATH})
    set_property(TARGET libappimageupdate-qt PROPERTY BUILD_WITH_INSTALL_RPATH ON)
endif()
