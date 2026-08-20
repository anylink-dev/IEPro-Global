# IE Pro 400 Global Standard — Demo 开发示例

[English](../en/05-demo-guide.md) | 中文

**文档版本**：V1.1　**日期**：2026-08-19  
**运行环境**：设备运行 Linux，预装 **Python 2.7.14**；本仓库 Demo 均为 **C 语言**，编译为单一可执行文件 `iepro_demo`，支持交互式菜单及对应的 CLI 子命令（见[《交叉编译工具说明》](06-cross-compile-toolchain.md)）。

可运行源码位于 [`demo/`](../../demo) 目录。  
**菜单、CLI 语法、硬件对照表** 以 [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md) 为准（[English](../../demo/README.md)）。

---

## 0. 开发前准备

| 项目 | 说明 |
|---|---|
| 目标系统 | Linux，ARM Cortex-A7，`arm-linux-gnueabihf` |
| 支持语言 | C/C++（交叉编译，Demo 使用）；Python 2.7.14（设备预装，可选用于客户自研脚本） |
| 可执行文件 | `demo/build/iepro_demo`（统一入口：菜单或 CLI） |
| 权限要求 | 访问 `/dev/tty*`、`can0`、GPIO、`/dev/watchdog`、蜂窝拨号通常需 **root** 权限 |
| 设备节点与 GPIO | 见 [`demo/README.zh-CN.md` — 硬件对照表](../../demo/README.zh-CN.md#硬件对照表) |

---

## 1. 快速开始

```bash
. scripts/env.toolchain.sh
make -C demo
./demo/build/iepro_demo
```

首次构建会自动解压 prebuilt 依赖，详见 [`demo/deps/README.md`](../../demo/deps/README.md)。

无参数运行进入交互菜单；CLI 用法：`./demo/build/iepro_demo --help` 或 `./demo/build/iepro_demo <模块名>`。

---

## 2. 源码结构

```
demo/
├── Makefile
├── build/iepro_demo
├── scripts/can_setup.sh
└── src/
    ├── main.c                # 菜单路由 + CLI 分发
    ├── demo.h
    ├── common/               # iepro_hw.h, menu_util, gpio_util, cli_util, serial_port, metrics
    └── modules/
        ├── serial_mod.c
        ├── can_mod.c
        ├── gpio_mod.c
        ├── cellular_mod.c
        ├── mqtt_mod.c
        ├── http_mod.c
        ├── modbus_mod.c
        └── wdt_mod.c
```

| 源文件 | 子模块 |
|---|---|
| [`demo/src/main.c`](../../demo/src/main.c) | 主菜单入口与 CLI 分发 |
| [`demo/src/common/cli_util.c`](../../demo/src/common/cli_util.c) | CLI 解析与模块路由 |
| [`demo/src/modules/serial_mod.c`](../../demo/src/modules/serial_mod.c) | 串口 |
| [`demo/src/modules/can_mod.c`](../../demo/src/modules/can_mod.c) | CAN |
| [`demo/src/modules/gpio_mod.c`](../../demo/src/modules/gpio_mod.c) | GPIO |
| [`demo/src/modules/cellular_mod.c`](../../demo/src/modules/cellular_mod.c) | 蜂窝（SIM7600G-H-PCIE） |
| [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) | MQTT 北向 |
| [`demo/src/modules/http_mod.c`](../../demo/src/modules/http_mod.c) | HTTP GET/POST（libcurl） |
| [`demo/src/modules/modbus_mod.c`](../../demo/src/modules/modbus_mod.c) | Modbus RTU/TCP |
| [`demo/src/modules/wdt_mod.c`](../../demo/src/modules/wdt_mod.c) | 硬件看门狗（`/dev/watchdog`） |
| [`demo/src/common/gpio_util.c`](../../demo/src/common/gpio_util.c) | GPIO 共享封装 |
| [`demo/src/common/metrics.c`](../../demo/src/common/metrics.c) | 示例 metrics JSON（MQTT 发布默认正文） |

---

## 3. 各模块开发说明

操作菜单与 CLI 详见 [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md)。以下为集成与联调补充说明。

### 3.1 串口（Serial）

使用 `microcom` 快速测试 RS485-1：

```bash
microcom -s 9600 /dev/ttymxc1
```

| 参数 | 取值 |
|---|---|
| 波特率 | 600 / 9600 / 19200 / 38400 / 57600 / 115200 / 256000 |
| 数据位 | 8 |
| 校验位 | 无 |
| 停止位 | 1 |
| 流控 | 无 |

> **说明**：256000 bps 通过 Linux `termios2` 自定义波特率（`BOTHER`）设置；其余为标准波特率常量。

### 3.2 CAN

子模块外可使用辅助脚本或 can-utils：

```bash
sh demo/scripts/can_setup.sh can0 250000
candump can0
cansend can0 123#1122334455667788
```

> **注意**：CAN 模块出厂已安装。请按现场总线配置终端电阻与波特率。

### 3.3 GPIO

通过 sysfs 手动测试 DI：

```bash
echo 117 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio117/direction
cat /sys/class/gpio/gpio117/value
```

### 3.4 MQTT

使用 **libmosquitto**（来自 `demo/deps/arm-linux-gnueabihf/`，默认已链接）。

部署前可编辑 [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) 中的 `MQTT_DEFAULT_BROKER`、`MQTT_DEFAULT_CLIENT_ID` 及主题宏，通过菜单配置，或在 CLI 中通过 `--broker`、`--topic` 传入。发布时消息体留空（菜单直接回车或省略 `--message`）将发送 [`metrics.c`](../../demo/src/common/metrics.c) 生成的示例 JSON。

命令行 Broker 联调（设备或上位机）：

```bash
mosquitto_pub -h <broker_ip> -p 1883 -t "iepro/<device_id>/data" -m '{"temp":25.3}'
mosquitto_sub -h <broker_ip> -p 1883 -t "iepro/<device_id>/cmd"
```

主题命名建议：

| 用途 | 主题示例 |
|---|---|
| 数据上报 | `iepro/<device_id>/data` |
| 指令下发 | `iepro/<device_id>/cmd` |
| 状态/心跳 | `iepro/<device_id>/status` |

> 以上主题结构仅为建议，客户可根据自身云平台规范自定义。

### 3.5 蜂窝（Cellular）

AT 口 `/dev/ttyUSB2`，NDIS 拨号建立 `wwan0`。模组电源由 **GPIO 69**（OUT）控制，启动默认关闭，使用前须上电（Demo 自动处理）。拨号、APN、排障详见[《4G 联网示例》](03-4g-connectivity.md)。

### 3.6 HTTP

使用 **libcurl**（静态链接）。支持 HTTPS；CLI `--ca` 或菜单中设置 CA 路径以校验证书，省略则跳过校验（仅建议实验环境）。

### 3.7 Modbus

使用 **libmodbus**。RTU 与串口子模块共用 `/dev/ttymxc1`–`mxc5`；TCP 使用标准套接字。主站支持周期轮询（`run`）及一次性 `read`/`write`；从站可通过菜单或 CLI 配置保持寄存器。

RTU/TCP 命令行示例见 [`demo/README.zh-CN.md` — CLI 示例](../../demo/README.zh-CN.md#cli-示例)。

### 3.8 硬件看门狗（Watchdog）

设备提供 Linux 标准硬件看门狗 **`/dev/watchdog`**。启用后须由用户进程定期 ioctl 喂狗（`WDIOC_KEEPALIVE`），超时未喂狗则系统自动重启。

Demo 模块 `wdt_mod.c`（由原 `hardwareWDT.py` 移植）提供：

| 操作 | 菜单 / CLI | 说明 |
|---|---|---|
| 启动喂狗 | 菜单 1 / `watchdog start [--timeout N]` | 默认超时 60 s；喂狗间隔约为超时的 1/3 |
| 停止 | 菜单 2 / `watchdog stop` | 向运行中的喂狗进程发 `SIGINT`，magic close `V` 后关闭设备 |
| 触发重启 | 菜单 3 / `watchdog reboot` | 向喂狗进程发 `SIGUSR1`，超时设为 1 s 后停止喂狗 |

运行中的喂狗进程 PID 写入 `/tmp/iepro_wdt.pid`，供 `stop`/`reboot` 跨终端控制。

> **注意**：看门狗一旦启用且未优雅关闭，停止喂狗将导致系统复位。生产环境建议将 `iepro_demo watchdog start` 配置为 systemd/init 自启服务，并确保业务进程异常退出时仍能触发保护性重启。

### 3.9 CLI

`iepro_demo` 支持**菜单模式**（无参数）与 **CLI 模式**（`iepro_demo <模块> <动作> …`），子命令与菜单项一一对应。完整语法与示例见 [`demo/README.zh-CN.md` — CLI](../../demo/README.zh-CN.md#cli)。

---

## 4. 扩展开发建议

- 在 `demo/src/modules/*_mod.c` 中添加业务逻辑，保持 `main.c` 仅负责菜单/CLI 路由。
- 共享 GPIO 操作使用 [`gpio_util.c`](../../demo/src/common/gpio_util.c)。
- 硬件常量统一维护在 [`iepro_hw.h`](../../demo/src/common/iepro_hw.h)。
- 第三方库说明见 [`demo/deps/README.md`](../../demo/deps/README.md)。

---

## 5. 相关文档

| 文档 | 内容 |
|---|---|
| [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md) | 编译、菜单、CLI、硬件对照表 |
| [交叉编译工具说明](06-cross-compile-toolchain.md) | 工具链安装与部署 |
| [4G 联网示例](03-4g-connectivity.md) | 蜂窝拨号与 AT 指令 |
| [`demo/deps/README.md`](../../demo/deps/README.md) | Prebuilt 依赖 |
