# main AppImageLauncher application
add_executable(AppImageLauncher
    main.cpp
    Launcher.cpp
    Launcher.h
    ui.ui
    ui.h
    ui.cpp)

target_link_libraries(AppImageLauncher shared PkgConfig::glib libappimage libappimageinfo nlohmann_json)

install(
    TARGETS
    AppImageLauncher
    RUNTIME DESTINATION bin COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION lib COMPONENT APPIMAGELAUNCHER
)