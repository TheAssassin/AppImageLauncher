add_library(filesystemwatcher STATIC filesystemwatcher.cpp filesystemwatcher.h)
target_link_libraries(filesystemwatcher PUBLIC Qt5::Core shared)
target_include_directories(filesystemwatcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
