find_path(INOTIFY_INCLUDE_DIR sys/inotify.h PATH_SUFFIXES inotify)
find_library(INOTIFY_LIBRARY inotify)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(INotify DEFAULT_MSG INOTIFY_INCLUDE_DIR)

IF(INOTIFY_FOUND)
  set(INotify_INCLUDE_DIRS ${INOTIFY_INCLUDE_DIR})

  add_library(inotify IMPORTED INTERFACE)
  set_property(TARGET inotify PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INOTIFY_INCLUDE_DIRS})
ENDIF(INOTIFY_FOUND)
