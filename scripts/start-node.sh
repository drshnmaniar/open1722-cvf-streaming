#!/bin/sh
set -eu

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

if [ "$#" -lt 1 ]; then
    printf 'Usage: %s sender [receiver-ip [port]] | receiver [port]\n' "$0" >&2
    exit 2
fi

node=$1
shift

case "$node" in
    sender)
        target=open1722-sender
        ;;
    receiver)
        target=open1722-receiver
        ;;
    *)
        printf 'Unknown node "%s". Choose sender or receiver.\n' "$node" >&2
        exit 2
        ;;
esac

if [ ! -f build/meson-private/coredata.dat ]; then
    printf '%s\n' "Meson is not configured. Run the Configure Meson task first." >&2
    exit 1
fi

meson compile -C build "$target"
exec "build/$target" "$@"
