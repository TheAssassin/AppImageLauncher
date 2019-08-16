#! /bin/bash

set -e

if [[ "$VERBOSE" != "" ]]; then
    set -x
fi

# shift does not work if no arguments have been passed, therefore we just handle that situation right now
if [[ "$1" == "" ]]; then
    echo "Error: no option passed, use --help for more information"
    exit 2
fi

firstarg="$1"
shift


prefix="appimagelauncher-lite"
install_dir=~/.local/lib/appimagelauncher-lite
installed_appimage_path="$install_dir"/appimagelauncher-lite.AppImage
settings_desktop_file_path=~/.local/share/applications/"$prefix"-AppImageLauncherSettings.desktop
systemd_user_units_dir=~/.config/systemd/user/
appimagelauncherd_systemd_service_name=appimagelauncherd.service
integrated_icon_path=~/.local/share/icons/hicolor/scalable/apps/AppImageLauncher-Lite.svg
no_desktop_integration_marker_path=~/.local/share/appimagekit/no_desktopintegration

test_globally_installed() {
    which AppImageLauncher &>/dev/null && return 0
    type AppImageLauncher &>/dev/null && return 0

    # SC2144 -d doesn't work with globs, using a loop therefore
    for i in /usr/lib/*/appimagelauncher; do
        if [[ -d "$i" ]]; then
            return 0
        fi
    done

    return 1
}

test_installed_already() {
    [[ -d "$install_dir" ]] && return 0

    return 1
}

test_root_user() {
    [[ "$(id -u)" -eq 0 ]]
}

ail_lite_notify_desktop_integration() {
    update-desktop-database ~/.local/share/applications
    gtk-update-icon-cache
}

ail_lite_install() {
    if [[ "$APPIMAGE" == "" ]]; then
        echo "Error: not running from AppImage, aborting"
        return 2
    fi

    # create default Applications directory
    mkdir -p ~/Applications

    # prepare install dir
    mkdir -p "$install_dir"
    mkdir -p "$install_dir"/systemd

    # copy ourselves to install dir
    cp "$APPIMAGE" "$installed_appimage_path"

    # set up appimagelauncherd
    cat > "$install_dir"/systemd/"$appimagelauncherd_systemd_service_name" <<EOF
[Unit]
Description=AppImageLauncher daemon

[Service]
ExecStart=${installed_appimage_path} appimagelauncherd
Restart=on-failure
RestartSec=10

[Install]
WantedBy=default.target
EOF

    # now we need to make systemd aware of your service file
    mkdir -p "$systemd_user_units_dir"
    ln -s "$install_dir"/systemd/"$appimagelauncherd_systemd_service_name" "$systemd_user_units_dir"/

    systemctl --user daemon-reload
    systemctl --user enable "$appimagelauncherd_systemd_service_name"
    systemctl --user restart "$appimagelauncherd_systemd_service_name"

    # set up desktop file for AppImageLauncherSettings
    cat > "$settings_desktop_file_path" <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Exec=${installed_appimage_path} AppImageLauncherSettings %f
Name=AppImageLauncher Settings
Icon=AppImageLauncher-Lite
Terminal=false
Categories=Settings;
X-AppImage-Integrate=false
StartupWMClass=AppImageLauncherSettings
EOF

    # copy icon for AppImageLauncherSettings
    # TODO: copy PNG icons, too
    install "$APPDIR"/usr/share/icons/hicolor/scalable/apps/AppImageLauncher.svg "$integrated_icon_path"

    # notify desktop of changes
    ail_lite_notify_desktop_integration

    # Suppress desktop integration scripts in AppImages
    mkdir -p $(dirname "$no_desktop_integration_marker_path")
    touch "$no_desktop_integration_marker_path"

    echo "AppImageLauncher Lite has been installed successfully."
    return 0
}

ail_lite_uninstall() {
    # remove appimagelauncherd systemd stuff
    systemctl --user stop "$appimagelauncherd_systemd_service_name"
    systemctl --user disable "$appimagelauncherd_systemd_service_name"
    systemctl --user daemon-reload

    # remove all the installed files
    rm -r "$install_dir"

    # remove desktop integration of several tools
    rm "$settings_desktop_file_path"
    rm "$integrated_icon_path"
    ail_lite_notify_desktop_integration

    # Attempt to remove desktop integration scripts in AppImages suppression
    [[ -f "$no_desktop_integration_marker_path" ]] && rm "$no_desktop_integration_marker_path"
    [[ -d $(dirname "$no_desktop_integration_marker_path") ]] && rmdir $(dirname "$no_desktop_integration_marker_path")

    echo "AppImageLauncher Lite has been uninstalled successfully."
    return 0
}

print_help() {
    echo "Usage: $0 <option> ..."
    echo
    echo "Main options:"
    echo "  install       Install AppImageLauncher into your user account"
    echo "  uninstall     Uninstall AppImageLauncher from your user account"
    echo "  help|--help   Display this help"
    echo
    echo "Other options (mainly for use by AppImageLauncher Lite internally):"
    echo "  appimagelauncherd          Run appimagelauncherd"
    echo "  AppImageLauncherSettings   Display AppImageLauncher Lite configuration utility"
    echo "  cli [or ali-cli]           Run AppImageLauncher cli (use \"cli --help\" for more information)"
    echo "  remove <path>              Run removal helper to remove AppImage <path>"
    echo "  update <path>              Run update helper to update AppImage <path>"
}

# ensure this important variable is available, as most operations (except for install) can be done without being run
# from via AppImage runtime (e.g., while the AppImage is extracted, which is great for testing)
export APPDIR=${APPDIR:-$(readlink -f $(dirname "$0"))}

case "$firstarg" in
    help|--help)
        print_help
        exit 0
        ;;
    appimagelauncherd|AppImageLauncherSettings)
        exec "$APPDIR"/usr/bin/"$firstarg" "$@"
        ;;
    cli|ail-cli)
        exec "$APPDIR"/usr/bin/ail-cli "$@"
        ;;
    remove|update)
        #exec "$APPDIR"/usr/lib/**/appimagelauncher/"$firstarg" "$@"
        exec "$APPDIR"/usr/bin/"$firstarg" "$@"
        ;;
    install)
        if test_globally_installed; then
            echo "Error: AppImageLauncher is installed system-wide already, not installing on top" 1>&2
            exit 2
        fi

        if test_root_user; then
            tput setaf 1
            tput bold
            cat <<EOF
##################################################################
#                                                                #
#   ██╗    ██╗ █████╗ ██████╗ ███╗   ██╗██╗███╗   ██╗ ██████╗    #
#   ██║    ██║██╔══██╗██╔══██╗████╗  ██║██║████╗  ██║██╔════╝    #
#   ██║ █╗ ██║███████║██████╔╝██╔██╗ ██║██║██╔██╗ ██║██║  ███╗   #
#   ██║███╗██║██╔══██║██╔══██╗██║╚██╗██║██║██║╚██╗██║██║   ██║   #
#   ╚███╔███╔╝██║  ██║██║  ██║██║ ╚████║██║██║ ╚████║╚██████╔╝   #
#    ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═══╝ ╚═════╝    #
#                                                                #
##################################################################
EOF
            tput sgr0

            echo
            echo "Installation as root is **not** supported or recommended!"
            echo "The installation routine is designed only for regular users."
            echo "If you run this with e.g., sudo, please re-run without, root permissions"
            echo "are not required by this installer."
            echo
            read -p "Continue at your own risk? [yN] " -n1 -r
            echo

            case "$REPLY" in
                y|Y)
                    echo "Continuing installation as per user's request"
                    ;;
                *)
                    return 1
                    ;;
            esac
        fi

        if test_installed_already; then
            echo "Error: AppImageLauncher Lite is installed already, please uninstall before trying to reinstall" 1>&2
            exit 2
        fi

        echo "Installing AppImageLauncher Lite"
        ail_lite_install
        ;;
    uninstall)
        if ! test_installed_already; then
            echo "Error: AppImageLauncher Lite does not seem to be installed" 1>&2
            exit 2
        fi

        echo "Uninstalling AppImageLauncher Lite"
        ail_lite_uninstall
        ;;
    *)
        echo "Unknown operation: $firstarg";
        exit 2
        ;;
esac



