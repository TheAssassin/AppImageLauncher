add_library(filesystemwatcher OBJECT filesystemwatcher.cpp filesystemwatcher.h)
target_link_libraries(filesystemwatcher PUBLIC Qt5::Core)
target_include_directories(filesystemwatcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
