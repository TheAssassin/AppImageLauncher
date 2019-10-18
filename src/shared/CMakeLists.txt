add_library(shared STATIC shared.h shared.cpp types.h)
target_link_libraries(shared PUBLIC PkgConfig::glib Qt5::Core Qt5::Widgets Qt5::DBus libappimage translationmanager trashbin)
if(ENABLE_UPDATE_HELPER)
    target_link_libraries(shared PUBLIC libappimageupdate)
endif()
target_compile_definitions(shared
    PRIVATE -DPRIVATE_LIBDIR="${_private_libdir}"
    PRIVATE -DCMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
