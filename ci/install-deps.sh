#! /bin/bash

set -euo pipefail

if [[ "$DOCKER_PLATFORM" == "" ]]; then
    echo "Usage: env DOCKER_PLATFORM=... bash $0"
    exit 2
fi

if [[ "$CI" == "" ]]; then
    echo "Caution: this script is supposed to run inside a (disposable) CI environment"
    echo "It will alter a system, and should not be run on workstations or alike"
    echo "You can export CI=1 to prevent this error from being shown again"
    exit 3
fi

case "$DOCKER_PLATFORM" in
    linux/amd64|linux/arm/v7|linux/arm64/v8)
        ;;
    *)
        echo "Error: unsupported architecture: $DOCKER_PLATFORM"
        exit 4
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
    libssl-dev
    rpm
    rpm2cpio
    liblzma-dev

    # cross-compiling for 32-bit is only really easy with clang, where we can specify the target as a compiler option
    # clang -target arm-linux-gnueabihf ...
    clang

    qtbase5-dev
    qt5-qmake
    qtdeclarative5-dev

    # libappimage
    libgcrypt-dev
    argagg-dev
    nlohmann-json3-dev
    libgpgme-dev
    libzstd-dev

    # linuxdeploy-plugin-native_packages
    pipx
)

# install 32-bit build dependencies and multilib/cross compilers for binfmt-bypass's 32-bit preload library
if [[ "$DOCKER_PLATFORM" == "linux/amd64" ]]; then
    dpkg --add-architecture i386
    packages+=(
        libc6-dev:i386
    )
elif [[ "$DOCKER_PLATFORM" == "linux/arm64/v8" ]]; then
    dpkg --add-architecture armhf
    packages+=(
        libc6-dev:armhf
    )
fi

# headless install
export DEBIAN_FRONTEND=noninteractive

apt-get update
apt-get -y --no-install-recommends install "${packages[@]}"

# install more recent CMake
cmake_arch="$(dpkg --print-architecture)"

case "$cmake_arch" in
    amd64)
        cmake_arch=x86_64
        ;;
    arm64)
        cmake_arch=aarch64
        ;;
esac

wget https://artifacts.assassinate-you.net/prebuilt-cmake/cmake-v3.29.6-ubuntu-focal-"${cmake_arch}".tar.gz -O - | \
    tar xz --strip-components=1 -C /usr

# install recent patchelf
patchelf_arch="$cmake_arch"
wget https://github.com/NixOS/patchelf/releases/download/0.18.0/patchelf-0.18.0-"$patchelf_arch".tar.gz -O- | tar xz -C/usr/local

  # g{cc,++}-multilib usually install these dependencies for us
  # however, as the multilib stuff is not available for ARM, we have to install these dev packages ourselves
  # we can't really predict the names of the packages (they differ on different distros/releases)
  # therefore, we have to install the other dependencies first to be able to query them with dpkg -l
if [[ "$DOCKER_PLATFORM" == "linux/arm64/v8" ]] || [[ "$DOCKER_PLATFORM" == "linux/amd64" ]]; then
    if [[ "$DOCKER_PLATFORM" == "linux/amd64" ]]; then
        arch_32bit=i386
    elif [[ "$DOCKER_PLATFORM" == "linux/arm64/v8" ]]; then
        arch_32bit=armhf
    else
        echo "Cannot determine 32-bit architecture matching 64-bit architecture $ARCH"
        exit 6
    fi

    apt-get install -y \
        "$(dpkg -l | grep libgcc | grep dev | awk '{print $2}' | cut -d: -f1 | uniq)":"$arch_32bit" \
        "$(dpkg -l | grep libstdc++ | grep dev | awk '{print $2}' | cut -d: -f1 | uniq)":"$arch_32bit"
fi
