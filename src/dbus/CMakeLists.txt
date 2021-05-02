function(common_library_options name)
    target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${name} PUBLIC
        -DAPPIMAGELAUNCHERD_DBUS_SERVICE_NAME="com.github.TheAssassin.AppImageLauncher.appimagelauncherd"
    )
    target_link_libraries(dbus_core PUBLIC Qt5::Core Qt5::DBus)
endfunction()


add_library(dbus_core STATIC dbus_exceptions.cpp dbus_exceptions.h)
common_library_options(dbus_core)


function(add_dbus_library name)
    add_library(${name} STATIC ${name}.h ${name}.cpp)
    common_library_options(${name})
    target_link_libraries(${name} PUBLIC dbus_core)
endfunction()

add_dbus_library(DBusServer)
target_link_libraries(DBusServer PRIVATE shared PkgConfig::glib libappimage)

add_dbus_library(DBusClient)
