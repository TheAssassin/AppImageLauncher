find_path(FUSE_INCLUDE_DIR fuse.h
    /usr/include
    /usr/include/x86_64-linux-gnu
    /usr/include/i386-linux-gnu
)

find_library(FUSE_LIBRARY fuse)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("FUSE" DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARY)

mark_as_advanced(FUSE_INCLUDE_DIR FUSE_LIBRARY)

message(STATUS "Found FUSE: ${FUSE_LIBRARY} (include dirs: ${FUSE_INCLUDE_DIR})")

add_library(libfuse IMPORTED SHARED)
set_property(TARGET libfuse PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${FUSE_INCLUDE_DIR}")
set_property(TARGET libfuse PROPERTY IMPORTED_LOCATION "${FUSE_LIBRARY}")
