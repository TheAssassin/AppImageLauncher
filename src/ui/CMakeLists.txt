function(install_legacy_helper_symlink legacy_name target_name)
    if(NOT BUILD_LITE)
        set(legacy_helper_path "\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${_private_libdir}/${legacy_name}")
        install(CODE "
            file(REMOVE \"${legacy_helper_path}\")
            file(CREATE_LINK
                \"${_bindir_from_private_libdir}/${target_name}\"
                \"${legacy_helper_path}\"
                SYMBOLIC
            )
        " COMPONENT APPIMAGELAUNCHER)
    endif()
endfunction()

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
add_executable(appimagelauncher-remove remove_main.cpp remove.ui resources.qrc)
target_link_libraries(appimagelauncher-remove shared translationmanager libappimage)
# see AppImageLauncher for a description
set_target_properties(appimagelauncher-remove PROPERTIES INSTALL_RPATH ${_rpath})

install(
    TARGETS
    appimagelauncher-remove
    RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
)

install_legacy_helper_symlink(remove appimagelauncher-remove)


# AppImage update helper
if(ENABLE_UPDATE_HELPER)
    add_executable(appimagelauncher-update update.ui update_main.cpp resources.qrc)
    target_link_libraries(appimagelauncher-update shared translationmanager libappimage libappimageupdate-qt Qt5::Quick Qt5::QuickWidgets Qt5::Qml)
    # see AppImageLauncher for a description
    set_target_properties(appimagelauncher-update PROPERTIES INSTALL_RPATH ${_rpath})

    install(
        TARGETS
        appimagelauncher-update
        RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
    )

    install_legacy_helper_symlink(update appimagelauncher-update)
endif()
