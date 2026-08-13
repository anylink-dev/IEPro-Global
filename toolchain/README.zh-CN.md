# 交叉编译工具链（本地安装）

[English](README.md) | 中文

IE Pro 400 Global Standard 的**主机端**交叉编译器不纳入 Git 版本管理。
请下载并解压到本目录；仓库中仅保留本说明文件。

链接阶段使用的**目标端**第三方库（mosquitto、modbus、curl、openssl）请放在
[`demo/deps/`](../demo/deps/README.md)，不要放在本目录。

## 工具链版本

| 项目 | 参数 |
|---|---|
| 工具链包 | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` |
| 目标平台 | ARM Cortex-A7，`arm-linux-gnueabihf`，glibc |
| 编译前缀 | `arm-linux-gnueabihf-` |
| 对应固件 | V1.0.0（见[版本说明](../docs/zh-CN/09-release-notes.md)） |

完整文档：[交叉编译工具说明](../docs/zh-CN/06-cross-compile-toolchain.md) |
[English](../docs/en/06-cross-compile-toolchain.md)

## 安装到本仓库

### 方式 A — 自动脚本（推荐）

在 Linux x86_64 主机上，于仓库根目录执行：

```bash
sh scripts/setup_toolchain.sh
. scripts/env.toolchain.sh
```

脚本会优先尝试社区镜像，最后才尝试 Linaro 官方地址。

### 方式 B — 手动下载（镜像）

Linaro **直链** `.tar.xz` 可能跳转到联系页，导致 `wget`/`curl` 失败。
请从以下镜像下载同一文件（约 88 MB）：

| 来源 | 地址 |
|---|---|
| dotsrc（Armbian 镜像） | https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |
| Ubuntu RU 镜像（Armbian） | https://ru.archive.ubuntu.com/mirrors/armbian/dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |

```bash
mkdir -p toolchain
wget -O /tmp/gcc-linaro.tar.xz \
  https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz
tar -xf /tmp/gcc-linaro.tar.xz -C toolchain/
```

### 方式 C — Linaro 目录页

在浏览器中打开 [Linaro 5.5-2017.10 arm-linux-gnueabihf 目录](https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/)，
点击 `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz` 下载。
若直链仍失败，请使用方式 A 或 B。

解压后的目录结构：

```
toolchain/
├── README.md          # 英文说明（纳入 Git）
├── README.zh-CN.md    # 本文件（纳入 Git）
└── gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/
    ├── bin/arm-linux-gnueabihf-gcc
    └── arm-linux-gnueabihf/
```

磁盘空间：预留 ≥ 2 GB。

## 激活环境

```bash
# 在仓库根目录
. scripts/env.toolchain.sh
```

验证：

```bash
arm-linux-gnueabihf-gcc --version
```

## 编译 Demo

```bash
. scripts/env.toolchain.sh
make -C demo
make -C demo WITH_MQTT=1    # 需 demo/deps/mosquitto/ 中的库
```

输出：`demo/build/iepro_demo`

## 自定义路径

| 变量 | 默认值 | 说明 |
|---|---|---|
| `IEPRO_TOOLCHAIN_ROOT` | `<repo>/toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` | 工具链根目录 |
| `IEPRO_CROSS_COMPILE` | `arm-linux-gnueabihf-` | 编译器前缀 |

示例 — 系统级安装于 `/opt`：

```bash
export IEPRO_TOOLCHAIN_ROOT=/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf
. scripts/env.toolchain.sh
```

## 说明

- 不要将解压后的工具链或 `.tar.xz` 压缩包提交到 Git（见根目录 `.gitignore`）。
- CI 或共享编译服务器可安装到 `/opt/`，并设置 `IEPRO_TOOLCHAIN_ROOT`。
- 若已有内部发放的压缩包，手动解压到 `toolchain/` 即可。
