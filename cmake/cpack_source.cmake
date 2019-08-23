# source package options

# ignore files
# requires four backslashes to escape dots etc., probably because the values are evaluated twice,
# once by CMake and then by CPack
set(CPACK_SOURCE_IGNORE_FILES
    # warning: the following pattern requires at least 6 characters before the "build" part to work as intended
    "([^t][^r][^a][^v][^i][^s])/.*build.*/"
    "/.*-prefix/"
    "\\\\..*AppImage$"
    "\\\\..*AppImage\\\\..*$"
    "\\\\..*zs-old.*$"
    "\\\\..*tar.*$"
    "\\\\..*deb.*$"
    "/.git/"
    "/.idea/"
)

# source tarball filename (without extension)
set(CPACK_SOURCE_PACKAGE_FILE_NAME appimagelauncher-${APPIMAGELAUNCHER_GIT_COMMIT}.source)

# build .tar.xz archives to get the maximum amount of compression and produce small tarballs
set(CPACK_SOURCE_GENERATOR TXZ)
