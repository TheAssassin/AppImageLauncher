<p align="center">
<img src="https://github.com/TheAssassin/AppImageLauncher/raw/master/resources/doc/screenshot.png"/>
</p>

# AppImageLauncher

<p align="center">
<img src="https://github.com/TheAssassin/AppImageLauncher/raw/master/resources/icons/hicolor/128x128/apps/AppImageLauncher.png"/>
</p>

AppImageLauncher makes your Linux desktop AppImage ready™. By installing it, you won't ever have to worry about AppImages again. You can always double click them without making them executable first, just like you should be able to do nowadays. You can integrate AppImages with a single mouse click, and manage them from your application launcher. Updating and removing AppImages becomes as easy as never before.

Due to its simple but efficient way to integrate into your system, it plays well with other applications that can be used to manage AppImages, for example app stores. However, it doesn't depend on any of those, and can run completely standalone.

Install AppImageLauncher today for your distribution and enjoy using AppImages as easy as never before!

> ### :exclamation: Please also check the [Wiki](https://github.com/TheAssassin/AppImageLauncher/wiki) for information how to install and use AppImageLauncher


## Features

### AppImage desktop integration

The core feature of AppImageLauncher is the so-called desktop integration. AppImageLauncher allows you to integrate AppImages you download into your application menu or launcher to make it easier for you to launch them. It also takes care of moving them into a central location, where you can find them later if you need access to them again. Furthermore, it sets up the update and removal entries in the launcher for you.

### Update management

AppImageLauncher provides a simple to use update mechanism. After desktop integration, the context menu of the AppImage's entry in the application launcher will have an entry for updating that launches a little helper tool that uses AppImageUpdate internally. Just click the entry and have the tool search and apply updates.

### Remove AppImages from system

Removing integrated AppImages is pretty simple, too. Similar to updating AppImages, you will find an entry in the context menu in the application launcher that triggers a removal tool. You will be asked to confirm the removal. If you choose to do so, the desktop integration is undone, and the file is removed from your system.

### CLI

For automation, AppImageLauncher provides a CLI tool called `ail-cli` that provides the most basic operations and can be used in scripts or generally from the terminal in headless environments. As of February 2020, only integration and unintegration are supported. More features planned!


## Differences between regular and Lite version

Starting with version 1.4.0, there's an edition of AppImageLauncher called *Lite*. AppImageLauncher Lite is pretty much the best of AppImageLauncher you can get without having root access to your computer. AppImageLauncher Lite is shipped as an AppImage that can be installed by a user from the command line, e.g. by calling `./appimagelauncher-lite...AppImage install`. The AppImage integrates itself in the users' home directory then.

**It is highly recommended to go with the traditional packages *if possible*, as they provide many more features and provide a much better overall experience.**


## About the project

AppImages and Linux desktops, that's two things which don't work together very well currently. Since AppImages are normal executables, it'd suffice if desktop environments like KDE, GNOME, Xfce, ... would assist users in making those files executable, but as we learned recently, some desktop environments consider this a security risk, and want to force users to use app stores.

Being executable isn't really all that is needed to provide a good desktop experience. AppImages should be accessible from the application menus and launchers. This so-called "desktop integration" can't be provided by the AppImages themselves even though some AppImages ship with a "desktop integration script" prompting the user to integrate the AppImages, as there's too many impliciations that require an external software, especially regarding the cleanup and removal of AppImages. Also, if applications are simply made executable, they're still spread all over the users' personal files and folders. The average user doesn't necessarily like a Downloads directory that is full of AppImages with cryptic filenames.

Therefore, new, system-side solutions have been developed to perform the desktop integration. The oldest available solution is [appimaged](https://github.com/AppImage/AppImageKit), a daemon users can install that performs everything in the background, automagically, without notifying the user in any way. It scans a predefined set of directories including `~/Downloads` and `~/.bin`, makes AppImages which are found executable and performs the desktop integration. This is rather inefficient, as appimaged's operations and monitoring produce a lot of file I/O. Also, many users don't like the lack of control.

A new solution for native AppImage support has been developed: AppImageLauncher. AppImageLauncher integrates deeply in the system and intercepts all attempts to open an AppImage, becoming the first instance to handle all AppImage invocations.

Being the launcher for AppImages, AppImageLauncher can control how the system treats AppImages. It can perform the desktop integration, AppImage removal (also called "uninstallation" sometimes, but as AppImages are not really installed, this term doesn't fit very well), and could be used for many other tasks in the future, like update checks and alike.


## Media and publications

### :movie_camera: :vhs: Videos & Podcasts

Tutorials:

- https://invidio.us/watch?v=D2WA2zdLvVk


Podcasts:

- https://dlnxtend.com/11
  - also: https://invidio.us/watch?v=yaZygqyN_KE


### :page_facing_up: Articles

A few articles have been written about AppImageLauncher already:

  - https://www.linuxuprising.com/2018/04/easily-run-and-integrate-appimage-files.html (English)
  - https://www.freeyourdesktop.com/2018/07/install-manage-appimages-with-appimagelauncher/ (English)
    - same article also available here: https://medium.com/@freeyourdesktopblog/install-manage-appimages-with-appimagelauncher-2a2078c55f37
  - http://linux-os.net/appimagelauncher-ejecuta-e-integra-facilmente-aplicaciones-en-appimage/ (Spanish)
    - same article also available here: https://blog.desdelinux.net/appimagelauncher-ejecuta-e-integra-facilmente-aplicaciones-en-appimage/
  - http://www.edivaldobrito.com.br/integrador-appimagelauncher-no-linux/ (Portuguese)
  - https://404.g-net.pl/2018/08/appimagelauncher/ (Polish)
  - https://linuxmint.hu/blog/2018/12/appimage (Hungarian)
  - https://www.freeyourdesktop.com/2018/07/install-manage-appimages-with-appimagelauncher/ (English)
    - please note that AppImageLauncher could *not* be "installed" via AppImage at that time, only recently we added a Lite version that now can be installed from an AppImage (more information will follow soon!)
  - https://forum.pardus.org.tr/t/appimagelauncher-tek-tiklama-ile-appimage-sisteminizle-butunlestiriniz/11275 (Turkish)
  - https://linuxteamvietnam.us/cach-chay-ung-dung-dinh-dang-appimage-tren-linux/ (Vietnamese)
    - contains a few wrong assumptions: as soon as AppImageLauncher is installed, you do not have to make AppImages executable, you can just double-click tem (or otherwise open them, e.g., from your browsers' downloads)
  - https://www.reallinuxuser.com/15-best-things-to-do-after-installing-zorin-os-15/ (English)
  - https://manjaro.site/how-to-install-appimage-launcher-on-ubuntu-19-04/ (English)
  - https://cubiclenate.com/2020/01/09/appimagelauncher-appimage-manager-on-opensuse/ (English)
    - see also: https://cubiclenate.com/2020/01/10/noodlings-lighting-the-emby-server-with-kdenlive/
  - https://chrisrmiller.com/2019/05/29/integrate-appimages-into-your-linux-distro/ (English)
  - https://medium.com/@freeyourdesktopblog/install-manage-appimages-with-appimagelauncher-2a2078c55f37 (English)
  - https://dolys.fr/forums/topic/gerer-les-appimage-sous-linux/ (French)


## Installation

### System wide Installation

AppImageLauncher is supposed to integrate deeply in the systems. Therefore, an installation via native system packages is the preferred way to install AppImageLauncher. This way, AppImageLauncher's package can perform the necessary steps to have your system use it for all AppImage invocations.

Compatibility table (likely incomplete, please feel free to send PRs to add distributions)

| Release filename | Build system | Compatible distributions (incomplete) |
| ---------------- | ------------ | ------------------------------------- |
| `appimagelauncher-<version>.xenial_(amd64,i386).deb` | Ubuntu xenial | Ubuntu xenial (16.04), Debian stretch (9), Netrunner 17.01 |
| `appimagelauncher-<version>.bionic_(amd64,i386).deb` | Ubuntu bionic | Ubuntu bionic (18.04), Ubuntu disco (19.04), Ubuntu eoan (19.10) and newer, Debian buster (1) and newer, Netrunner 19.01, 19.08 and newer |
| ~~`appimagelauncher-<version>.disco_(amd64,i386).deb`~~  | ~~Ubuntu disco~~  | ~~Ubuntu disco (19.04)~~ |
| ~~`appimagelauncher-<version>.eoan_(amd64,i386).deb`~~   | ~~Ubuntu eoan~~   | ~~Ubuntu eoan (19.10) and newer~~ |
| ~~`appimagelauncher-<version>.buster_(amd64,i386).deb`~~ | ~~Debian buster~~ | ~~Debian buster (10) and newer, Netrunner 19.01 and 19.08~~ |
| `appimagelauncher-<version>.(i386,x86_64).rpm`       | Ubuntu xenial | openSUSE Leap 42 and newer, possibly openSUSE Tumbleweed, SUSE Enterprise Linux, RHEL 7, CentOS 7 |

  - Ubuntu trusty (14.04) and newer
    - **Important:** Ubuntu bionic (and newer) broke with the backwards compatibility of its `libcurl` packages, therefore users of these systems need to install the special `bionic` package
  - Debian stable (jessie, 8) and newer
  - Netrunner 17 and newer
  - openSUSE Leap 42 and newer
  - openSUSE Tumbleweed

The installation of packages on systems with a set of packages similar to one of the listed ones (e.g., Linux Mint, Fedora, etc.) should work as well.

Manjaro and Netrunner Rolling users can install AppImageLauncher with a distribution-provided package called `appimagelauncher`.

Arch Linux, Manjaro, Antergos and Netrunner Rolling users can use AUR to install AppImageLauncher by installing [appimagelauncher-git](https://aur.archlinux.org/packages/appimagelauncher-git) (thanks @NuLogicSystems for setting up the build).

Other systems derived from the listed ones, such as for instance Linux Mint (Ubuntu), should support AppImageLauncher as well. If they don't, please don't hesitate to create an issue on GitHub.

**Note:** If your system is not listed above as supported, please feel free to request support in an issue on GitHub. We can then discuss adding support.   


## How it works

AppImageLauncher is responsible for the desktop integration. When the user launches an AppImage, the software checks whether the AppImage has been integrated already. If not, it displays a dialog prompting the user whether to run the AppImage once, or move it to a predefined location and adding it to the application menus, launchers, etc.


## Technical background information

Details about how AppImageLauncher registers itself in the system can be found on [this Wiki page](https://github.com/TheAssassin/AppImageLauncher/wiki/Idea).
