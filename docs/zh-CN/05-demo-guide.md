# IE Pro 400 GlobalStandard — Demo 开发示例

[English](../en/05-demo-guide.md) | 中文

**文档版本**：V1.0　**日期**：2026-07-22
**运行环境**：设备运行 Linux，预装 **Python 2.7.14**；本仓库 Demo 均为 **C 语言**，编译为单一可执行文件 `iepro_demo`（Demo 版本 V1.0），通过控制台菜单进入各子模块（见[《交叉编译工具说明》](06-cross-compile-toolchain.md)）。

可运行源码统一存放在 [`/demo`](../../demo) 目录（纯英文），详见 [`/demo/README.md`](../../demo/README.md)。

## 0. 开发前准备

| 项目 | 说明 |
|---|---|
| 目标系统 | Linux，ARM Cortex-A7，`arm-linux-gnueabihf` |
| 支持语言 | C/C++（交叉编译，Demo 使用）；Python 2.7.14（设备预装，可选用于客户自研脚本） |
| 可执行文件 | `demo/build/iepro_demo`（统一入口，含交互式主菜单） |
| 权限要求 | 访问 `/dev/tty*`、`can0`、GPIO、蜂窝拨号通常需 **root** 权限 |

### 设备节点与 GPIO 对照表

| 逻辑名称 | 系统接口 | 说明 |
|---|---|---|
| RS485-1 | `/dev/ttymxc1` | 波特率 600～256000；RS485 硬件自动方向控制 |
| RS485-2 | `/dev/ttymxc2` | 波特率 600～256000；RS485 硬件自动方向控制 |
| RS232-1 | `/dev/ttymxc5` | 波特率 600～256000 |
| CAN | `can0` | 默认 250000 bps；需硬件安装 CAN 模块 |
| 蜂窝 AT | `/dev/ttyUSB2` | SIM7600 AT 调试口 |
| 蜂窝数据 | `wwan0` | NDIS 拨号数据接口 |
| DI（X1） | GPIO 117 | 无源输入（干接点），与 GND 短接=1 |
| DO（Y1） | GPIO 118 | 无源输出（干接点） |
| 拨码开关 1 | GPIO 124 | ON=1，OFF=0 |
| 拨码开关 2 | GPIO 121 | ON=1，OFF=0 |
| Reset 按键 | GPIO 119 | 按压=1，松开=0 |
| POWER LED | — | 电源指示灯，上电即亮（硬件控制，非 GPIO） |
| NET LED | GPIO 122 | 点亮=1 |
| RUN LED | GPIO 71 | 点亮=1 |
| WARN LED | GPIO 123 | 点亮=1 |

常量定义见 [`demo/src/common/iepro_hw.h`](../../demo/src/common/iepro_hw.h)。

## 1. 编译与运行

```bash
# 激活交叉编译环境（推荐，在仓库根目录执行）
. scripts/env.toolchain.sh

# 交叉编译
make -C demo

# 含 MQTT 子模块（需 demo/deps/mosquitto/ 中的库）
make -C demo WITH_MQTT=1

# 部署到设备后运行
./iepro_demo
```

也可显式指定前缀：`make -C demo CROSS_COMPILE=arm-linux-gnueabihf-`

### 操作说明

- 在任意菜单按 **`0`** 返回上一级。
- 在循环任务中按 **Ctrl+C** 停止循环并回到当前子菜单。
- 在菜单输入提示处按 **Ctrl+C** 与按 `0` 等效。

启动后显示主菜单：

```
 1) Serial  (RS232 / RS485)
 2) CAN     (SocketCAN)
 3) GPIO    (DI / DO / DIP / LED / Reset button)
 4) MQTT    (northbound publish)
 5) Cellular (SIM7600 4G)
 0) Exit
```

### 源码结构

```
demo/
├── Makefile
├── build/iepro_demo          # 编译输出
├── scripts/can_setup.sh      # CAN 辅助脚本
└── src/
    ├── main.c                # 主菜单入口
    ├── demo.h
    ├── common/               # iepro_hw.h, menu_util, gpio_util, metrics
    └── modules/              # 各子模块
        ├── serial_mod.c
        ├── can_mod.c
        ├── gpio_mod.c
        ├── cellular_mod.c
        └── mqtt_mod.c
```

| 源文件 | 子模块 |
|---|---|
| [`demo/src/main.c`](../../demo/src/main.c) | 主菜单入口 |
| [`demo/src/modules/serial_mod.c`](../../demo/src/modules/serial_mod.c) | 串口 |
| [`demo/src/modules/can_mod.c`](../../demo/src/modules/can_mod.c) | CAN |
| [`demo/src/modules/gpio_mod.c`](../../demo/src/modules/gpio_mod.c) | GPIO |
| [`demo/src/modules/cellular_mod.c`](../../demo/src/modules/cellular_mod.c) | 蜂窝（SIM7600） |
| [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) | MQTT 北向 |
| [`demo/src/common/gpio_util.c`](../../demo/src/common/gpio_util.c) | GPIO 共享封装 |
| [`demo/src/common/metrics.c`](../../demo/src/common/metrics.c) | 示例指标 JSON（MQTT 使用） |

## 2. 串口子模块（Serial）

主菜单选 `1` 进入。按提示选择端口 `1=RS232-1`、`2=RS485-1`、`3=RS485-2` 及波特率。

```
 1) Loop receive (Ctrl+C to stop)
 2) Loop send (Ctrl+C to stop)
 3) Loop echo (receive & reply, Ctrl+C to stop)
 0) Back
```

也可使用 `microcom` 快速测试 RS485-1：

```bash
microcom -s 9600 /dev/ttymxc1
```

### 常用串口参数

| 参数 | 取值 |
|---|---|
| 波特率 | 600 / 9600 / 19200 / 38400 / 57600 / 115200 / 256000 |
| 数据位 | 8 |
| 校验位 | 无 |
| 停止位 | 1 |
| 流控 | 无 |

## 3. CAN 子模块（SocketCAN）

主菜单选 `2` 进入。

```
 1) Bring up can0 (default 250000 bps)
 2) Listen for one frame (3s timeout)
 3) Send test frame (ID 0x123)
 0) Back
```

也可在子模块外使用辅助脚本或 can-utils：

```bash
sh demo/scripts/can_setup.sh can0 250000
candump can0
cansend can0 123#1122334455667788
```

> **注意**：使用 CAN 接口需在硬件底板上安装 CAN 模块。

## 4. GPIO 子模块（DI/DO）

主菜单选 `3` 进入。

```
 1) Init all board GPIO (DI/DO/DIP/Reset)
 2) Monitor DI, DIP & Reset button (Ctrl+C to stop)
 3) Set DO high (Y1)
 4) Set DO low  (Y1)
 5) Run demo pulse on DO (Ctrl+C to stop)
 6) LED test (steady ON / OFF / fast blink)
 0) Back
```

手动测试 DI：

```bash
echo 117 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio117/direction
cat /sys/class/gpio/gpio117/value
```

## 5. MQTT 北向子模块

主菜单选 `4` 进入。使用 **libmosquitto**（编译时需 `WITH_MQTT=1`）。

编译前编辑 [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) 中的 `MQTT_BROKER` 和 `MQTT_DEVICE_ID`（当前为示例占位值，部署前需改为实际 Broker 与设备 ID）。

```
 1) Publish one sample message
 2) Run publish loop (10s interval, Ctrl+C to stop)
 0) Back
```

上报 payload 包含 DI/DIP 读数及 `metrics.c` 生成的示例 JSON。

### 命令行测试（mosquitto_pub / mosquitto_sub）

```bash
mosquitto_pub -h <broker_ip> -p 1883 -t "iepro/<device_id>/data" -m '{"temp":25.3}'
mosquitto_sub -h <broker_ip> -p 1883 -t "iepro/<device_id>/cmd"
```

### 主题命名建议

| 用途 | 主题示例 |
|---|---|
| 数据上报 | `iepro/<device_id>/data` |
| 指令下发 | `iepro/<device_id>/cmd` |
| 状态/心跳 | `iepro/<device_id>/status` |

> 以上主题结构仅为建议，客户可根据自身云平台规范自定义。

## 6. 蜂窝子模块（Cellular）

主菜单选 `5` 进入。通过 AT 口 `/dev/ttyUSB2` 与 SIM7600 通信，NDIS 拨号建立 `wwan0` 数据连接。详见[《4G 联网示例》](03-4g-connectivity.md)。

```
 1) Module version (ATI)
 2) Firmware version (AT+CGMR)
 3) IMEI (AT+CGSN)
 4) ICCID (AT+CCID)
 5) IMSI (AT+CIMI)
 6) SIM status (AT+CPIN?)
 7) Signal CSQ (AT+CSQ)
 8) Operator (AT+COPS?)
 9) Network mode (AT+CNSMOD?)
10) Registration status (AT+CEREG? / AT+CGREG?)
11) Dial-up status (AT$QCRMCALL?)
12) Cell info (AT+CPSI?)
13) Connect (NDIS dial-up)
14) Disconnect
15) Renew DHCP on wwan0
16) Ping test (8.8.8.8 via wwan0)
17) AT command help
18) Send custom AT command
 0) Back
```

选项 **13 Connect** 的 APN 配置：

```
 1) Auto (3GPP, no APN)
 2) Custom APN
 0) Cancel
```

选项 **17** 列出常用 AT 指令说明；选项 **18** 可自由输入 AT 命令（`AT` 前缀可选）。

## 7. 扩展开发建议

- 在 `demo/src/modules/*_mod.c` 中添加业务逻辑，保持 `main.c` 仅负责菜单路由。
- 共享 GPIO 操作使用 [`gpio_util.c`](../../demo/src/common/gpio_util.c)。
- 硬件常量统一维护在 [`iepro_hw.h`](../../demo/src/common/iepro_hw.h)。
- 编译与部署详见[《交叉编译工具说明》](06-cross-compile-toolchain.md)。
