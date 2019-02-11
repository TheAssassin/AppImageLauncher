# this little snippet makes sure that no absolute paths end up in the binaries built by CMake
# it will replace such paths with relative ones
# see https://reproducible-builds.org/docs/build-path/ for more information

cmake_minimum_required(VERSION 3.4)

include(CheckCCompilerFlag)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "Release build detected, enabling reproducible builds mode")
    get_filename_component(abs_source_path ${PROJECT_SOURCE_DIR} ABSOLUTE)
    file(RELATIVE_PATH rel_source_path ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

    set(map_fix ${abs_source_path}=${rel_source_path})

    # can only add flags when the compiler supports them
    # known working compilers: GCC >= 8
    foreach(type debug macro)
        set(flag_name -f${type}-prefix-map)
        set(flags ${flag_name}=${map_fix})

        check_c_compiler_flag(${flags} ${type}_prefix_map_flag_available)

        if(${type}_prefix_map_flag_available)
            set(extra_flags "${extra_flags} ${flags}")
        else()
            message(WARNING "${flag_name} not available, cannot enable full reproducible builds mode")
        endif()
    endforeach()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${extra_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${extra_flags}")
endif()
