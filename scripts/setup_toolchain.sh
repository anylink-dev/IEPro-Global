#!/bin/sh
# Download and extract the IE Pro cross-compilation toolchain into toolchain/.
#
# Usage (from repository root, on Linux x86_64):
#   sh scripts/setup_toolchain.sh
#
# The official Linaro direct URL may redirect or fail; this script tries
# community mirrors first.

set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
DEST_DIR="$REPO_ROOT/toolchain"
ARCHIVE_NAME=gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz
TOOLCHAIN_DIRNAME=gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf

MIRROR_1="https://mirrors.dotsrc.org/armbian-dl/_toolchain/${ARCHIVE_NAME}"
MIRROR_2="https://ru.archive.ubuntu.com/mirrors/armbian/dl/_toolchain/${ARCHIVE_NAME}"
OFFICIAL="https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/${ARCHIVE_NAME}"

mkdir -p "$DEST_DIR"

if [ -x "$DEST_DIR/$TOOLCHAIN_DIRNAME/bin/arm-linux-gnueabihf-gcc" ]; then
    echo "Toolchain already installed: $DEST_DIR/$TOOLCHAIN_DIRNAME"
    exit 0
fi

TMP=$(mktemp)
cleanup() { rm -f "$TMP"; }
trap cleanup EXIT

fetch_url() {
    url=$1
    if command -v wget >/dev/null 2>&1; then
        wget -O "$TMP" "$url"
    elif command -v curl >/dev/null 2>&1; then
        curl -fsSL -o "$TMP" "$url"
    else
        echo "Error: wget or curl is required." >&2
        exit 1
    fi
}

for url in "$MIRROR_1" "$MIRROR_2" "$OFFICIAL"; do
    echo "Trying: $url"
    if fetch_url "$url" && tar -tf "$TMP" >/dev/null 2>&1; then
        echo "Download OK."
        tar -xf "$TMP" -C "$DEST_DIR"
        echo "Installed to: $DEST_DIR/$TOOLCHAIN_DIRNAME"
        echo "Next: . scripts/env.toolchain.sh"
        exit 0
    fi
    echo "Failed or invalid archive, trying next mirror..."
    rm -f "$TMP"
    : > "$TMP"
done

echo "All download sources failed." >&2
echo "See toolchain/README.md for manual mirror links." >&2
exit 1
