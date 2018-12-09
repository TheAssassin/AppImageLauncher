add_library(trashbin STATIC trashbin.cpp trashbin.h)
target_link_libraries(trashbin PUBLIC Qt5::Core libappimage shared)
target_compile_definitions(trashbin
    PRIVATE -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
)
target_include_directories(translationmanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
