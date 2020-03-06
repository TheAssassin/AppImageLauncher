# How to build AppImageLauncher

If there are no pre-build binaries for your platform you may consider to build AppImageLauncher yourself. Pleas follow these instructions.

## Download source
```
git clone https://github.com/TheAssassin/AppImageLauncher.git
cd AppImageLauncher
git submodule update --init --recursive
```

## Dependencies

All dependencies need to be installed as development versions. In most distributions, you just need to append `-dev` or `-devel` to the listed names. Some names may differ on your system.

 - make
 - cmake
 - glib-2
 - cairo
 - librsvg
 - fuse
 - libarchive
 - libXpm
 - qt5
 - libcurl
 - boost

## Build

Please update the `PREFIX` if you want.

```shell
export PREFIX="/usr/local/"
mkdir build
cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DUSE_SYSTEM_GTEST=ON \
    -DUSE_SYSTEM_XZ=ON \
    -DUSE_SYSTEM_LIBARCHIVE=ON \
    -DBUILD_TESTING=OFF
make libappimage libappimageupdate libappimageupdate-qt
cmake .
make
sudo make install
```
