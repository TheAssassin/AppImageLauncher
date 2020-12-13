# needed for LINK_OPTIONS/target_link_options
cmake_minimum_required(VERSION 3.13)

project(appimage-binfmt-bypass)

# configure names globally
set(bypass_bin binfmt-bypass)
set(preload_lib binfmt-bypass-preload)
# we're not using "i386" or "armhf" but instead the more generic "32bit" suffix, making things a little easier in the
# code
set(preload_lib_32bit ${preload_lib}_32bit)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

string(TOLOWER ${CMAKE_C_COMPILER_ID} lower_cmake_c_compiler_id)

# on 64-bit systems such as x86_64 or aarch64/arm64v8, we need to provide a 32-bit preload library to allow the users
# to execute compatible 32-bit binaries, as the regular preload library cannot be preloaded due to the architecture
# being incompatible
# unfortunately, compiling on ARM is significantly harder than just adding a compiler flag, as gcc-multilib is missing
# it's likely easier with clang, but that remains to be verified
if(CMAKE_SYSTEM_PROCESSOR MATCHES x86_64)
    message(STATUS "64-bit target detected (${CMAKE_SYSTEM_PROCESSOR}), building 32-bit preload library as well")
    set(build_32bit_preload_library true)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES arm64 OR CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
    string(TOLOWER ${CMAKE_C_COMPILER_ID} lower_cmake_c_compiler_id)

    if(lower_cmake_c_compiler_id MATCHES clang)
        # clang-3.8, the default clang on xenial, doesn't like the templates in elf.cpp with the varying return types
        if(NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 3.9.0)
            message(FATAL_ERROR "clang too old, please upgrade to, e.g., clang-8")
        endif()

        message(STATUS "64-bit target detected (${CMAKE_SYSTEM_PROCESSOR}), building 32-bit preload library as well")
        set(build_32bit_preload_library true)
    else()
        message(WARNING "64-bit target detected (${CMAKE_SYSTEM_PROCESSOR}), but compiling with ${lower_cmake_c_compiler_id}, cannot build 32-bit preload library")
    endif()
endif()

function(make_preload_lib_target target_name)
    # library to be preloaded when launching the patched runtime binary
    # we need to build with -fPIC, otherwise we can't use it with $LD_PRELOAD
    add_library(${target_name} SHARED preload.c logging.h)
    target_link_libraries(${target_name} PRIVATE dl)
    target_compile_options(${target_name} PRIVATE
        -fPIC
        -DCOMPONENT_NAME="lib"
        # hide all symbols by default
        -fvisibility=hidden
    )
    # compatibility with CMake < 3.13
    set_target_properties(${target_name} PROPERTIES LINK_OPTIONS
        # hide all symbols by default
        -fvisibility=hidden
    )
endfunction()

make_preload_lib_target(${preload_lib})

if(build_32bit_preload_library)
    make_preload_lib_target(${preload_lib_32bit})

    if(CMAKE_SYSTEM_PROCESSOR MATCHES x86_64)
        if(lower_cmake_c_compiler_id MATCHES clang)
            target_compile_options(${preload_lib_32bit} PRIVATE "--target=i386-linux-gnu")
            target_link_options(${preload_lib_32bit} PRIVATE "--target=i386-linux-gnu")
        else()
            # GCC style flags
            target_compile_options(${preload_lib_32bit} PRIVATE "-m32")
            target_link_options(${preload_lib_32bit} PRIVATE "-m32")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES arm64 OR CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
        target_compile_options(${preload_lib_32bit} PRIVATE "--target=arm-linux-gnueabihf")
        target_link_options(${preload_lib_32bit} PRIVATE "--target=arm-linux-gnueabihf")
    else()
        message(FATAL_ERROR "unknown processor architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()


# binary that extracts the runtime, patches it and launches it, preloading the library
add_executable(${bypass_bin} main.cpp elf.cpp logging.h elf.h)
target_link_libraries(${bypass_bin} PRIVATE dl)
target_compile_options(${bypass_bin} PRIVATE
    -DPRELOAD_LIB_NAME="$<TARGET_FILE_NAME:${preload_lib}>"
    -DCOMPONENT_NAME="bin"
    -D_GNU_SOURCE
)
if(build_32bit_preload_library)
    target_compile_options(${bypass_bin} PRIVATE
        -DPRELOAD_LIB_NAME_32BIT="$<TARGET_FILE_NAME:${preload_lib_32bit}>"
    )
endif()

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
if(build_32bit_preload_library)
    add_dependencies(${bypass_bin} ${preload_lib_32bit})
endif()

# no need to set rpath on bypass binary, it doesn't depend on any private libraries
# the preload lib is used via its absolute path anyway

install(
    TARGETS ${bypass_bin} ${preload_lib}
    RUNTIME DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
    LIBRARY DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
)
if(build_32bit_preload_library)
    install(
        TARGETS ${preload_lib}
        LIBRARY DESTINATION ${_private_libdir} COMPONENT APPIMAGELAUNCHER
    )
endif()
