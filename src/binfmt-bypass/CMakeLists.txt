cmake_minimum_required(VERSION 3.2)
project(appimage-binfmt-bypass)

# configure names globally
set(bypass_bin binfmt-bypass)
set(preload_lib binfmt-bypass-preload)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# library to be preloaded when launching the patched runtime binary
# we need to build with -fPIC, otherwise we can't use it with $LD_PRELOAD
add_library(${preload_lib} SHARED preload.c logging.h)
target_link_libraries(${preload_lib} PRIVATE dl)
target_compile_options(${preload_lib} PRIVATE
    -fPIC
    -DCOMPONENT_NAME="lib"
    # hide all symbols by default
    -fvisibility=hidden
)
# compatibility with CMake < 3.13
set_target_properties(${preload_lib} PROPERTIES LINK_OPTIONS
    # hide all symbols by default
    -fvisibility=hidden
)

# binary that extracts the runtime, patches it and launches it, preloading the library
add_executable(${bypass_bin} main.cpp elf.cpp logging.h elf.h)
target_link_libraries(${bypass_bin} PRIVATE dl)
target_compile_options(${bypass_bin} PRIVATE -DPRELOAD_LIB_NAME="$<TARGET_FILE_NAME:${preload_lib}>" -DCOMPONENT_NAME="bin" -D_GNU_SOURCE)

include(CheckCSourceCompiles)
message(STATUS "Checking whether memfd_create(...) is available")
check_c_source_compiles("
    #define _GNU_SOURCE
    #include <sys/mman.h>
    int main(int argc, char** argv) {
        memfd_create(\"test\", 0);
    }
    "
    HAVE_MEMFD_CREATE
)

if(HAVE_MEMFD_CREATE)
    target_compile_options(${bypass_bin} PRIVATE -DHAVE_MEMFD_CREATE)
else()
    message(WARNING "memfd_create not available, falling back to shm_open")
    target_link_libraries(${bypass_bin} PRIVATE rt)
endif()

# the binary uses the library, so let's make sure it's up to date before we build the bypass binary
add_dependencies(${bypass_bin} ${preload_lib})

# no need to set rpath on bypass binary, it doesn't depend on any private libraries
# the preload lib is used via its absolute path anyway

install(
    TARGETS ${bypass_bin} ${preload_lib}
    RUNTIME DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
)
