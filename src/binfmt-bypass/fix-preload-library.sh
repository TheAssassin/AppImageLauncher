#! /bin/bash

set -euo pipefail

glibc_ok_version="2.4"

find_too_new_symbols() {
    glibc_symbols=( "$(nm --dynamic --undefined-only --with-symbol-versions "$1" | grep "GLIBC_")" )

    for glibc_symbol in "${glibc_symbols[@]}"; do
        # shellcheck disable=SC2001
        glibc_symbol_version="$(sed 's|.*GLIBC_\([\.0-9]\+\)$|\1|' <<< "$glibc_symbol")"
        newest_glibc_symbol_version="$(echo -e "$glibc_ok_version\\n$glibc_symbol_version" | sort -V | tail -n1)"

        # make sure the newest version found is <= the one we define as ok
        if [[ "$newest_glibc_symbol_version" == "$glibc_ok_version" ]]; then
            return 1
        fi
    done

    return 0
}

for file in "$@"; do
    # obviously, this is a hack, but it should work well enough since we just need to do it for one single symbol from libdl
    patchelf --debug --clear-symbol-version dlsym "$file"

    nm_data="$(nm --dynamic --undefined-only --with-symbol-versions "$file")"

    if find_too_new_symbols "$file"; then
        echo "Error: found symbol version markers newer than $glibc_ok_version:"
        echo "$nm_data"
        exit 1
    fi
done
