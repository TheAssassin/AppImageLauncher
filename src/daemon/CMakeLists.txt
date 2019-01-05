# daemon binary
add_executable(appimagelauncherd main.cpp worker.cpp worker.h)
target_link_libraries(appimagelauncherd shared filesystemwatcher PkgConfig::glib libappimage)
set_target_properties(appimagelauncherd PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/appimagelauncher")

install(
    TARGETS
    appimagelauncherd
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT APPIMAGELAUNCHER
)
