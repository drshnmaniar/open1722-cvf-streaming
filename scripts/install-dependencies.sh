#!/bin/sh
set -eu

if ! command -v apt-get >/dev/null 2>&1; then
    printf '%s\n' "This installer supports Debian/Ubuntu-based systems with apt-get." >&2
    exit 1
fi

if [ "$(id -u)" -eq 0 ]; then
    SUDO=
elif command -v sudo >/dev/null 2>&1; then
    SUDO=sudo
else
    printf '%s\n' "Install sudo or run this script as root." >&2
    exit 1
fi

$SUDO apt-get update
$SUDO apt-get install -y \
    build-essential \
    clang-format \
    gstreamer1.0-libav \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-tools \
    libgstreamer1.0-dev \
    meson \
    ninja-build \
    pkg-config
