# IE Pro 400 GlobalStandard — Cross-Compilation Toolchain Guide

English | [中文](../zh-CN/06-cross-compile-toolchain.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## Quick Start (5 minutes)

On **Ubuntu 18.04 / 20.04 / 22.04 (x86_64)**, from installing dependencies to building the demo:

```bash
# 1. Install host dependencies
sudo apt update
sudo apt install -y build-essential git make wget

# 2. Go to the repository root
cd IEPro

# 3. Download and extract the toolchain into toolchain/ (~88 MB, mirrors tried automatically)
sh scripts/setup_toolchain.sh

# 4. Activate the cross-compile environment (run once per new terminal)
. scripts/env.toolchain.sh

# 5. Verify the toolchain
arm-linux-gnueabihf-gcc --version

# 6. Build the demo
make -C demo
```

**Success check**: `demo/build/iepro_demo` exists and `file demo/build/iepro_demo` reports an ARM executable.

> The official Linaro direct URL may fail; the setup script uses community mirrors. See [`toolchain/README.md`](../../toolchain/README.md) ([中文](../../toolchain/README.zh-CN.md)) for mirror URLs and manual install.

## 1. Scope

To protect core low-level implementations, this package **only provides**:

- Cross-compilation toolchain (GCC / SDK)
- Demo example source code (serial/CAN/GPIO/MQTT — see [Demo Development Guide](05-demo-guide.md))
- Required headers and user-space libraries (no kernel driver source)
- Build/packaging/deployment scripts

**Not provided**: kernel source, bootloader source, vendor proprietary driver source, factory image build tools.

For low-level driver customization, contact AnyLink technical support at [anylink.io](https://anylink.io).

## 2. Target Platform

| Item | Spec |
|---|---|
| Target CPU architecture | ARM Cortex-A7 (`arm-linux-gnueabihf`) |
| Target libc | glibc (bundled with Linaro GCC 5.5.0 toolchain) |
| Cross-compile prefix | `arm-linux-gnueabihf-` |
| Target kernel version | Linux (vendor pre-installed standard distribution) |
| Reference SDK version | gcc-linaro-5.5.0-2017.10 |
| Matching firmware version | V1.0.0 (see [Release Notes](09-release-notes.md)) |

## 3. Obtain the Toolchain

| Item | Details |
|---|---|
| Package name | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar` |
| Official index | [Linaro Releases](https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/) (direct `.tar.xz` link may fail — use mirrors below) |
| Recommended mirror | [dotsrc Armbian mirror](https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz) |
| Checksum | No SHA256 provided; download via the online mirrors above or `setup_toolchain.sh` |
| Install path | `toolchain/` in repo, or `/opt/CrossComplie/` |

```bash
# Recommended: use the repo setup script (tries mirrors automatically)
sh scripts/setup_toolchain.sh

# Or download from a mirror manually
mkdir -p toolchain
wget -O /tmp/gcc-linaro.tar.xz \
  https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz
tar -xf /tmp/gcc-linaro.tar.xz -C toolchain/
. scripts/env.toolchain.sh
```

## 4. Environment Setup

### 4.1 Host Requirements

| Item | Recommendation |
|---|---|
| OS | Ubuntu 18.04 / 20.04 / 22.04 (x86_64) |
| Disk space | ≥ 2 GB |
| Dependencies | `build-essential`, `git`, `make`, `wget` |

### 4.2 Environment Variables

**Recommended** (toolchain under `toolchain/` in the repo):

```bash
. scripts/env.toolchain.sh
```

This sets `PATH`, `CROSS_COMPILE`, `CC`, and related variables. Run it once in each new terminal.

**Manual setup** (toolchain installed under `/opt/CrossComplie/`):

```bash
export PATH="$PATH:/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin"
export CROSS_COMPILE=arm-linux-gnueabihf-
export CC=${CROSS_COMPILE}gcc
```

**Persistent setup** (optional — append to `~/.bashrc` or `/etc/profile`):

```bash
export PATH=$PATH:/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin
```

If using the in-repo `toolchain/`, you can add `. scripts/env.toolchain.sh` to `~/.bashrc`.

Verify:

```bash
arm-linux-gnueabihf-gcc --version
```

## 5. Build Demo Examples

Run `. scripts/env.toolchain.sh` first, then:

```bash
make -C demo
make -C demo WITH_MQTT=1    # optional — needs libs under demo/deps/mosquitto/
```

Outputs:

| Binary | Description | Dependencies |
|---|---|---|
| `demo/build/iepro_demo` | Unified demo console (serial/CAN/GPIO/cellular/MQTT submodules) | None by default; MQTT needs `WITH_MQTT=1` + libmosquitto |

> **Note**: The device ships with Python 2.7.14, but all demos are C and build into a single `iepro_demo` binary.

## 6. Deploy to Device

### 6.1 SCP Transfer (Device Online)

```bash
scp demo/build/iepro_demo root@<device-ip>:/tmp/
ssh root@<device-ip> "chmod +x /tmp/iepro_demo && /tmp/iepro_demo"
```

### 6.2 TF Card Transfer

Copy the binary to a TF card, insert into the device, mount, and run. See [Datasheet](01-datasheet.md) §3.6.

## 7. Debugging Tips

| Method | Notes |
|---|---|
| SSH remote debugging | SSH into the device, run the binary, and inspect output (recommended) |
| gdbserver | **Not preinstalled** on the device; cross-compile and deploy gdbserver yourself if needed |
| Log inspection | **No fixed application log path** — define paths in your own application; use standard Linux tools such as `dmesg` and `journalctl` for system logs |
| Static linking | If the target lacks shared libraries, try the `-static` flag |

## 8. Common Build Issues

| Issue | Troubleshooting |
|---|---|
| Cross-compiler not found | Check `PATH` environment variable |
| Link error `cannot find -lxxx` | Confirm the library exists in the toolchain sysroot, or use static linking |
| `not found` at runtime (missing shared lib) | Run `ldd` on the device, or compile with `-static` |
| glibc version mismatch | Confirm toolchain libc matches device firmware (Linaro GCC 5.5.0) |
