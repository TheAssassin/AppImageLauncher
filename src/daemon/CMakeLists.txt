# appimagelauncherd
add_executable(appimagelauncherd
    main.cpp
    daemon_worker.cpp
    daemon_worker.h)

target_link_libraries(appimagelauncherd shared filesystemwatcher PkgConfig::glib libappimage)

# set binary runtime rpath to make sure the libappimage.so built and installed by this project is going to be used
# by the installed binaries (be it the .deb, the AppImage, or whatever)
# in order to make the whole install tree relocatable, a relative path is used
file(RELATIVE_PATH LIBS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/bin ${CMAKE_INSTALL_PREFIX}/lib/appimagelauncher)
set_target_properties(AppImageLauncher PROPERTIES INSTALL_RPATH "\$ORIGIN/${LIBS_INSTALL_PATH}")
set_target_properties(appimagelauncherd PROPERTIES INSTALL_RPATH "\$ORIGIN/${LIBS_INSTALL_PATH}")

# add dependency on runtime to make sure it's built
add_dependencies(AppImageLauncher runtime)
add_dependencies(appimagelauncherd runtime)

install(
    TARGETS
    appimagelauncherd
    RUNTIME DESTINATION bin COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION lib COMPONENT APPIMAGELAUNCHER
)
