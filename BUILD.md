# How to build AppImageLauncher

If there are no pre-build binaries for your platform you may consider to build AppImageLauncher yourself. Please follow these instructions.


## Download source

This step is only required if you would like to build the source from GitHub. *If you have a local copy, please skip this section.*

You can download the source either using Git or by mannually downloading a tarball on the [Releases page](https://github.com/TheAssassin/AppImageLauncher/releases).

The branch can be customized by setting the `-b` or `--branch` argument. Select `stable` to download the current stable source or use `master` to get the most current unstable source containing the latest features.

```shell
git clone https://github.com/TheAssassin/AppImageLauncher.git -b stable
cd AppImageLauncher
git submodule update --init --recursive
```


## Dependencies

All dependencies need to be installed as development libraries. Some names may differ on your system.

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

Please update the `PREFIX` if you want. The prefix is the location the final application will be installed to. Usual locations may be `/usr/local` (default), `/usr`, `~/.local` or `/opt`.

```shell
export PREFIX="/usr/local/"
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX="$PREFIX"

# See https://github.com/TheAssassin/AppImageLauncher/issues/251 for more details why this is required
make libappimage libappimageupdate libappimageupdate-qt
cmake .

make
```

Now you may create a distribution package or alternatively install the source for testing purpose.

*Note: This may harm your system. It's highly recommended to build and install distribution packages instead.*

```shell
sudo make install
```
