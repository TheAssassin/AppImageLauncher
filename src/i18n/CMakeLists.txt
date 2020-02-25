add_library(translationmanager translationmanager.cpp translationmanager.h)
target_link_libraries(translationmanager PUBLIC Qt5::Core Qt5::Widgets shared)
target_include_directories(translationmanager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(translationmanager l10n)
