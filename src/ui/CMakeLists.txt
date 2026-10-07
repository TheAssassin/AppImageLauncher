if(NOT BUILD_LITE)
    # main AppImageLauncher application
    add_executable(AppImageLauncher main.cpp resources.qrc first-run.cpp first-run.h first-run.ui integration_dialog.cpp integration_dialog.h integration_dialog.ui)
    target_link_libraries(AppImageLauncher shared PkgConfig::glib libappimage shared)

    # set binary runtime rpath to make sure the libappimage.so built and installed by this project is going to be used
    # by the installed binaries (be it the .deb, the AppImage, or whatever)
    # in order to make the whole install tree relocatable, a relative path is used
    set_target_properties(AppImageLauncher PROPERTIES INSTALL_RPATH ${_rpath})

    install(
        TARGETS
        AppImageLauncher
        RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
        LIBRARY DESTINATION ${_libdir} COMPONENT APPIMAGELAUNCHER
    )
endif()

# AppImageLauncherSettings application
add_executable(AppImageLauncherSettings settings_main.cpp resources.qrc settings_dialog.ui settings_dialog.cpp)
target_link_libraries(AppImageLauncherSettings shared)

# set binary runtime rpath to make sure the libappimage.so built and installed by this project is going to be used
# by the installed binaries (be it the .deb, the AppImage, or whatever)
# in order to make the whole install tree relocatable, a relative path is used
set_target_properties(AppImageLauncherSettings PROPERTIES INSTALL_RPATH ${_rpath})

install(
    TARGETS
    AppImageLauncherSettings
    RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${_libdir} COMPONENT APPIMAGELAUNCHER
)

# AppImage removal helper
add_executable(remove remove_main.cpp remove.ui resources.qrc)
target_link_libraries(remove shared translationmanager libappimage)
# see AppImageLauncher for a description
# in addition to $ORIGIN (where libappimage.so etc. are installed to), we also need the flat lib dir (see
# _rpath_flat_libdir in install.cmake), as that's where linuxdeploy bundles transitive dependencies (e.g.,
# libgpgme, libgpg-error, libassuan pulled in by libappimageupdate) when deploying the AppDir; without it, those
# libraries silently fail to load at runtime
set_target_properties(remove PROPERTIES INSTALL_RPATH "\$ORIGIN;${_rpath_flat_libdir}")

install(
    TARGETS
    remove
    RUNTIME DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
)


# AppImage update helper
if(ENABLE_UPDATE_HELPER)
    add_executable(update update.ui update_main.cpp resources.qrc)
    target_link_libraries(update shared translationmanager libappimage libappimageupdate-qt Qt5::Quick Qt5::QuickWidgets Qt5::Qml)
    # see remove target above for a description of why the flat lib dir is included, too
    set_target_properties(update PROPERTIES INSTALL_RPATH "\$ORIGIN;${_rpath_flat_libdir}")

    install(
        TARGETS
        update
        RUNTIME DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
    )
endif()
