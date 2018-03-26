<p align="center">
<img src="https://github.com/TheAssassin/AppImageLauncher/raw/master/resources/doc/screenshot.png"/>
</p>

# AppImageLauncher

AppImages and Linux desktops, that's two things which don't work together very well currently. Since AppImages are normal executables, it'd suffice if desktop environments like KDE, GNOME, Xfce, ... would assist users in making those files executable, but as we learned recently, some desktop environments consider this a security risk, and want to force users to use app stores.

Being executable isn't really all that is needed to provide a good desktop experience. AppImages should be accessible from the application menus and launchers. This so-called "desktop integration" can't be provided by the AppImages themselves even though some AppImages ship with a "desktop integration script" prompting the user to integrate the AppImages, as there's too many impliciations that require an external software, especially regarding the cleanup and removal of AppImages. Also, if applications are simply made executable, they're still spread all over the users' personal files and folders. The average user doesn't necessarily like a Downloads directory that is full of AppImages with cryptic filenames.

Therefore, new, system-side solutions have been developed to perform the desktop integration. The oldest available solution is [appimaged](https://github.com/AppImage/AppImageKit), a daemon users can install that performs everything in the background, automagically, without notifying the user in any way. It scans a predefined set of directories including `~/Downloads` and `~/.bin`, makes AppImages which are found executable and performs the desktop integration. This is rather inefficient, as appimaged's operations and monitoring produce a lot of file I/O. Also, many users don't like the lack of control.

A new solution for native AppImage support has been developed: AppImageLauncher. AppImageLauncher integrates deeply in the system and intercepts all attempts to open an AppImage, becoming the first instance to handle all AppImage invocations.

Being the launcher for AppImages, AppImageLauncher can control how the system treats AppImages. It can perform the desktop integration, AppImage removal (also called "uninstallation" sometimes, but as AppImages are not really installed, this term doesn't fit very well), and could be used for many other tasks in the future, like update checks and alike.


## Installation

### System wide Installation

AppImageLauncher is supposed to integrate deeply in the systems. Therefore, an installation via native system packages is the preferred way to install AppImageLauncher. This way, AppImageLauncher's package can perform the necessary steps to have your system use it for all AppImage invocations.

At the moment, only Debian-based systems are supported. The packages are built on Ubuntu trusty, and _should_ work on all major systems releasted after 2014. You can find the latest builds of the `.deb` archive on the [release page](https://github.com/TheAssassin/AppImageLauncher/releases).
Please beware that the version number is merely cosmetic, and will not change, although the package may be updated. As AppImageLauncher is still under heavy development, there is no package archive provided for you to use, so you will have to install it by hand. Please refer to your operating system's manual to find out how to install the packages. There is a step-by-step [tutorial on askubuntu.com](https://askubuntu.com/questions/40779/how-do-i-install-a-deb-file-via-the-command-line) that shows how to install them using a terminal. If you prefer to use the graphical user interface, you should be able to double-click the downloaded file, or drag-and-drop the package into the software center, which will take care of the installation.

### AppImage

For evaluation purposes, you can download an AppImage from the [release page](https://github.com/TheAssassin/AppImageLauncher/releases), which will demonstrate how AppImageLauncher works. Please beware that certain features, like the removal of AppImages, require AppImageLauncher to be installed system wide.


## How it works

AppImageLauncher is responsible for the desktop integration. When the user launches an AppImage, the software checks whether the AppImage has been integrated already. If not, it displays a dialog prompting the user whether to run the AppImage once, or move it to a predefined location and adding it to the application menus, launchers, etc.


## Technical background information

Details about how AppImageLauncher registers itself in the system can be found on [this Wiki page](https://github.com/TheAssassin/AppImageLauncher/wiki/Idea).
