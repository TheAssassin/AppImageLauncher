cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_executable(AppImageLauncher main.cpp)
target_link_libraries(AppImageLauncher Qt5::Core Qt5::Widgets libappimage)
target_compile_definitions(AppImageLauncher
    PRIVATE -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
)

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

install(
    TARGETS remove
    RUNTIME DESTINATION lib/appimagelauncher COMPONENT APPIMAGELAUNCHER
)
