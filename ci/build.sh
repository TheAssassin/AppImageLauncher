#! /bin/bash

set -euo pipefail

# use RAM disk if possible
if [ -d /dev/shm ] && mount | grep /dev/shm | grep -v -q noexec; then
    TEMP_BASE=/dev/shm
elif [ -d /docker-ramdisk ]; then
    TEMP_BASE=/docker-ramdisk
else
    TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" AppImageLauncher-build-XXXXXX)"

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")"/..)"
OLD_CWD="$(readlink -f .)"

pushd "$BUILD_DIR"

# list available versions of Qt to be able to choose the right one for the build
cat <<\EOF
##########################
# Available Qt versions: #
##########################
EOF
qtchooser -list-versions
echo

# the Docker images provide a clang/clang++ symlink to the actual clang[++] binaries
# see install-deps.sh for more information
cmake_args=(
    "-DCMAKE_C_COMPILER=clang"
    "-DCMAKE_CXX_COMPILER=clang++"
    "-DCMAKE_INSTALL_PREFIX=/usr"
    "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
    "-DCPACK_DEBIAN_COMPATIBILITY_LEVEL=$DIST"
    "-DBUILD_TESTING=OFF"
    "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
)

if [[ "${BUILD_LITE:-}" != "" ]]; then
    cmake_args+=("-DBUILD_LITE=ON")
fi

export QT_SELECT=qt5

cmake "$REPO_ROOT" "${cmake_args[@]}"

make -j$(nproc)

# prepare AppDir
make install DESTDIR=AppDir

ARCH="$(dpkg --print-architecture)"
[[ "$ARCH" == "amd64" ]] && ARCH=x86_64

# build release formats
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"$ARCH".AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-native_packages/releases/download/continuous/linuxdeploy-plugin-native_packages-x86_64.AppImage
chmod -v +x linuxdeploy*-"$ARCH".AppImage

VERSION=$(src/cli/ail-cli --version | awk '{print $3}')

gha_build="$GITHUB_RUN_NUMBER"

if [[ "$gha_build" != "" ]]; then
    VERSION="${VERSION}-gha${gha_build}"
else
    VERSION="${VERSION}-local"
fi

VERSION="${VERSION}~$(cd "$REPO_ROOT" && git rev-parse --short HEAD)"

# might seem pointless, but it's necessary to have the version number written inside the AppImage as well
export VERSION

OUTPUT="$(echo appimagelauncher-lite-"$VERSION"-"$ARCH".AppImage | tr '~' -)"
export OUTPUT

export APPIMAGE_EXTRACT_AND_RUN=1

# since we extracted common parts from the installer built into the AppRun script, we have to copy the "library" script
# before building an AppImage
install "$REPO_ROOT"/resources/appimagelauncher-lite-installer-common.sh "$(readlink -f AppDir/)"

./linuxdeploy-"$ARCH".AppImage --plugin qt --appdir "$(readlink -f AppDir)" --custom-apprun "$REPO_ROOT"/resources/appimagelauncher-lite-AppRun.sh --output native_packages \
    -d "$REPO_ROOT"/resources/appimagelauncher-lite.desktop \
    -e "$(find AppDir/usr/lib/{,*/}appimagelauncher/remove | head -n1)" \
    -e "$(find AppDir/usr/lib/{,*/}appimagelauncher/update | head -n1)"

mv "$OUTPUT" "$OLD_CWD"
