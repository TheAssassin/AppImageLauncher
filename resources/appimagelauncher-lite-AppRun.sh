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

# ensure this important variable is available, as most operations (except for install) can be done without being run
# from via AppImage runtime (e.g., while the AppImage is extracted, which is great for testing)
export APPDIR=${APPDIR:-$(readlink -f $(dirname "$0"))}

# import common script code
source "$APPDIR"/appimagelauncher-lite-installer-common.sh

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



