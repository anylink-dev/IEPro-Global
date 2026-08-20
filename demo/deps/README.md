# demo/deps — Third-Party Dependencies

Cross-compiled libraries for the IE Pro 400 Global Standard Demo (`arm-linux-gnueabihf`).

**Version**: 0.2 — see [Versioning](#versioning) below.

---

## Overview

- Single install prefix per GNU triplet (`deps/arm-linux-gnueabihf/`);
- **Prebuilt snapshot in Git** — extract and build; no local rebuild required;
- **Sources on demand** — `packages/*.tar.gz` stay out of Git; pins in [`packages/source.txt`](packages/source.txt);
- **Makefile** — links all prebuilt libraries by default; static link from `DEPS_PREFIX`.

| In scope | Out of scope |
|----------|--------------|
| `demo/deps/` layout and scripts | Device OS package managers |
| `prebuilt/` snapshot, `MANIFEST.md`, `CHANGELOG.md` | Firmware changes |
| `demo/Makefile` `-I/-L` rules | New Demo application modules |

---

## Directory Layout

```
deps/
├── README.md
├── MANIFEST.md                  # current prebuilt: versions, SHA256, artifacts
├── CHANGELOG.md                 # prebuilt release history
├── buildDepends.sh
├── packages/source.txt
├── build/                       # not in Git
├── arm-linux-gnueabihf/         # DEPS_PREFIX (not in Git)
└── prebuilt/
    ├── arm-linux-gnueabihf.tar.gz
    └── arm-linux-gnueabihf.tar.gz.sha256
```

**Git**: tracked — `README.md`, `MANIFEST.md`, `CHANGELOG.md`, `buildDepends.sh`, `packages/source.txt`, `prebuilt/*.{tar.gz,sha256}`.  
Not tracked — `build/`, `arm-linux-gnueabihf/`, `packages/*.tar.gz`, `packages/*/`.

Build order (see `source.txt`): `libiconv` → `openssl` → `mosquitto` → `curl` → `libmodbus`.

---

## Quick Start (prebuilt)

```bash
. scripts/env.toolchain.sh
make -C demo
```

On the first build, `demo/Makefile` runs `deps-prebuilt` and extracts
`prebuilt/arm-linux-gnueabihf.tar.gz` when `deps/arm-linux-gnueabihf/` is missing.

Optional manual extract (same result):

```bash
. scripts/env.toolchain.sh
cd demo/deps
./buildDepends.sh --extract-prebuilt
cd ../..
make -C demo
```

`IEPRO_DEPS_PREFIX` → `demo/deps/arm-linux-gnueabihf`.  
If `DEPS_PREFIX` already exists, extraction is skipped unless `--force` is passed.

Verify tarball (optional): `cd demo/deps/prebuilt && sha256sum -c arm-linux-gnueabihf.tar.gz.sha256`

---

## Build from Source (maintainers)

Requirements: Linux, `bash`, `. scripts/env.toolchain.sh`, `make`, `curl` or `wget`.

```bash
. scripts/env.toolchain.sh
cd demo/deps
./buildDepends.sh --fetch-sources
./buildDepends.sh --force-build --build-all --strip
./buildDepends.sh --pack-prebuilt
# Update MANIFEST.md and CHANGELOG.md, then commit
```

| Command | Description |
|---------|-------------|
| `--fetch-sources` | Download missing tarballs from `source.txt` |
| `--build-all` / `--only <name>` | Cross-compile and install |
| `--extract-prebuilt` / `--pack-prebuilt` | Extract or pack `prebuilt/${CROSS_HOST}.tar.gz` (`pack-prebuilt` omits `bin/`) |
| `--strip` | Strip ELFs under `DEPS_PREFIX` (skip `.a`) |
| `--force-fetch` / `--force-extract` / `--force-build` | Force redo |
| `--clean-build` / `--clean-prefix --force` | Remove `build/` or `DEPS_PREFIX` |

Incremental builds use `build/<module>/.build_completed` (skip unless `--force-build`). Run `./buildDepends.sh --help` for paths and env vars.

---

## Makefile Integration

`demo/Makefile` defines a `deps-prebuilt` target (auto-run before linking) that
extracts `prebuilt/$(CROSS_HOST).tar.gz` when `$(DEPS_PREFIX)/lib` is missing.

```makefile
DEPS_PREFIX ?= deps/$(CROSS_HOST)    # arm-linux-gnueabihf
```

Static link from `$(DEPS_PREFIX)/lib`; system libs stay dynamic (`-Wl,-Bstatic` … `-Wl,-Bdynamic`).

All prebuilt libraries are linked by default:

| Libraries | Notes |
|-----------|-------|
| `-lmosquitto -lmodbus -lcurl -lssl -lcrypto` | static, from `DEPS_PREFIX` |
| `-lpthread -ldl` | dynamic, system |

Compile defines: `-DWITH_MQTT -DWITH_MODBUS -DWITH_CURL -DWITH_SSL`.

---

## Versioning

**What `0.2` means** — version of the **demo/deps framework** (directory layout, `buildDepends.sh` behaviour, prebuilt packaging). It is **not** OpenSSL/curl/etc. versions; those live in [`packages/source.txt`](packages/source.txt) and [`MANIFEST.md`](MANIFEST.md).

**Where it is defined** — this line in `README.md` is the only canonical spec version. [`CHANGELOG.md`](CHANGELOG.md) records each release; [`MANIFEST.md`](MANIFEST.md) describes the current prebuilt tarball only.

**When to bump**

| Change | Bump | Update |
|--------|------|--------|
| Doc/script fix, no new prebuilt | optional patch (e.g. 0.1 → 0.1.1) or skip | README if bumped; CHANGELOG note |
| Rebuild prebuilt (lib bump, build fix) | minor (0.1 → 0.2) | README version, CHANGELOG entry, MANIFEST, `prebuilt/*`, commit |
| Breaking layout/API change | major (0.2 → 1.0) | same as minor + migrate docs/Makefile |

After a prebuilt rebuild: `./buildDepends.sh --pack-prebuilt` → sync [`MANIFEST.md`](MANIFEST.md) (SHA256, library table) → add [`CHANGELOG.md`](CHANGELOG.md) entry → bump version here if needed.

---

## Related

| File | Content |
|------|---------|
| [`MANIFEST.md`](MANIFEST.md) | Current prebuilt versions, checksum, install artifacts |
| [`CHANGELOG.md`](CHANGELOG.md) | Prebuilt release notes |
| [`packages/source.txt`](packages/source.txt) | Upstream version pins and download URLs |
| [`demo/README.md`](../README.md) | Demo build ([中文](README.zh-CN.md)) |
| [`scripts/env.toolchain.sh`](../../scripts/env.toolchain.sh) | Toolchain and `IEPRO_DEPS_PREFIX` |
