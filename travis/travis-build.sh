#! /bin/bash

set -x
set -e

# use RAM disk if possible
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" AppImageLauncher-build-XXXXXX)

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname "${BASH_SOURCE[0]}")/..)
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

# install more recent CMake version which fixes some linking issue in CMake < 3.10
# Fixes https://github.com/TheAssassin/AppImageLauncher/issues/106
# Upstream bug: https://gitlab.kitware.com/cmake/cmake/issues/17389
wget https://cmake.org/files/v3.13/cmake-3.13.2-Linux-x86_64.tar.gz -qO- | tar xz --strip-components=1
export PATH=$(readlink -f bin/):"$PATH"
which cmake
cmake --version

EXTRA_CMAKE_FLAGS=

if [ "$BIONIC" != "" ]; then
    EXTRA_CMAKE_FLAGS="-DCPACK_DEBIAN_COMPATIBILITY_LEVEL=bionic"
elif [ "$COSMIC" != "" ]; then
    EXTRA_CMAKE_FLAGS="-DCPACK_DEBIAN_COMPATIBILITY_LEVEL=cosmic"
else
    EXTRA_CMAKE_FLAGS="-DUSE_CCACHE=ON"
fi

if [ "$ARCH" == "i386" ]; then
    EXTRA_CMAKE_FLAGS="$EXTRA_CMAKE_FLAGS -DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/cmake/toolchains/i386-linux-gnu.cmake -DUSE_SYSTEM_XZ=ON -DUSE_SYSTEM_LIBARCHIVE=ON"
    if [ "$BIONIC" == "" ] && [ "$COSMIC" == "" ]; then
        export QT_SELECT=qt5-i386-linux-gnu
    else
        export QT_SELECT=qt5
    fi
fi

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo $EXTRA_CMAKE_FLAGS -DTRAVIS_BUILD=ON -DBUILD_TESTING=OFF

# now, compile
make

# re-run cmake to find built shared objects with the globs, and update the CPack files
cmake .

# build Debian package
cpack -V -G DEB

# skip RPM and source tarball build on bionic
if [ "$BIONIC" == "" ] && [ "$COSMIC" == "" ]; then
    # build RPM package
    cpack -V -G RPM

    # build source tarball
    # generates a lot of output, therefore not run in verbose mode
    cpack --config CPackSourceConfig.cmake

    # generate log for debugging
    # CPack is very verbose, therefore we generate a file and upload it
    cpack --config CPackSourceConfig.cmake -V
fi

# move AppImages to old cwd
if [ "$BIONIC" == "" ] && [ "$COSMIC" == "" ]; then
    mv appimagelauncher*.{deb,rpm}* appimagelauncher*.tar* "$OLD_CWD"/
else
    mv appimagelauncher*.deb* "$OLD_CWD"/
fi

popd
