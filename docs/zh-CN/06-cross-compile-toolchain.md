# IE Pro 400 GlobalStandard — 交叉编译工具说明

[English](../en/06-cross-compile-toolchain.md) | 中文

**文档版本**：V1.0　**日期**：2026-07-22

## 快速上手（5 分钟）

在 **Ubuntu 18.04 / 20.04 / 22.04（x86_64）** 上，从安装依赖到编出 Demo，按顺序执行：

```bash
# 1. 安装基础依赖
sudo apt update
sudo apt install -y build-essential git make wget

# 2. 进入仓库根目录
cd IEPro

# 3. 下载并解压工具链到 toolchain/（约 88 MB，脚本自动尝试镜像）
sh scripts/setup_toolchain.sh

# 4. 激活交叉编译环境（每个新终端需执行一次）
. scripts/env.toolchain.sh

# 5. 验证工具链
arm-linux-gnueabihf-gcc --version

# 6. 编译 Demo
make -C demo
```

**成功标志**：生成 `demo/build/iepro_demo`，且 `file demo/build/iepro_demo` 显示为 ARM 可执行文件。

> 官方 Linaro 直链可能无法下载，脚本会使用社区镜像。更多镜像地址与手动安装见 [`toolchain/README.zh-CN.md`](../../toolchain/README.zh-CN.md)（[English](../../toolchain/README.md)）。

## 1. 开放范围说明

为保护核心底层实现，本工具包**仅开放**：

- 交叉编译工具链（GCC 工具链 / SDK）
- Demo 示例源码（串口/CAN/GPIO/MQTT，见[《Demo 开发示例》](05-demo-guide.md)）
- 必要的头文件与链接库（用户态接口，不含内核驱动源码）
- 编译/打包/部署脚本

**不开放**：内核源码、Bootloader 源码、厂商私有驱动源码、出厂镜像制作工具。

如需底层驱动定制开发，请联系紫清科技技术支持（[anylink.io](https://anylink.io)）另行申请。

## 2. 目标平台信息

| 项目 | 参数 |
|---|---|
| 目标 CPU 架构 | ARM Cortex-A7（`arm-linux-gnueabihf`） |
| 目标 libc | glibc（Linaro GCC 5.5.0 工具链自带） |
| 交叉编译前缀 | `arm-linux-gnueabihf-` |
| 目标内核版本 | Linux（厂商预装标准发行版） |
| 参考 SDK 版本 | gcc-linaro-5.5.0-2017.10 |
| 对应固件版本 | V1.0.0（见[《版本说明》](09-release-notes.md)） |

## 3. 工具链获取

| 方式 | 说明 |
|---|---|
| 工具链包名 | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar` |
| 官方目录页 | [Linaro Releases](https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/)（直链可能失效，见下方镜像） |
| 推荐镜像 | [dotsrc Armbian mirror](https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz) |
| 校验 | 不提供 SHA256 校验值；请通过上述在线镜像或 `setup_toolchain.sh` 脚本下载 |
| 安装路径示例 | 仓库内 `toolchain/` 或 `/opt/CrossComplie/` |

```bash
# 推荐：使用仓库脚本（自动尝试镜像）
sh scripts/setup_toolchain.sh

# 或手动从镜像下载并解压
mkdir -p toolchain
wget -O /tmp/gcc-linaro.tar.xz \
  https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz
tar -xf /tmp/gcc-linaro.tar.xz -C toolchain/
. scripts/env.toolchain.sh
```

## 4. 环境搭建

### 4.1 Linux 主机要求

| 项目 | 建议 |
|---|---|
| 操作系统 | Ubuntu 18.04 / 20.04 / 22.04（x86_64） |
| 磁盘空间 | ≥ 2 GB |
| 依赖包 | `build-essential`, `git`, `make`, `wget` |

### 4.2 环境变量配置

**推荐**（使用仓库脚本，工具链安装在 `toolchain/` 下）：

```bash
. scripts/env.toolchain.sh
```

脚本会自动设置 `PATH`、`CROSS_COMPILE`、`CC` 等变量。每个新打开的终端都需要执行一次。

**手动设置**（工具链安装在 `/opt/CrossComplie/` 时）：

```bash
export PATH="$PATH:/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin"
export CROSS_COMPILE=arm-linux-gnueabihf-
export CC=${CROSS_COMPILE}gcc
```

**永久设置**（可选，写入 `~/.bashrc` 或 `/etc/profile`）：

```bash
export PATH=$PATH:/opt/CrossComplie/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin
```

若使用仓库内 `toolchain/`，可将 `. scripts/env.toolchain.sh` 写入 `~/.bashrc`。

验证安装：

```bash
arm-linux-gnueabihf-gcc --version
```

## 5. 编译 Demo 示例

先执行 `. scripts/env.toolchain.sh` 激活环境，然后：

```bash
make -C demo
make -C demo WITH_MQTT=1    # 可选，需 demo/deps/mosquitto/ 中的库
```

编译产物：

| 输出文件 | 说明 | 依赖 |
|---|---|---|
| `demo/build/iepro_demo` | 统一 Demo 控制台（含串口/CAN/GPIO/蜂窝/MQTT 子模块） | 默认无；MQTT 需 `WITH_MQTT=1` + libmosquitto |

> **注意**：设备预装 Python 2.7.14，但本仓库 Demo 均为 C 语言，编译为单一可执行文件 `iepro_demo`。

## 6. 部署到设备

### 6.1 通过 SCP 传输（设备已联网）

```bash
scp demo/build/iepro_demo root@<设备IP>:/tmp/
ssh root@<设备IP> "chmod +x /tmp/iepro_demo && /tmp/iepro_demo"
```

### 6.2 通过 TF 卡传输

将编译产物复制到 TF 卡，插入设备后挂载并运行。参见[规格书](01-datasheet.md) §3.6。

## 7. 调试建议

| 方法 | 说明 |
|---|---|
| SSH 远程调试 | 通过 SSH 登录设备，直接运行程序并查看输出（推荐） |
| gdbserver | 设备**未预置** gdbserver；如需 GDB 远程调试，请自行交叉编译并部署 |
| 日志查看 | 设备**无固定应用日志路径**，由客户应用自行定义；系统级日志可用 `dmesg`、`journalctl` 等标准 Linux 命令查看 |
| 静态编译 | 若目标设备缺少动态库，可尝试 `-static` 编译选项 |

## 8. 常见编译问题

| 问题 | 排查方向 |
|---|---|
| 找不到交叉编译器命令 | 检查 `PATH` 环境变量是否正确设置 |
| 链接时报 `cannot find -lxxx` | 确认目标库是否包含在工具链 sysroot 内，或改用静态编译 |
| 目标设备运行时报 `not found`（动态库缺失） | 使用 `ldd` 在设备端检查依赖，或改用 `-static` 编译 |
| glibc 版本不兼容 | 确认工具链 libc 版本与设备固件一致（Linaro GCC 5.5.0） |
