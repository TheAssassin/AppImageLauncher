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

# skip RPM and AppImage build on bionic
if [ "$BIONIC" == "" ]; then
    # build RPM package
    cpack -V -G RPM

    # build source tarball
    # generates a lot of output, therefore not run in verbose mode
    cpack --config CPackSourceConfig.cmake

    # build AppImage
    # create AppDir
    mkdir -p AppDir

    # install to AppDir
    make install DESTDIR=AppDir

    # determine Git commit ID
    # linuxdeployqt uses this for naming the file
    export VERSION="git"$(cd "$REPO_ROOT" && date +%Y%m%d -u -d "$(git show -s --format=%ci $(git rev-parse HEAD))").$(cd "$REPO_ROOT" && git rev-parse --short HEAD)

    # prepend Travis build number if possible
    if [ "$TRAVIS_BUILD_NUMBER" != "" ]; then
        export VERSION="travis$TRAVIS_BUILD_NUMBER-$VERSION"
    fi

    # remove other unnecessary data
    find AppDir -type f -iname '*.a' -delete
    rm -rf AppDir/usr/include


    # get linuxdeployqt
    wget https://github.com/TheAssassin/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    wget https://github.com/TheAssassin/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy*-x86_64.AppImage

    find AppDir/

    unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH

    # fix for trusty: default qmake is for qt4
    if [ -x /usr/lib/x86_64-linux-gnu/qt5/bin/qmake ]; then
        export QMAKE="/usr/lib/x86_64-linux-gnu/qt5/bin/qmake"
    fi

    # bundle application
    export UPDATE_INFORMATION="gh-releases-zsync|TheAssassin|AppImageLauncher|AppImageLauncher*-$ARCH.AppImage.zsync"

    ./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage
fi

# move AppImages to old cwd
if [ "$BIONIC" == "" ]; then
    mv AppImageLauncher*.AppImage* appimagelauncher*.{deb,rpm}* appimagelauncher*.tar* "$OLD_CWD"/
else
    mv appimagelauncher*.deb* "$OLD_CWD"/

fi

popd
