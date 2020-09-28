# general CPack options

set(PROJECT_VERSION ${APPIMAGELAUNCHER_VERSION})
set(CPACK_GENERATOR "DEB")

# make sure to only package APPIMAGELAUNCHER component
set(CPACK_COMPONENTS_ALL APPIMAGELAUNCHER APPIMAGELAUNCHER_CLI)
# package them all in a single package, otherwise cpack would generate one package per component by default
# https://cmake.org/cmake/help/v3.0/module/CPackComponent.html#variable:CPACK_COMPONENTS_GROUPING
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

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
