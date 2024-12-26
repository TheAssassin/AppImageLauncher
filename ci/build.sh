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
    "-DBUILD_TESTING=OFF"
    "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
)

if [[ "${BUILD_LITE:-}" == "" ]]; then
    cmake_args+=("-DENABLE_UPDATE_HELPER=ON")
else
    cmake_args+=("-DBUILD_LITE=ON")
fi

export QT_SELECT=qt5

cmake "$REPO_ROOT" "${cmake_args[@]}"

make -j$(nproc)

# prepare AppDir
make install DESTDIR=AppDir

ARCH="$(dpkg --print-architecture)"

# "translate" to linuxdeploy/AppImage architecture
# note: linuxdeploy and AppImage differ in i386/i686, but we don't support that any more anyway
case "$ARCH" in
    amd64)
        ARCH=x86_64
        ;;
    arm64)
        ARCH=aarch64
        ;;
esac

# build release formats
curl -LO https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-"$ARCH".AppImage
curl -LO https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"$ARCH".AppImage

VERSION=$(src/cli/ail-cli --version | awk '{print $3}')

gha_build="${GITHUB_RUN_NUMBER:-}"

if [[ "$gha_build" != "" ]]; then
    VERSION="${VERSION}-gha${gha_build}"
else
    VERSION="${VERSION}-local"
fi

# should be overwritten for every output plugin below, this is a fallback only
export LINUXDEPLOY_OUTPUT_APP_NAME=appimagelauncher

LINUXDEPLOY_OUTPUT_VERSION="${VERSION}~$(cd "$REPO_ROOT" && git rev-parse --short HEAD)"
export LINUXDEPLOY_OUTPUT_VERSION

export APPIMAGE_EXTRACT_AND_RUN=1

linuxdeploy_extra_args=()

if [[ "${BUILD_LITE:-}" == "" ]]; then
    # configure linuxdeploy-plugin-native_packages
    export LDNP_PACKAGE_NAME=appimagelauncher

    export LDNP_BUILD="deb rpm"
    export LDNP_DESCRIPTION=""
    export LDNP_SHORT_DESCRIPTION=""

    case "$ARCH" in
        x86_64)
            rpm_build_arch=x86_64
            deb_build_arch=amd64
            ;;
        i?86)
            rpm_build_arch=i386
            deb_build_arch=i386
            ;;
        armhf|aarch64)
            deb_build_arch="$ARCH"
            rpm_build_arch="$ARCH"
            ;;
        *)
            echo "Unsupported architecture: $ARCH"
            exit 2
            ;;
    esac

    # common meta info
    export LDNP_META_URL="https://github.com/TheAssassin/AppImageLauncher"
    export LDNP_META_BUG_URL="https://github.com/TheAssassin/AppImageLauncher/issues"
    export LDNP_META_VENDOR="TheAssassin"

    export LDNP_META_DEB_DEPENDS="systemd, libgl1, libfontconfig1, libharfbuzz0b, libfribidi0"
    export LDNP_META_DEB_BUILD_ARCH="$deb_build_arch"
    export LDNP_META_DEB_PRE_DEPENDS="bash"
    export LDNP_DEB_EXTRA_DEBIAN_FILES="${BUILD_DIR}/cmake/debian/postinst;${BUILD_DIR}/cmake/debian/postrm"

    export LDNP_META_RPM_REQUIRES="systemd libGL.so.1 libfontconfig.so.1 libfreetype.so.6 libfribidi.so.0 libgpg-error.so.0 libharfbuzz.so.0"
    export LDNP_META_RPM_BUILD_ARCH="$rpm_build_arch"
    export LDNP_RPM_SCRIPTLET_POST="${BUILD_DIR}/cmake/debian/postinst"
    export LDNP_RPM_SCRIPTLET_PREUN="${BUILD_DIR}/cmake/debian/postrm"

    # updater is not available for the lite build
    linuxdeploy_extra_args+=(
        -e "$(find AppDir/usr/lib/*/appimagelauncher/update | head -n1)"
        --output native_packages
    )

    # tools like pipx would have side effects on the build host and it's generally a bit overkill for our purpose
    if which python3.13 &> /dev/null; then
        # python3.13, installed from the deadsnakes PPA for the Docker builds
        python3() {
            python3.13 "$@"
        }
    fi
    python3 -m venv venv
    venv/bin/pip install git+https://github.com/linuxdeploy/linuxdeploy-plugin-native_packages
    export PATH="$PWD/venv/bin:$PATH"
else
    linuxdeploy_extra_args+=(
        --custom-apprun "$REPO_ROOT"/resources/appimagelauncher-lite-AppRun.sh
        --output appimage
    )

    LDAI_OUTPUT="$(echo appimagelauncher-lite-"$VERSION"-"$ARCH".AppImage | tr '~' -)"
    export LDAI_OUTPUT

    # since we extracted common parts from the installer built into the AppRun script, we have to copy the "library" script
    # before building an AppImage
    install "$REPO_ROOT"/resources/appimagelauncher-lite-installer-common.sh "$(readlink -f AppDir/)"
fi

chmod -v +x linuxdeploy*-"$ARCH".AppImage

# workaround for QEMU
for appimage in *.AppImage; do
    dd if=/dev/zero bs=1 count=3 seek=8 conv=notrunc of="$appimage"
done

./linuxdeploy-"$ARCH".AppImage -v0 \
    --appdir "$(readlink -f AppDir)" \
    --plugin qt \
    -d AppDir/usr/share/applications/appimagelauncher.desktop \
    -e "$(find AppDir/usr/lib/*/appimagelauncher/remove | head -n1)" \
    "${linuxdeploy_extra_args[@]}"

if [[ "${BUILD_LITE:-}" == "" ]]; then
    mv "$LDNP_PACKAGE_NAME"*.{rpm,deb} "$OLD_CWD"
else
    mv "$LDAI_OUTPUT" "$OLD_CWD"
fi
