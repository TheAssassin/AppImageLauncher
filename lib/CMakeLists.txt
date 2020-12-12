# required for pkg-config module's imported target feature
cmake_minimum_required(VERSION 3.6)

# note for self: include libappimage before AppImageUpdate to use the version here, not the one shipped with AppImageUpdate
# the AppImageUpdate submodule tends to be out of date
if(NOT USE_SYSTEM_LIBAPPIMAGE)
    add_subdirectory(libappimage EXCLUDE_FROM_ALL)
endif()

# used by the update helper
if(ENABLE_UPDATE_HELPER)
    set(BUILD_QT_UI On CACHE BOOL "" FORCE)
    add_subdirectory(AppImageUpdate EXCLUDE_FROM_ALL)
endif()

if(CI_BUILD)
    # TODO: use latest version of CMake once it contains the following change:
    # https://gitlab.kitware.com/cmake/cmake/commit/00a9d133fb2838ebb756d684659c5d51f577ede3
    # when this is available, the workaround here isn't necessary any more, instead install(TARGETS ...) can be used
    # and the BUILD_WITH_INSTALL_RPATH property doesn't need to be set any more
    message(WARNING "CI_BUILD is set to ON, binaries can only be tested after installation!")
    set(DEPS_RPATH "\$ORIGIN")
    set_property(TARGET libappimageupdate PROPERTY INSTALL_RPATH ${DEPS_RPATH})
    set_property(TARGET libappimageupdate PROPERTY BUILD_WITH_INSTALL_RPATH ON)
    set_property(TARGET libappimageupdate-qt PROPERTY INSTALL_RPATH ${DEPS_RPATH})
    set_property(TARGET libappimageupdate-qt PROPERTY BUILD_WITH_INSTALL_RPATH ON)
endif()
