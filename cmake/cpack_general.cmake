# general CPack options

set(PROJECT_VERSION 1.0)
set(CPACK_GENERATOR "DEB")

# make sure to only package APPIMAGELAUNCHER component
set(CPACK_COMPONENTS_ALL APPIMAGELAUNCHER)

# global options
set(CPACK_PACKAGE_CONTACT "TheAssassin")
set(CPACK_PACKAGE_HOMEPAGE "https://github.com/TheAssassin/AppImageLauncher")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

# versioning
# it appears setting CPACK_DEBIAN_PACKAGE_VERSION doesn't work, hence setting CPACK_PACKAGE_VERSION
set(CPACK_PACKAGE_VERSION ${APPIMAGELAUNCHER_VERSION})

# TODO: insert some useful description
set(CPACK_COMPONENT_APPIMAGELAUNCHER_PACKAGE_DESCRIPTION "AppImageLauncher")

# find more suitable section for package
set(CPACK_COMPONENT_APPIMAGELAUNCHER_PACKAGE_SECTION misc)

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

# source tarball configuration
set(CPACK_SOURCE_PACKAGE_FILE_NAME appimagelauncher-git${APPIMAGELAUNCHER_GIT_COMMIT})
set(CPACK_SOURCE_GENERATOR TXZ)
