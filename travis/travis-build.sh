#! /bin/bash

set -x
set -e

# use RAM disk if possible
if [ "$BIONIC" == "" ] && [ -d /dev/shm ]; then
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
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

EXTRA_CMAKE_FLAGS=

if [ "$BIONIC" == "" ]; then
    EXTRA_CMAKE_FLAGS="-DUSE_CCACHE=ON"
else
    EXTRA_CMAKE_FLAGS="-DCPACK_DEBIAN_COMPATIBILITY_LEVEL=bionic"
fi

if [ "$ARCH" == "i386" ]; then
    EXTRA_CMAKE_FLAGS="$EXTRA_CMAKE_FLAGS -DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/cmake/toolchains/i386-linux-gnu.cmake -DUSE_SYSTEM_XZ=ON -DUSE_SYSTEM_LIBARCHIVE=ON"
    if [ "$BIONIC" == "" ]; then
        export QT_SELECT=qt5-i386-linux-gnu
    else
        export QT_SELECT=qt5
    fi
fi

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo $EXTRA_CMAKE_FLAGS -DTRAVIS_BUILD=ON -DBUILD_TESTING=OFF

# now, compile
make

# build Debian package
cpack -V -G DEB

# skip RPM and source tarball build on bionic
if [ "$BIONIC" == "" ]; then
    # build RPM package
    cpack -V -G RPM

    # build source tarball
    # generates a lot of output, therefore not run in verbose mode
    cpack --config CPackSourceConfig.cmake
fi

# move AppImages to old cwd
if [ "$BIONIC" == "" ]; then
    mv appimagelauncher*.{deb,rpm}* appimagelauncher*.tar* "$OLD_CWD"/
else
    mv appimagelauncher*.deb* "$OLD_CWD"/
fi

popd
