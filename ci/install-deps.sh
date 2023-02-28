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
    focal)
        ;;
    *)
        echo "Error: unsupported distribution: $DIST"
        exit 5
        ;;
esac

set -x

echo 'tzdata tzdata/Areas select Europe' | debconf-set-selections
echo 'tzdata tzdata/Zones/Europe select Paris' | debconf-set-selections
export DEBIAN_FRONTEND=noninteractive

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
    rpm
    rpm2cpio
    liblzma-dev
    argagg-dev
    libgcrypt20-dev
    libgpgme-dev

    # cross-compiling for 32-bit is only really easy with clang, where we can specify the target as a compiler option
    # clang -target arm-linux-gnueabihf ...
    clang

    qtbase5-dev
    qt5-qmake
    qtdeclarative5-dev
    qtchooser
    qtbase5-dev-tools
)

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
cmake_arch="$ARCH"
if [[ "$cmake_arch" == "arm64"* ]]; then
    cmake_arch=aarch64
fi
wget https://artifacts.assassinate-you.net/prebuilt-cmake/continuous/cmake-v3.25.2-ubuntu_"$DIST"-"${cmake_arch:-"${ARCH}"}".tar.gz -qO- | \
    tar xz -C/usr/local --strip-components=1

pushd /tmp
    git clone https://github.com/nlohmann/json.git -b v3.11.2 --depth=1
    pushd json
        mkdir build
        cd build
        cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
        make -j "$(nproc --ignore=1)" install
    popd
    rm -rf json/

    git clone https://github.com/NixOS/patchelf -b 0.17.2 --depth=1
    pushd patchelf
        autoreconf -fi
        mkdir build
        cd build
        ../configure --prefix=/usr/local
        make -j "$(nproc --ignore=1)" install
    popd
    rm -rf patchelf/
popd
