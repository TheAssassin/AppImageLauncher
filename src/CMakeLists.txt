cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_executable(AppImageLauncher main.cpp)
target_link_libraries(AppImageLauncher Qt5::Core Qt5::Widgets libappimage)

install(
    TARGETS
    AppImageLauncher
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    COMPONENT APPIMAGELAUNCHER
)
