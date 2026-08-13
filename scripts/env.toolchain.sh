#!/bin/sh
# Activate the IE Pro cross-compilation toolchain for the current shell.
#
# Usage (from repository root):
#   . scripts/env.toolchain.sh
#
# Optional overrides:
#   IEPRO_TOOLCHAIN_ROOT  path to extracted gcc-linaro directory
#   IEPRO_CROSS_COMPILE   compiler prefix (default: arm-linux-gnueabihf-)

iepro_script_path() {
    if [ -n "${BASH_VERSION:-}" ] && [ -n "${BASH_SOURCE:-}" ]; then
        printf '%s\n' "${BASH_SOURCE[0]}"
        return 0
    fi
    if [ -n "${ZSH_VERSION:-}" ]; then
        # shellcheck disable=SC2296
        eval 'printf "%s\n" "${(%):-%x}"'
        return 0
    fi
    printf '%s\n' "$0"
}

_resolve_repo_root() {
    _script=$(iepro_script_path)
    case "$_script" in
        */env.toolchain.sh)
            _dir=$(CDPATH= cd -- "$(dirname -- "$_script")" && pwd)
            CDPATH= cd -- "$_dir/.." && pwd
            return 0
            ;;
    esac

    if [ -f "./scripts/env.toolchain.sh" ]; then
        pwd
        return 0
    fi

    echo "Could not locate the repository root." >&2
    echo "cd to the IEPro-Global repo, then run: . scripts/env.toolchain.sh" >&2
    return 1
}

if ! REPO_ROOT=$(_resolve_repo_root); then
    return 1 2>/dev/null || exit 1
fi

TOOLCHAIN_DIRNAME=gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf
DEFAULT_TOOLCHAIN_ROOT="$REPO_ROOT/toolchain/$TOOLCHAIN_DIRNAME"

if [ -n "${IEPRO_TOOLCHAIN_ROOT:-}" ]; then
    IEPRO_TOOLCHAIN_ROOT=$IEPRO_TOOLCHAIN_ROOT
else
    IEPRO_TOOLCHAIN_ROOT=$DEFAULT_TOOLCHAIN_ROOT
fi
IEPRO_CROSS_COMPILE=${IEPRO_CROSS_COMPILE:-arm-linux-gnueabihf-}

BIN_DIR="$IEPRO_TOOLCHAIN_ROOT/bin"
GCC="$BIN_DIR/${IEPRO_CROSS_COMPILE}gcc"
SYSROOT_CANDIDATE="$IEPRO_TOOLCHAIN_ROOT/arm-linux-gnueabihf/libc"

if [ ! -x "$GCC" ]; then
    echo "Cross compiler not found: $GCC" >&2
    if [ ! -d "$REPO_ROOT/toolchain/$TOOLCHAIN_DIRNAME" ]; then
        echo "Toolchain not installed yet. From the repo root, run:" >&2
        echo "  sh scripts/setup_toolchain.sh" >&2
    fi
    echo "See toolchain/README.md" >&2
    return 1 2>/dev/null || exit 1
fi

export PATH="$BIN_DIR:$PATH"
export CROSS_COMPILE="$IEPRO_CROSS_COMPILE"
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export AR="${CROSS_COMPILE}ar"
export STRIP="${CROSS_COMPILE}strip"
export IEPRO_TOOLCHAIN_ROOT
export IEPRO_CROSS_COMPILE

if [ -d "$SYSROOT_CANDIDATE" ]; then
    export SYSROOT="$SYSROOT_CANDIDATE"
fi

echo "IE Pro toolchain ready: $($GCC --version | head -n 1)"
