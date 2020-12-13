#! /bin/bash

if [ "$ARCH" == "" ]; then
    echo "Error: you must set \$ARCH"
    exit 2
fi

set -x
set -e

# use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
    TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
    TEMP_BASE=/docker-ramdisk
else
    TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" AppImageLauncher-lite-build-XXXXXX)"

cleanup() {
    [[ -d "$BUILD_DIR" ]] && rm -rf "$BUILD_DIR"
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")"/..)"
OLD_CWD="$(readlink -f .)"

pushd "$BUILD_DIR"

EXTRA_CMAKE_FLAGS=()
export QT_SELECT=qt5

if [ "$ARCH" == "i386" ]; then
    EXTRA_CMAKE_FLAGS+=(
        "-DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/cmake/toolchains/i386-linux-gnu.cmake"
        "-DUSE_SYSTEM_XZ=ON"
        "-DUSE_SYSTEM_LIBARCHIVE=ON"
    )

    # TODO check if this can be removed
    if [ "$DIST" == "xenial" ]; then
        export QT_SELECT=qt5-i386-linux-gnu
    fi
fi

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo "${EXTRA_CMAKE_FLAGS[@]}" -DCI_BUILD=ON -DBUILD_TESTING=OFF -DBUILD_LITE=ON

# compile dependencies
make -j "$(nproc)" libappimage libappimageupdate libappimageupdate-qt

# re-run cmake to update paths to dependencies
cmake .

# build rest
make -j "$(nproc)"

# prepare AppDir
make install DESTDIR=AppDir

# build AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"$ARCH".AppImage
chmod -v +x linuxdeploy*-"$ARCH".AppImage

VERSION=$(src/cli/ail-cli --version | awk '{print $3}')

travis_build="$GITHUB_RUN_NUMBER"

if [[ "$travis_build" != "" ]]; then
    VERSION="${VERSION}-travis${travis_build}"
else
    VERSION="${VERSION}-local"
fi

VERSION="${VERSION}~$(cd "$REPO_ROOT" && git rev-parse --short HEAD)"

# might seem pointless, but it's necessary to have the version number written inside the AppImage as well, so don't remove
export VERSION

OUTPUT="$(echo appimagelauncher-lite-"$VERSION"-"$ARCH".AppImage | tr '~' -)"
export OUTPUT

export APPIMAGE_EXTRACT_AND_RUN=1

# since we extracted common parts from the installer built into the AppRun script, we have to copy the "library" script
# before building an AppImage
install "$REPO_ROOT"/resources/appimagelauncher-lite-installer-common.sh "$(readlink -f AppDir/)"

./linuxdeploy-"$ARCH".AppImage --plugin qt --appdir "$(readlink -f AppDir)" --custom-apprun "$REPO_ROOT"/resources/appimagelauncher-lite-AppRun.sh --output appimage \
    -d "$REPO_ROOT"/resources/appimagelauncher-lite.desktop \
    -e "$(find AppDir/usr/lib/{,*/}appimagelauncher/remove | head -n1)" \
    -e "$(find AppDir/usr/lib/{,*/}appimagelauncher/update | head -n1)"

mv "$OUTPUT" "$OLD_CWD"
