# main AppImageLauncher application
add_executable(AppImageLauncher main.cpp)
target_link_libraries(AppImageLauncher shared PkgConfig::glib libappimage shared)

# set binary runtime rpath to make sure the libappimage.so built and installed by this project is going to be used
# by the installed binaries (be it the .deb, the AppImage, or whatever)
# in order to make the whole install tree relocatable, a relative path is used
set_target_properties(AppImageLauncher PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/appimagelauncher")

# add dependency on AppImageKit's runtime to make sure it's built before/along with appimagelauncherd
add_dependencies(appimagelauncherd runtime)

install(
    TARGETS
    AppImageLauncher
    RUNTIME DESTINATION bin COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION lib COMPONENT APPIMAGELAUNCHER
)


# AppImage removal helper
add_executable(remove remove_main.cpp remove.ui)
target_link_libraries(remove shared translationmanager libappimage)
# see AppImageLauncher for a description
set_target_properties(remove PROPERTIES INSTALL_RPATH "\$ORIGIN")


# AppImage update helper
add_executable(update update_main.cpp)
target_link_libraries(update shared translationmanager libappimage libappimageupdate-qt)
# see AppImageLauncher for a description
set_target_properties(update PROPERTIES INSTALL_RPATH "\$ORIGIN")

install(
    TARGETS remove update
    RUNTIME DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
