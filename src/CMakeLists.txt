cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.40)
include_directories(${GLIB_INCLUDE_DIRS})

add_executable(AppImageLauncher main.cpp)
target_link_libraries(AppImageLauncher Qt5::Core Qt5::Widgets ${GLIB_LIBRARIES} libappimage xdg-basedir)
target_compile_definitions(AppImageLauncher
    PRIVATE -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
)
# set binary runtime rpath to make sure the libappimage.so built and installed by this project is going to be used
# by the installed binaries (be it the .deb, the AppImage, or whatever)
# in order to make the whole install tree relocatable, a relative path is used
set_target_properties(AppImageLauncher PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/appimagelauncher")

# add dependency on runtime to make sure it's built
add_dependencies(AppImageLauncher runtime)

install(
    TARGETS
    AppImageLauncher
    RUNTIME DESTINATION bin COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION lib COMPONENT APPIMAGELAUNCHER
)


add_executable(remove remove_main.cpp)
target_link_libraries(remove Qt5::Core Qt5::Widgets libappimage)
# see AppImageLauncher for a description
set_target_properties(remove PROPERTIES INSTALL_RPATH "\$ORIGIN")

install(
    TARGETS remove
    RUNTIME DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)


add_executable(update update_main.cpp)
target_link_libraries(update Qt5::Core Qt5::Widgets libappimage libappimageupdate-qt)
# see AppImageLauncher for a description
set_target_properties(update PROPERTIES INSTALL_RPATH "\$ORIGIN")

install(
    TARGETS update
    RUNTIME DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
