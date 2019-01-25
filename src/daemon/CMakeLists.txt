# daemon binary
add_executable(appimagelauncherd main.cpp worker.cpp worker.h)
target_link_libraries(appimagelauncherd shared filesystemwatcher PkgConfig::glib libappimage)
set_target_properties(appimagelauncherd PROPERTIES INSTALL_RPATH ${_rpath})

install(
    TARGETS
    appimagelauncherd
    RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${_libdir} COMPONENT APPIMAGELAUNCHER
)
