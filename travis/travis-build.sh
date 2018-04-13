#! /bin/bash

set -x
set -e

# use RAM disk if possible
if [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" AppImageUpdate-build-XXXXXX)

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

cmake "$REPO_ROOT" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DUSE_CCACHE=ON

# now, compile
make -j$(nproc)

# build Debian package
cpack -V -G DEB

# build source tarball
cpack --config CPackSourceConfig.cmake -V

# build AppImage
# create AppDir
mkdir -p AppDir

# install to AppDir
make install DESTDIR=AppDir

# determine Git commit ID
# linuxdeployqt uses this for naming the file
export VERSION="git"$(cd "$REPO_ROOT" && git rev-parse --short HEAD)

# prepend Travis build number if possible
if [ "$TRAVIS_BUILD_NUMBER" != "" ]; then
    export VERSION="travis$TRAVIS_BUILD_NUMBER-$VERSION"
fi

# remove other unnecessary data
find AppDir -type f -iname '*.a' -delete
rm -rf AppDir/usr/include


# get linuxdeployqt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage


LINUXDEPLOYQT_ARGS=

if [ "$CI" == "" ]; then
    LINUXDEPLOYQT_ARGS="-no-copy-copyright-files"
fi


find AppDir/

unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH

# bundle application
./linuxdeployqt-continuous-x86_64.AppImage \
    AppDir/usr/share/applications/appimagelauncher.desktop \
    $LINUXDEPLOYQT_ARGS \
    -verbose=1 -bundle-non-qt-libs

# bundle application
./linuxdeployqt-continuous-x86_64.AppImage \
    AppDir/usr/share/applications/appimagelauncher.desktop \
    $LINUXDEPLOYQT_ARGS \
    -verbose=1 -bundle-non-qt-libs -appimage

# move AppImages to old cwd
mv AppImageLauncher*.AppImage* appimagelauncher*.deb* appimagelauncher*.tar* "$OLD_CWD"/

popd
