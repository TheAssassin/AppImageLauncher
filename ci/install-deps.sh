#! /bin/bash

set -e

if [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... bash $0"
    exit 2
fi

if [[ "$CI" == "" ]]; then
    echo "Caution: this script is supposed to run inside a (disposable) CI environment"
    echo "It will alter a system, and should not be run on workstations or alike"
    echo "You can export CI=1 to prevent this error from being shown again"
    exit 3
fi

case "$ARCH" in
    x86_64|i386|armhf|arm64)
        ;;
    *)
        echo "Error: unsupported architecture: $ARCH"
        exit 4
        ;;
esac

case "$DIST" in
    xenial|bionic)
        ;;
    *)
        echo "Error: unsupported distribution: $DIST"
        exit 5
        ;;
esac

set -x

packages=(
    libcurl4-openssl-dev
    libfuse-dev
    desktop-file-utils
    libglib2.0-dev
    libcairo2-dev
    libssl-dev
    ca-certificates
    libbsd-dev
    qttools5-dev-tools
    gcc
    g++
    make
    build-essential
    git
    automake
    autoconf
    libtool
    patch
    wget
    vim-common
    desktop-file-utils
    pkg-config
    libarchive-dev
    libboost-filesystem-dev
    librsvg2-dev
    librsvg2-bin
    libssl1.0.0
    rpm
    rpm2cpio
    liblzma-dev

    # cross-compiling for 32-bit is only really easy with clang, where we can specify the target as a compiler option
    # clang -target arm-linux-gnueabihf ...
    # we must use clang > 3.8.0, and newer versions should work as drop-in replacement, so we can just use the newest
    # clang available on xenial on all platforms
    clang-8
)

if [[ "$BUILD_LITE" == "" ]]; then
    packages+=(
        qtbase5-dev
        qt5-qmake
    )
else
    apt-get update
    apt-get -y --no-install-recommends install software-properties-common
    add-apt-repository -y ppa:beineri/opt-qt-5.14.2-"$DIST"
    packages+=(
        qt514-meta-minimal
    )
fi

# install 32-bit build dependencies and multilib/cross compilers for binfmt-bypass's 32-bit preload library
if [[ "$ARCH" == "x86_64" ]]; then
    dpkg --add-architecture i386
    packages+=(
        libc6-dev:i386
    )
elif [[ "$ARCH" == "arm64"* ]]; then
    dpkg --add-architecture armhf
    packages+=(
        libc6-dev:armhf
    )
fi

apt-get update
apt-get -y --no-install-recommends install "${packages[@]}"

  # g{cc,++}-multilib usually install these dependencies for us
  # however, as the multilib stuff is not available for ARM, we have to install these dev packages ourselves
  # we can't really predict the names of the packages (they differ on different distros/releases)
  # therefore, we have to install the other dependencies first to be able to query them with dpkg -l
if [[ "$ARCH" == "arm64"* ]] || [[ "$ARCH" == "x86_64" ]]; then
    if [[ "$ARCH" == "x86_64" ]]; then
        ARCH_32BIT=i386
    elif [[ "$ARCH" == "arm64"* ]]; then
        ARCH_32BIT=armhf
    else
        echo "Cannot determine 32-bit architecture matching 64-bit architecture $ARCH"
        exit 6
    fi

    apt-get install -y \
        "$(dpkg -l | grep libgcc | grep dev | awk '{print $2}' | cut -d: -f1 | uniq)":"$ARCH_32BIT" \
        "$(dpkg -l | grep libstdc++ | grep dev | awk '{print $2}' | cut -d: -f1 | uniq)":"$ARCH_32BIT"
fi

# install more recent CMake version which fixes some linking issue in CMake < 3.10
# Fixes https://github.com/TheAssassin/AppImageLauncher/issues/106
# Upstream bug: https://gitlab.kitware.com/cmake/cmake/issues/17389
CMAKE_ARCH="$ARCH"
if [[ "$CMAKE_ARCH" == "arm64"* ]]; then
    CMAKE_ARCH=arm64v8
fi
wget https://artifacts.assassinate-you.net/prebuilt-cmake/continuous/cmake-v3.19.1-ubuntu_xenial-"$CMAKE_ARCH".tar.gz -qO- | \
    tar xz -C/usr/local --strip-components=1

if [[ "$BUILD_LITE" != "" ]]; then
    # https://github.com/TheAssassin/AppImageLauncher/issues/199
    apt-get update
    apt-get -y install libgtk2.0-dev libgl1-mesa-dev
    source /opt/qt*/bin/qt*env.sh || true
    git clone http://code.qt.io/qt/qtstyleplugins.git
    cd qtstyleplugins
    qmake
    make -j"$(nproc)"
    make install
fi

# provide clang/clang++ symlink to actual clang binaries
# without this, boost (dep from libappimage) doesn't build properly, as it can't find the compiler binaries
# since we have to install this symlink anyway, we're also using it when passing CMAKE_C{,XX}_COMPILER in the build script
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-8 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-8 100
