# AppImageLauncher
makes your Linux desktop AppImage ready™
<p align="center">
<img src="https://github.com/TheAssassin/AppImageLauncher/raw/master/resources/icons/hicolor/128x128/apps/AppImageLauncher.png"/>
</p>

Integrate AppImages to your application launcher with one click, and manage, update and remove them from there.
Double-click AppImages to open them, without having to make them executable first.

AppImageLauncher plays well with other applications managing AppImages, for example app stores.
However, it doesn't depend on any of those, and can run completely standalone.

<p align="center">
<img src="https://github.com/TheAssassin/AppImageLauncher/raw/master/resources/doc/screenshot.png"/>
</p>

> ### :smiley: Info on how to install and use AppImageLauncher on the [wiki](https://github.com/TheAssassin/AppImageLauncher/wiki).

AppImageLauncher is a novel and unique solution of integrating with the system. It intercepts all attempts to open an AppImage to provide its integration features.

Being the launcher for AppImages, AppImageLauncher can control how the system treats AppImages. It integrates them into the system, provides helpers for updating or removing AppImages, and a lot more.

On their first execution (i.e., if they have not been integrated yet), it displays a dialog prompting the user whether to run the AppImage once, or move it to a predefined location and adding it to the application menus, launchers, etc.


## Features

### AppImage desktop integration

This core feature allows you to integrate AppImages you download into your application menu or launcher, to make it easier for you to launch them. It also takes care of moving them into a central location, where you can find them later.
Furthermore, it sets up the update and removal entries in the launcher for you.

AppImages use the term "desktop integration", as they're not "installed" in the traditional sense. They remain single, self-contained executable files. AppImageLauncher and other tools extract and patch the [desktop entry](https://specifications.freedesktop.org/desktop-entry-spec/latest/) as well as the related [icons](https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html) into the relevant locations.
More information on desktop integration can be found in the [AppImage docs](https://docs.appimage.org/reference/desktop-integration.html).


### Update management

After desktop integration, the context menu of the AppImage's entry in the application launcher will have an "Update" entry that launches a little helper tool to apply updates.


### Remove AppImages from system

Click the "Remove" entry in the context menu in the application launcher and the removal tool will ask you to confirm.
If you choose to do so, the desktop integration is undone, and the file is removed from your system.


### CLI

The packages ship with a CLI tool called `ail-cli`, providing basic operations in the terminal, for automation in scripts etc. As of February 2020, only integration and unintegration are supported. More features planned!


## Differences between regular and Lite version

From version 1.4.0, there's a *Lite* edition of AppImageLauncher.
It provides all of AppImageLauncher you can get without having root access to your computer. AppImageLauncher Lite is shipped as an AppImage, which can be installed by users from the command-line.
Enter `./appimagelauncher-lite...AppImage install`. in a terminal, and it integrates itself in the users' home directory.

**Traditional packages are highly recommended *if possible*, as they provide many more features and provide a much better overall experience.**

*Note: if you're interested in a GUI installer, please consider sending a pull request. More information in [#243](https://github.com/TheAssassin/AppImageLauncher/issues/243).*

## About

Currently, desktop environments consider exexutables a security risk, and would rather have users use the app stores they include.

The ability to easily run them, however, isn't all that's needed to provide a good AppImage desktop experience. Making them accessible from the application menus and launchers is a level of "desktop integration" that can't be provided by the AppImages themselves properly (even though some AppImages ship with a "desktop integration script" prompting the user to do so). There are too many impliciations requiring external software, especially regarding cleanup and removal of AppImages. (If applications are simply made executable, they're still spread all over the users' personal files and folders.) The average user will not find a "Downloads" directory full of AppImages with cryptic filenames friendly.

Therefore, system-side ways have been developed to perform the desktop integration.
One of the first solutions was[appimaged](https://github.com/AppImage/appimaged), a daemon users could install to perform everything automagically in the background, without notifying the user in any way.
It scans a predefined set of directories including `~/Downloads` and `~/.bin`, making recognized AppImages executable and then performing the desktop integration. Those operations and monitoring produced a lot of file I/O, so were rather inefficent. Also, many users don't like the lack of control. The approach also opens attack vectors and thus can be considered a security hazard, as a vulnerability discovered in appimaged recently has shown.


## As seen in

### :movie_camera: Tutorials:

* [Eric Adams](https://invidio.us/watch?v=D2WA2zdLvVk)


### :sound: Podcasts:

* [DLN Xtend 11h](ttps://dlnxtend.com/11)
  + [Invidious](https://invidio.us/watch?v=yaZygqyN_KE)
* [DLN Xtend 14](https://dlnxtend.com/14)
  + [Invidious](https://invidio.us/watch?v=QCkJ74kOlGQ)


### :page_facing_up: Articles

  * [Linux Uprising](https://www.linuxuprising.com/2018/04/easily-run-and-integrate-appimage-files.html) (English)
  * [Linux-OS.net](https://linux-os.net/appimagelauncher-ejecuta-e-integra-facilmente-aplicaciones-en-appimage/) (Spanish)
    + Same article also available [here](https://blog.desdelinux.net/appimagelauncher-ejecuta-e-integra-facilmente-aplicaciones-en-appimage/).
  * [Edvaldo Brito](https://www.edivaldobrito.com.br/integrador-appimagelauncher-no-linux/) (Portuguese)
  * [przystajnik](https://404.g-net.pl/2018/08/appimagelauncher/) (Polish)
  * [Linux Mint Magyar Közösség](https://linuxmint.hu/blog/2018/12/appimage) (Hungarian)
  * [FreeYourDesktop](https://medium.com/@freeyourdesktopblog/install-manage-appimages-with-appimagelauncher-2a2078c55f37) (English)
    + Please note that AppImageLauncher could *not* be "installed" via AppImage at that time, only recently was a Lite version added, that now can be installed from an AppImage (more info to follow.)
  * [Pardus forum](https://forum.pardus.org.tr/t/appimagelauncher-tek-tiklama-ile-appimage-sisteminizle-butunlestiriniz/11275) (Turkish)
  * [Linux Team Vietnam](https://linuxteamvietnam.us/cach-chay-ung-dung-dinh-dang-appimage-tren-linux/) (Vietnamese)
    + Contains a few wrong assumptions: as soon as AppImageLauncher is installed, you do not have to make AppImages executable, you can just double-click them (or otherwise open them, e.g., from your file- or web-browsers' downloads).
  * [RealLinuxUser](https://www.reallinuxuser.com/15-best-things-to-do-after-installing-zorin-os-15/) (English)
  * [manjaro.site](https://manjaro.site/how-to-install-appimage-launcher-on-ubuntu-19-04/) (English)
  * [CubicleNate's Techpad](https://cubiclenate.com/2020/01/09/appimagelauncher-appimage-manager-on-opensuse/) (English)
    + See also: [CubicleNate's Techpad](https://cubiclenate.com/2020/01/10/noodlings-lighting-the-emby-server-with-kdenlive/)
  * [CHRIS R MILLER](https://chrisrmiller.com/2019/05/29/integrate-appimages-into-your-linux-distro/) (English)
  * [doLys Forum](https://dolys.fr/forums/topic/gerer-les-appimage-sous-linux/) (French)
  * [LINUXUSER](https://linux-user.gr/t/eykolh-chrhsh-efarmogwn-appimage/2066) (Greek)


## Installation

### System-wide Installation

AppImageLauncher integrates deeply into the system. Therefore, an installation via native system packages is the preferred way to install AppImageLauncher. This way, AppImageLauncher's package can perform the necessary steps to have your system use it for all AppImage invocations.

Compatibility table (likely incomplete.) Please, feel free to open PRs to add distributions.

| Release filename | Build system | Compatible distributions (incomplete) |
| ---------------- | ------------ | ------------------------------------- |
| `appimagelauncher-<version>.xenial_(amd64,i386).deb` | Ubuntu Xenial | Ubuntu Xenial (16.04), Debian Stretch (9), Netrunner 17.01 |
| `appimagelauncher-<version>.bionic_(amd64,i386).deb` | Ubuntu Bionic | Ubuntu Bionic (18.04), Ubuntu Disco (19.04), Ubuntu Eoan (19.10) and newer, Debian buster (10) and newer, Netrunner 19.01, 19.08 and newer |
| ~~`appimagelauncher-<version>.disco_(amd64,i386).deb`~~  | ~~Ubuntu Disco~~  | ~~Ubuntu Disco (19.04)~~ |
| ~~`appimagelauncher-<version>.eoan_(amd64,i386).deb`~~   | ~~Ubuntu Eoan~~   | ~~Ubuntu Eoan (19.10) and newer~~ |
| ~~`appimagelauncher-<version>.buster_(amd64,i386).deb`~~ | ~~Debian Buster~~ | ~~Debian Buster (10) and newer, Netrunner 19.01 and 19.08~~ |
| `appimagelauncher-<version>.(i386,x86_64).rpm`       | Ubuntu Xenial | openSUSE Leap 42 and newer, possibly openSUSE Tumbleweed, SUSE Enterprise Linux, RHEL 7, CentOS 7 |

  - Ubuntu Trusty (14.04) and newer
    - **Important:** Ubuntu Bionic (and newer) broke with the backwards compatibility of its `libcurl` packages, therefore users of these systems need to install the special `bionic` package
  - Debian stable (Jessie, 8) and newer
  - Netrunner 17 and newer
  - openSUSE Leap 42 and newer
  - openSUSE Tumbleweed

The installation of packages on systems with a set of packages similar to one of the listed ones (e.g., Linux Mint, Fedora, etc.) should work as well.

**Note:** For Ubuntu 19.04 and newer as well as derivatives, there's two PPAs (personal package archives) available providing a simple way to install and update AppImageLauncher (thanks [Peter Jonas](https://github.com/shoogle) for creating and maintaining them):

  - stable (provides all releases): https://launchpad.net/~appimagelauncher-team/+archive/ubuntu/stable
  - daily (latest and greatest but possibly unstable): https://launchpad.net/~appimagelauncher-team/+archive/ubuntu/daily

Manjaro and Netrunner Rolling users can install AppImageLauncher with a distribution-provided package called `appimagelauncher`.

Arch Linux, Manjaro, Antergos and Netrunner Rolling users can use AUR to install AppImageLauncher by installing [appimagelauncher-git](https://aur.archlinux.org/packages/appimagelauncher-git) (thanks @NuLogicSystems for setting up the build).

Other systems derived from the listed ones, such as for instance Linux Mint (Ubuntu), should support AppImageLauncher as well.
If they don't, please don't hesitate to create an issue.

**Note:** Feel free to request support for other distributions by [opening an issue](https://github.com/TheAssassin/AppImageLauncher/issues/new).


### Build from source

Build instructions in [BUILD.md](BUILD.md).


## Background

Technical details about how AppImageLauncher registers itself on [this wiki page](https://github.com/TheAssassin/AppImageLauncher/wiki/Idea).
