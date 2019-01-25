# use latest FUSE API
add_definitions(-DFUSE_USE_VERSION=26)
# required by FUSE
add_definitions(-D_FILE_OFFSET_BITS=64)

if(NOT TARGET libfuse)
    find_package(FUSE REQUIRED)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(appimagelauncherfs main.cpp fs.cpp fs.h error.h)
target_link_libraries(appimagelauncherfs PUBLIC libfuse Boost::filesystem)

# ISO C++ spawns annoying warnings about string literals
target_compile_options(appimagelauncherfs PRIVATE -Wno-write-strings)

install(
    TARGETS appimagelauncherfs COMPONENT APPIMAGELAUNCHERFS
    RUNTIME DESTINATION ${_bindir}
)
