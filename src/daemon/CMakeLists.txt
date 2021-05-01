# daemon binary
add_executable(appimagelauncherd main.cpp worker.cpp worker.h)
target_link_libraries(appimagelauncherd shared filesystemwatcher Qt5::DBus PkgConfig::glib libappimage)
set_target_properties(appimagelauncherd PROPERTIES INSTALL_RPATH ${_rpath})
target_compile_definitions(appimagelauncherd PRIVATE -DAPPIMAGELAUNCHERD_DBUS_SERVICE_NAME="${APPIMAGELAUNCHERD_DBUS_SERVICE_NAME}")

install(
    TARGETS
    appimagelauncherd
    RUNTIME DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${_libdir} COMPONENT APPIMAGELAUNCHER
)
