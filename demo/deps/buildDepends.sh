#!/usr/bin/env bash
# buildDepends.sh — fetch, cross-compile, and pack third-party libs for demo/deps.
# See demo/deps/README.md.
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
DEPS_ROOT=$SCRIPT_DIR
PACKAGES_DIR=$DEPS_ROOT/packages
BUILD_ROOT=$DEPS_ROOT/build
PREBUILT_DIR=$DEPS_ROOT/prebuilt
SOURCE_FILE=$PACKAGES_DIR/source.txt

CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabihf-}
CROSS_HOST=${CROSS_HOST:-${CROSS_COMPILE%-}}
DEPS_PREFIX=${DEPS_PREFIX:-${IEPRO_DEPS_PREFIX:-$DEPS_ROOT/$CROSS_HOST}}

CC=${CC:-${CROSS_COMPILE}gcc}
AR=${AR:-${CROSS_COMPILE}ar}
RANLIB=${RANLIB:-${CROSS_COMPILE}ranlib}
STRIP=${STRIP:-${CROSS_COMPILE}strip}

FORCE_FETCH=0
FORCE_EXTRACT=0
FORCE_BUILD=0
FORCE=0
STRIP_AFTER_BUILD=${IEPRO_DEPS_STRIP:-0}

declare -A PKG_VERSION=()
declare -A PKG_URL=()
declare -A PKG_EXTRACT_DIR=()
ORDERED_PKG_NAMES=()

log() { printf '>> %s\n' "$*"; }
die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --fetch-sources       Download missing packages/*.tar.gz from packages/source.txt
  --build-all           Fetch (if needed), extract, and build all libraries
  --only <name>         Build one package by section name (e.g. curl, openssl)
  --extract-prebuilt    Extract prebuilt/\${CROSS_HOST}.tar.gz into DEPS_PREFIX
  --pack-prebuilt       Create prebuilt/\${CROSS_HOST}.tar.gz from DEPS_PREFIX
  --clean-build         Remove \$BUILD_ROOT
  --clean-prefix        Remove \$DEPS_PREFIX (requires --force)
  --force               With --clean-prefix or overwrite extract/prebuilt
  --force-fetch         Re-download tarballs even if present
  --force-extract       Re-extract source trees
  --force-build         Ignore .build_completed markers
  --strip               After build, strip ELFs under DEPS_PREFIX (shared libs, bins; skip .a)
  --help                Show this help

Environment:
  CROSS_COMPILE   default: arm-linux-gnueabihf-
  CROSS_HOST      default: arm-linux-gnueabihf
  DEPS_PREFIX     default: \$DEPS_ROOT/\$CROSS_HOST or \$IEPRO_DEPS_PREFIX
  CC, AR, RANLIB, STRIP  cross tools (from env.toolchain.sh)
  IEPRO_DEPS_STRIP  set to 1 to enable --strip by default

Paths:
  DEPS_ROOT=$DEPS_ROOT
  DEPS_PREFIX=$DEPS_PREFIX
  BUILD_ROOT=$BUILD_ROOT
EOF
}

parse_source_txt() {
    local section="" key value

    PKG_VERSION=()
    PKG_URL=()
    PKG_EXTRACT_DIR=()
    ORDERED_PKG_NAMES=()

    [[ -f "$SOURCE_FILE" ]] || die "missing $SOURCE_FILE"

    while IFS= read -r line || [[ -n "$line" ]]; do
        line=${line%%#*}
        line=${line//$'\r'/}
        [[ -z "${line//[[:space:]]/}" ]] && continue

        if [[ "$line" =~ ^\[([a-zA-Z0-9_-]+)\]$ ]]; then
            section=${BASH_REMATCH[1]}
            ORDERED_PKG_NAMES+=("$section")
            continue
        fi

        [[ -z "$section" ]] && continue
        [[ "$line" != *=* ]] && continue

        key=${line%%=*}
        value=${line#*=}
        key=${key//[[:space:]]/}
        value=${value#"${value%%[![:space:]]*}"}

        case "$key" in
            version) PKG_VERSION[$section]=$value ;;
            url) PKG_URL[$section]=$value ;;
            extract_dir) PKG_EXTRACT_DIR[$section]=$value ;;
        esac
    done < "$SOURCE_FILE"

    [[ ${#ORDERED_PKG_NAMES[@]} -gt 0 ]] || die "no packages defined in $SOURCE_FILE"
}

pkg_module_id() {
    local name=$1
    local ver=${PKG_VERSION[$name]:?}
    echo "${name}-${ver}"
}

pkg_extract_dir() {
    local name=$1
    if [[ -n "${PKG_EXTRACT_DIR[$name]:-}" ]]; then
        echo "${PKG_EXTRACT_DIR[$name]}"
    else
        echo "$(pkg_module_id "$name")"
    fi
}

pkg_tarball_path() {
    local name=$1
    echo "$PACKAGES_DIR/$(pkg_module_id "$name").tar.gz"
}

pkg_source_dir() {
    local name=$1
    echo "$PACKAGES_DIR/$(pkg_extract_dir "$name")"
}

download_file() {
    local url=$1 dest=$2
    log "Downloading $url"
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --connect-timeout 30 -o "$dest.part" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "$dest.part" "$url"
    else
        die "curl or wget required to download sources"
    fi
    mv "$dest.part" "$dest"
}

fetch_sources() {
    parse_source_txt
    local name url dest

    for name in "${ORDERED_PKG_NAMES[@]}"; do
        url=${PKG_URL[$name]:-}
        [[ -n "$url" ]] || die "[$name] missing url in source.txt"
        dest=$(pkg_tarball_path "$name")

        if [[ -f "$dest" && $FORCE_FETCH -eq 0 ]]; then
            log "Tarball exists, skip: $(basename "$dest")"
            continue
        fi

        download_file "$url" "$dest"
        log "Saved $(basename "$dest")"
    done
}

extract_sources() {
    parse_source_txt
    local name dest src_dir

    for name in "${ORDERED_PKG_NAMES[@]}"; do
        dest=$(pkg_tarball_path "$name")
        src_dir=$(pkg_source_dir "$name")

        [[ -f "$dest" ]] || die "missing tarball $dest (run --fetch-sources first)"

        if [[ -d "$src_dir" && $FORCE_EXTRACT -eq 0 ]]; then
            log "Source tree exists, skip: $(basename "$src_dir")"
            continue
        fi

        rm -rf "$src_dir"
        log "Extracting $(basename "$dest")"
        tar -xzf "$dest" -C "$PACKAGES_DIR"

        [[ -d "$src_dir" ]] || die "expected directory after extract: $src_dir"
    done
}

build_stamp() {
    local module_id=$1
    echo "$BUILD_ROOT/$module_id/.build_completed"
}

is_built() {
    local module_id=$1
    [[ $FORCE_BUILD -eq 1 ]] && return 1
    [[ -f "$(build_stamp "$module_id")" ]]
}

mark_built() {
    local module_id=$1
    mkdir -p "$BUILD_ROOT/$module_id"
    touch "$(build_stamp "$module_id")"
}

ensure_cross_compiler() {
    command -v "$CC" >/dev/null 2>&1 || die "cross compiler not found: $CC (run . scripts/env.toolchain.sh)"
}

# Autotools: install runtime files only; doc/locale paths point at a stub under build/.
make_install_runtime() {
    local doc_stub="$BUILD_ROOT/.install-docs-stub"
    mkdir -p "$doc_stub"/{man,info,doc,html,locale}
    make install \
        mandir="$doc_stub/man" \
        infodir="$doc_stub/info" \
        docdir="$doc_stub/doc" \
        htmldir="$doc_stub/html" \
        localedir="$doc_stub/locale"
    rm -rf \
        "$DEPS_PREFIX/share/man" \
        "$DEPS_PREFIX/share/doc" \
        "$DEPS_PREFIX/share/info" \
        "$DEPS_PREFIX/share/locale" \
        "$DEPS_PREFIX/share/gettext" 2>/dev/null || true
}

is_strippable_elf() {
    local f=$1
    file -b "$f" 2>/dev/null | grep -qiE 'ELF.*(ARM|EABI)'
}

strip_installed_artifacts() {
    local dir f list stripped=0

    command -v "$STRIP" >/dev/null 2>&1 || die "strip not found: $STRIP"
    command -v file >/dev/null 2>&1 || die "file(1) required for --strip"

    log "Stripping ELFs under $DEPS_PREFIX (skip .a archives)"

    list=$(mktemp)
    for dir in lib bin sbin; do
        [[ -d "$DEPS_PREFIX/$dir" ]] || continue
        find "$DEPS_PREFIX/$dir" -type f ! -name '*.a' 2>/dev/null >> "$list"
    done

    while IFS= read -r f; do
        [[ -z "$f" ]] && continue
        is_strippable_elf "$f" || continue
        "$STRIP" --strip-unneeded "$f"
        stripped=$((stripped + 1))
    done < "$list"
    rm -f "$list"

    log "Stripped $stripped file(s) with $STRIP"
}

maybe_strip_after_build() {
    [[ $STRIP_AFTER_BUILD -eq 1 ]] || return 0
    [[ -d "$DEPS_PREFIX" ]] || return 0
    strip_installed_artifacts
}

is_real_static_archive() {
    local f=$1
    [[ -f "$f" ]] || return 1
    [[ "$(head -c 8 "$f" 2>/dev/null)" == '!<arch>'* ]]
}

is_libtool_linker_script() {
    local f=$1
    [[ -f "$f" ]] || return 1
    is_real_static_archive "$f" && return 1
    grep -qE 'INPUT[[:space:]]*\(|GROUP[[:space:]]*\(' "$f" 2>/dev/null
}

archive_has_members() {
    local f=$1
    is_libtool_linker_script "$f" && return 1
    is_real_static_archive "$f" || return 1
    "$AR" t "$f" 2>/dev/null | grep -q .
}

find_libtool_static_archive() {
    local bdir=$1 libname=$2
    local path

    for path in \
        "$bdir/lib/.libs/$libname" \
        "$bdir/libs/.libs/$libname" \
        "$bdir/.libs/$libname" \
        "$bdir/src/.libs/$libname"; do
        if archive_has_members "$path"; then
            echo "$path"
            return 0
        fi
    done

    while IFS= read -r path; do
        if archive_has_members "$path"; then
            echo "$path"
            return 0
        fi
    done < <(find "$bdir" -path "*/.libs/$libname" -type f 2>/dev/null)

    return 1
}

# libtool may install lib*.a as linker scripts when shared+static; copy the real .a from .libs/.
fix_libtool_installed_archive() {
    local libname=$1 bdir=$2 dest src

    dest="$DEPS_PREFIX/lib/$libname"
    src=$(find_libtool_static_archive "$bdir" "$libname") \
        || die "missing real $libname under $bdir (expected */.libs/$libname)"

    if is_libtool_linker_script "$dest"; then
        log "Replacing libtool linker script: $dest"
    elif archive_has_members "$dest"; then
        return 0
    fi

    install -D -m 644 "$src" "$dest"
    archive_has_members "$dest" || die "invalid static archive after install: $dest"
}

finalize_after_build() {
    [[ -d "$DEPS_PREFIX" ]] || return 0
    maybe_strip_after_build
}

build_libiconv() {
    local name=$1 module_id=$2 src=$3
    local bdir=$BUILD_ROOT/$module_id

    is_built "$module_id" && { log "$module_id already built, skip"; return 0; }

    mkdir -p "$bdir"
    log "Configuring $module_id"
    (
        export CC=${CROSS_COMPILE}gcc
        cd "$bdir"
        "$src/configure" \
            --host="$CROSS_HOST" \
            --prefix="$DEPS_PREFIX" \
            --enable-shared \
            --enable-static \
            --disable-nls
        make -j"$(nproc 2>/dev/null || echo 2)"
        make_install_runtime
    )
    fix_libtool_installed_archive libiconv.a "$bdir"
    mark_built "$module_id"
}

build_openssl() {
    local name=$1 module_id=$2 src=$3
    local bdir=$BUILD_ROOT/$module_id

    is_built "$module_id" && { log "$module_id already built, skip"; return 0; }

    mkdir -p "$bdir"
    log "Configuring $module_id"
    (
        # OpenSSL: host gcc + cross-compile-prefix (do not export CC=gcc globally).
        export CC=gcc
        cd "$bdir"
        "$src/Configure" no-asm shared no-async linux-generic32 \
            --prefix="$DEPS_PREFIX" \
            --cross-compile-prefix="$CROSS_COMPILE"
        # OpenSSL 1.1.x: "shared" adds .so; static .a are built by default (no "no-static").
        if grep -q '\-m64' Makefile 2>/dev/null; then
            sed -i 's/-m64//g' Makefile
        fi
        make -j"$(nproc 2>/dev/null || echo 2)"
        make install_sw
        # install_sw = software only (libs/headers/tools); no man/html docs.
    )
    mark_built "$module_id"
}

build_mosquitto() {
    local name=$1 module_id=$2 src=$3
    local bdir=$BUILD_ROOT/$module_id
    local work=$bdir/src
    local lib_dir=$work/lib

    is_built "$module_id" && { log "$module_id already built, skip"; return 0; }

    rm -rf "$work"
    mkdir -p "$work"
    cp -a "$src/." "$work/"

    # Mosquitto: CROSS_COMPILE + host gcc for lib/ Makefile (do not export globally).
    local -a mq_flags=(
        WITH_TLS=yes
        WITH_SHARED_LIBRARIES=yes
        WITH_STATIC_LIBRARIES=yes
        WITH_DOCS=no
        prefix="$DEPS_PREFIX"
        CROSS_COMPILE=${CROSS_HOST}-
        CC=gcc
        CFLAGS="-I$DEPS_PREFIX/include"
        LDFLAGS="-L$DEPS_PREFIX/lib -Wl,-rpath-link,$DEPS_PREFIX/lib"
    )
    log "Building $module_id (libmosquitto client library)"
    make -C "$lib_dir" clean >/dev/null 2>&1 || true
    make -C "$lib_dir" -j"$(nproc 2>/dev/null || echo 2)" "${mq_flags[@]}"
    make -C "$lib_dir" install "${mq_flags[@]}"
    mark_built "$module_id"
}

build_curl() {
    local name=$1 module_id=$2 src=$3
    local bdir=$BUILD_ROOT/$module_id

    is_built "$module_id" && { log "$module_id already built, skip"; return 0; }

    mkdir -p "$bdir"
    log "Configuring $module_id"
    (
        export CC=${CROSS_COMPILE}gcc
        cd "$bdir"
        "$src/configure" \
            --prefix=${DEPS_PREFIX} \
            --target=${CROSS_HOST} \
            --host=${CROSS_HOST} \
            --build="$(gcc -dumpmachine 2>/dev/null || echo x86_64-linux-gnu)" \
            --without-zlib \
            --with-ssl=${DEPS_PREFIX} \
            --disable-ipv6 --disable-manual \
            --with-pic --enable-static --enable-shared --disable-ldap --disable-ldaps --without-libidn LDFLAGS=-ldl
        make -j"$(nproc 2>/dev/null || echo 2)"
        make_install_runtime
    )
    fix_libtool_installed_archive libcurl.a "$bdir"
    mark_built "$module_id"
}

libmodbus_tarball_is_release() {
    local tarball=$1
    [[ -f "$tarball" ]] || return 1
    tar -tzf "$tarball" 2>/dev/null | grep -qE '^[^/]+/configure$' || return 1
    tar -tzf "$tarball" 2>/dev/null | grep -q 'build-aux/install-sh' || return 1
}

libmodbus_source_ready() {
    local src=$1
    [[ -f "$src/configure" && -f "$src/build-aux/install-sh" ]]
}

ensure_libmodbus_source() {
    local name=$1
    local src tarball url

    parse_source_txt
    src=$(pkg_source_dir "$name")
    tarball=$(pkg_tarball_path "$name")
    libmodbus_source_ready "$src" && return 0

    log "libmodbus source incomplete; checking release tarball"
    if ! libmodbus_tarball_is_release "$tarball"; then
        log "Removing invalid libmodbus tarball (likely GitHub /archive/ snapshot): $(basename "$tarball")"
        rm -f "$tarball"
        url=${PKG_URL[$name]:?}
        [[ "$url" == *"/releases/download/"* ]] || die "packages/source.txt [libmodbus] url must be GitHub releases/download/.../libmodbus-X.Y.Z.tar.gz (got: $url)"
        download_file "$url" "$tarball"
        libmodbus_tarball_is_release "$tarball" || die "downloaded libmodbus tarball still invalid (no configure); check network or source.txt url"
    fi

    rm -rf "$src"
    log "Extracting $(basename "$tarball")"
    tar -xzf "$tarball" -C "$PACKAGES_DIR"
    [[ -d "$src" ]] || die "expected directory after extract: $src"
    libmodbus_source_ready "$src" || die "libmodbus extract failed: $src still missing configure / build-aux/install-sh"
}

build_libmodbus() {
    local name=$1 module_id=$2 src=$3
    local bdir=$BUILD_ROOT/$module_id

    is_built "$module_id" && { log "$module_id already built, skip"; return 0; }

    ensure_libmodbus_source "$name"
    src=$(pkg_source_dir "$name")

    mkdir -p "$bdir"
    log "Configuring $module_id"
    (
        export CC=${CROSS_COMPILE}gcc
        cd "$bdir"
        "$src/configure" \
            --host="$CROSS_HOST" \
            --prefix="$DEPS_PREFIX" \
            --disable-tests \
            --enable-shared \
            --enable-static
        make -j"$(nproc 2>/dev/null || echo 2)"
        make_install_runtime
    )
    fix_libtool_installed_archive libmodbus.a "$bdir"
    mark_built "$module_id"
}

build_one() {
    local name=$1
    local module_id src

    module_id=$(pkg_module_id "$name")
    src=$(pkg_source_dir "$name")

    if [[ "$name" == "libmodbus" ]]; then
        ensure_libmodbus_source "$name"
        src=$(pkg_source_dir "$name")
    elif [[ ! -d "$src" ]]; then
        die "source not extracted: $src"
    fi

    [[ -d "$src" ]] || die "source not extracted: $src"

    case "$name" in
        libiconv) build_libiconv "$name" "$module_id" "$src" ;;
        openssl) build_openssl "$name" "$module_id" "$src" ;;
        mosquitto) build_mosquitto "$name" "$module_id" "$src" ;;
        curl) build_curl "$name" "$module_id" "$src" ;;
        libmodbus) build_libmodbus "$name" "$module_id" "$src" ;;
        *) die "no build recipe for [$name]" ;;
    esac
}

build_all() {
    fetch_sources
    extract_sources
    ensure_cross_compiler
    mkdir -p "$DEPS_PREFIX" "$BUILD_ROOT"

    parse_source_txt
    local name
    for name in "${ORDERED_PKG_NAMES[@]}"; do
        build_one "$name"
    done
    maybe_strip_after_build
    log "All packages built into $DEPS_PREFIX"
}

build_only() {
    local only=$1
    parse_source_txt
    local found=0 name
    for name in "${ORDERED_PKG_NAMES[@]}"; do
        [[ "$name" == "$only" ]] && found=1
    done
    [[ $found -eq 1 ]] || die "unknown package [$only] (see packages/source.txt)"

    fetch_sources
    extract_sources
    ensure_cross_compiler
    mkdir -p "$DEPS_PREFIX" "$BUILD_ROOT"
    build_one "$only"
    finalize_after_build
}

extract_prebuilt() {
    local archive=$PREBUILT_DIR/${CROSS_HOST}.tar.gz
    [[ -f "$archive" ]] || die "missing prebuilt archive: $archive"

    if [[ -d "$DEPS_PREFIX" ]] && [[ -n "$(ls -A "$DEPS_PREFIX" 2>/dev/null)" ]]; then
        [[ ${FORCE:-0} -eq 1 ]] || die "$DEPS_PREFIX exists (use --force to overwrite)"
        rm -rf "$DEPS_PREFIX"
    fi

    log "Extracting $archive -> $DEPS_ROOT"
    tar -xzf "$archive" -C "$DEPS_ROOT"
    [[ -d "$DEPS_PREFIX" ]] || die "expected $DEPS_PREFIX after extract"
}

pack_prebuilt() {
    [[ -d "$DEPS_PREFIX" ]] || die "DEPS_PREFIX not found: $DEPS_PREFIX"
    mkdir -p "$PREBUILT_DIR"
    local archive_name=${CROSS_HOST}.tar.gz
    local archive=$PREBUILT_DIR/$archive_name
    local staging

    staging=$(mktemp -d)
    trap "rm -rf '${staging}'" RETURN

    log "Staging copy $DEPS_PREFIX -> $staging/$CROSS_HOST"
    cp -a "$DEPS_PREFIX" "$staging/$CROSS_HOST"

    if [[ -d "$staging/$CROSS_HOST/bin" ]]; then
        log "Removing bin/ from staging (not shipped in prebuilt)"
        rm -rf "$staging/$CROSS_HOST/bin"
    fi

    log "Packing $staging/$CROSS_HOST -> $archive"
    tar -czf "$archive" -C "$staging" "$CROSS_HOST"

    if command -v sha256sum >/dev/null 2>&1; then
        (cd "$PREBUILT_DIR" && sha256sum "$archive_name") > "$archive.sha256"
        log "Wrote $archive.sha256"
    elif command -v shasum >/dev/null 2>&1; then
        (cd "$PREBUILT_DIR" && shasum -a 256 "$archive_name") > "$archive.sha256"
        log "Wrote $archive.sha256"
    fi

    log "Update MANIFEST.md and CHANGELOG.md"
}

clean_build() {
    rm -rf "$BUILD_ROOT"
    log "Removed $BUILD_ROOT"
}

clean_prefix() {
    [[ ${FORCE:-0} -eq 1 ]] || die "refusing to remove $DEPS_PREFIX without --force"
    rm -rf "$DEPS_PREFIX"
    log "Removed $DEPS_PREFIX"
}

main() {
    local action=""

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --fetch-sources) action=fetch_sources ;;
            --build-all) action=build_all ;;
            --only)
                shift
                [[ $# -gt 0 ]] || die "--only requires a package name"
                action=only
                ONLY_NAME=$1
                ;;
            --extract-prebuilt) action=extract_prebuilt ;;
            --pack-prebuilt) action=pack_prebuilt ;;
            --clean-build) action=clean_build ;;
            --clean-prefix) action=clean_prefix ;;
            --force) FORCE=1 ;;
            --force-fetch) FORCE_FETCH=1 ;;
            --force-extract) FORCE_EXTRACT=1 ;;
            --force-build) FORCE_BUILD=1 ;;
            --strip) STRIP_AFTER_BUILD=1 ;;
            --help|-h) usage; exit 0 ;;
            *) die "unknown option: $1 (try --help)" ;;
        esac
        shift
    done

    [[ -n "$action" ]] || { usage; exit 1; }

    case "$action" in
        fetch_sources) fetch_sources ;;
        build_all) build_all ;;
        only) build_only "$ONLY_NAME" ;;
        extract_prebuilt) extract_prebuilt ;;
        pack_prebuilt) pack_prebuilt ;;
        clean_build) clean_build ;;
        clean_prefix) clean_prefix ;;
    esac
}

main "$@"
