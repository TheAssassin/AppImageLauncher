#! /bin/bash

if [ "$ARCH" == "" ]; then
    echo "Error: you must set \$ARCH"
    exit 2
fi

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

if [ "$ARCH" == "i386" ]; then
    EXTRA_CMAKE_FLAGS="$EXTRA_CMAKE_FLAGS -DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/cmake/toolchains/i386-linux-gnu.cmake -DUSE_SYSTEM_XZ=ON -DUSE_SYSTEM_LIBARCHIVE=ON"
    # TODO check if this can be removed
    if [ "$DEBIAN_DIST" != "bionic" ] && [ "$DEBIAN_DIST" != "cosmic" ]; then
        export QT_SELECT=qt5-i386-linux-gnu
    else
        export QT_SELECT=qt5
    fi
fi

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo $EXTRA_CMAKE_FLAGS -DTRAVIS_BUILD=ON -DBUILD_TESTING=OFF -DBUILD_LITE=ON

# compile dependencies
make -j $(nproc) libappimage libappimageupdate libappimageupdate-qt

# re-run cmake to update paths to dependencies
cmake .

# build rest
make -j $(nproc)

# prepare AppDir
make install DESTDIR=AppDir

# build AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"$ARCH".AppImage
chmod -v +x linuxdeploy*-"$ARCH".AppImage

VERSION=$(src/cli/ail-cli --version | awk '{print $3}')

travis_build="$TRAVIS_BUILD_NUMBER"

if [[ "$travis_build" != "" ]]; then
    VERSION="${VERSION}-travis${travis_build}"
else
    VERSION="${VERSION}-local"
fi

VERSION="${VERSION}~$(cd "$REPO_ROOT" && git rev-parse --short HEAD)"

# might seem pointless, but it's necessary to have the version number written inside the AppImage as well, so don't remove
export VERSION

export OUTPUT=$(echo appimagelauncher-lite-"$VERSION"-"$ARCH".AppImage | tr '~' -)

export APPIMAGE_EXTRACT_AND_RUN=1

# since we extracted common parts from the installer built into the AppRun script, we have to copy the "library" script
# before building an AppImage
install "$REPO_ROOT"/resources/appimagelauncher-lite-installer-common.sh $(readlink -f AppDir/)

./linuxdeploy-"$ARCH".AppImage --plugin qt --appdir $(readlink -f AppDir) --custom-apprun "$REPO_ROOT"/resources/appimagelauncher-lite-AppRun.sh --output appimage \
    -d "$REPO_ROOT"/resources/appimagelauncher-lite.desktop \
    -e $(find AppDir/usr/lib/{,*/}appimagelauncher/remove | head -n1) \
    -e $(find AppDir/usr/lib/{,*/}appimagelauncher/update | head -n1)

mv "$OUTPUT" "$OLD_CWD"
