#! /bin/bash

if [[ "$DIST" == "" ]] || [[ "$ARCH" == "" ]]; then
    echo "Usage: env DIST=... ARCH=... bash $0"
    exit 2
fi

# this script is supposed to be run on Debian(-like) distros currently
# therefore, we abort if we can't detect such an environment
# for instance, we automatically set the debian compat level for CMake to $DIST, and also want to build (working) .deb archives
dist_id="$(cat /etc/*release | grep -E '^ID=' | cut -d= -f2)"
dist_like_id="$(cat /etc/*release | grep -E '^ID_LIKE=' | cut -d= -f2)"

if [[ "$dist_id" != "debian" ]] && [[ "$dist_like_id" != "debian" ]]; then
    echo "Error: this script can only be used on Debian(-like) distributions"
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
    "-DCI_BUILD=ON"
    "-DBUILD_TESTING=OFF"
)

export QT_SELECT=qt5

if [ "$ARCH" == "i386" ]; then
    cmake_args+=(
        "-DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/cmake/toolchains/i386-linux-gnu.cmake"
        "-DUSE_SYSTEM_XZ=ON"
        "-DUSE_SYSTEM_LIBARCHIVE=ON"
    )

    if [ "$DIST" == "xenial" ]; then
        export QT_SELECT=qt5-i386-linux-gnu
    fi
fi

if [ "$ARCH" == "armhf" ]; then
    export QT_SELECT=qt5-arm-linux-gnueabihf
fi

if [ "$ARCH" == "arm64" ]; then
    # only clang allows for easy cross-compilation of a 32-bit version of the binfmt-bypass preload lib
    cmake_args+=(
        "-DCMAKE_C_COMPILER=clang-8"
        "-DCMAKE_CXX_COMPILER=clang++-8"
    )
fi

cmake "$REPO_ROOT" "${cmake_args[@]}" 

# now, compile
if [[ "$CI" == "" ]]; then
    nproc="$(nproc --ignore=1)"
else
    nproc="$(nproc)"
fi

nproc=1
make -j "$nproc"

# re-run cmake to find built shared objects with the globs, and update the CPack files
cmake .

# build Debian package
cpack -V -G DEB

# skip RPM and source tarball build on bionic
if [ "$DIST" == "xenial" ]; then
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
if [ "$DIST" == "xenial" ]; then
    mv appimagelauncher*.{deb,rpm}* appimagelauncher*.tar* "$OLD_CWD"/
else
    mv appimagelauncher*.deb* "$OLD_CWD"/
fi

popd
