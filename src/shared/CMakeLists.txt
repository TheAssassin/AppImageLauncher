add_library(shared STATIC shared.h shared.cpp)
target_link_libraries(shared PUBLIC PkgConfig::glib Qt5::Core Qt5::Widgets Qt5::DBus libappimage translationmanager trashbin libappimageupdate)
target_compile_definitions(shared
    PRIVATE -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
    PRIVATE -DCMAKE_INSTALL_LIBDIR="${CMAKE_INSTALL_LIBDIR}"
    PRIVATE -DCMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
