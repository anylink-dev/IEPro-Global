# Cross-compilation toolchain (local install)

English | [中文](README.zh-CN.md)

The **host-side** cross compiler for IE Pro 400 Global Standard is not stored in Git.Download and extract it here; only this README is tracked.

Target libraries used at link time (mosquitto, modbus, curl, openssl) belong under
[`demo/deps/`](../demo/deps/README.md), not in this directory.

## Toolchain version

| Item | Value |
|---|---|
| Package | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` |
| Target | ARM Cortex-A7, `arm-linux-gnueabihf`, glibc |
| Prefix | `arm-linux-gnueabihf-` |
| Firmware | V1.0.0 (see [release notes](../docs/en/10-release-notes.md)) |

Full guide: [Cross-Compilation Toolchain](../docs/en/06-cross-compile-toolchain.md) |
[中文](../docs/zh-CN/06-cross-compile-toolchain.md)

## Install into this repo

### Option A — automated script (recommended)

On a Linux x86_64 host, from the repository root:

```bash
sh scripts/setup_toolchain.sh
. scripts/env.toolchain.sh
```

The script tries community mirrors first, then the Linaro official URL.

### Option B — manual download (mirrors)

The **direct** Linaro `.tar.xz` URL may redirect to a contact page and fail with
`wget`/`curl`. Use one of these mirrors for the same file (~88 MB):

| Source | URL |
|---|---|
| dotsrc (Armbian mirror) | https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |
| Ubuntu RU mirror (Armbian) | https://ru.archive.ubuntu.com/mirrors/armbian/dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |

```bash
mkdir -p toolchain
wget -O /tmp/gcc-linaro.tar.xz \
  https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz
tar -xf /tmp/gcc-linaro.tar.xz -C toolchain/
```

### Option C — Linaro directory page

Browse the [Linaro 5.5-2017.10 arm-linux-gnueabihf index](https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/)
and click `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz` in a browser.
If the direct link still fails, use Option A or B.

Expected layout after extraction:

```
toolchain/
├── README.md          # this file (tracked)
├── README.zh-CN.md    # Chinese guide (tracked)
└── gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/
    ├── bin/arm-linux-gnueabihf-gcc
    └── arm-linux-gnueabihf/
```

Disk space: allow ≥ 2 GB.

## Activate environment

```bash
# from repository root
. scripts/env.toolchain.sh
```

Verify:

```bash
arm-linux-gnueabihf-gcc --version
```

## Build demo

```bash
. scripts/env.toolchain.sh
make -C demo    # prebuilt deps auto-extract on first build (see demo/deps/README.md)
```

Output: `demo/build/iepro_demo`

## Overrides

| Variable | Default | Purpose |
|---|---|---|
| `IEPRO_TOOLCHAIN_ROOT` | `<repo>/toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` | Custom toolchain path |
| `IEPRO_CROSS_COMPILE` | `arm-linux-gnueabihf-` | Compiler prefix |

Example — system-wide install under `/opt`:

```bash
export IEPRO_TOOLCHAIN_ROOT=/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf
. scripts/env.toolchain.sh
```

## Notes

- Do not commit extracted toolchain files or `.tar.xz` archives (see root `.gitignore`).
- For CI or shared build servers, install once under `/opt/` and set `IEPRO_TOOLCHAIN_ROOT`.
- If you already have the tarball from internal distribution, extract it into `toolchain/` manually.
