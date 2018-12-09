add_library(translationmanager translationmanager.cpp translationmanager.h)
target_link_libraries(translationmanager PUBLIC Qt5::Core Qt5::Widgets)
target_compile_definitions(translationmanager
    PRIVATE -DCMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
target_include_directories(translationmanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
