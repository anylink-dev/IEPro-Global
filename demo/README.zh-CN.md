# IE Pro 400 Global Standard — Demo 控制台

[English](README.md) | 中文

所有 Demo 均为 **纯 C**，编译为单一可执行文件 `build/iepro_demo`。

**本文**为操作说明（编译、菜单、CLI、硬件对照表）。  
集成与扩展开发见 [`docs/zh-CN/05-demo-guide.md`](../docs/zh-CN/05-demo-guide.md)（[English](../docs/en/05-demo-guide.md)）。

> **说明**：设备预装 **Python 2.7.14**。本 Demo 套件为纯 C，不使用 Python。

## 目录结构

```
demo/
├── Makefile              # 构建规则；首次构建自动解压 prebuilt 依赖
├── README.md             # 英文操作说明
├── README.zh-CN.md       # 本文件
├── build/                # 输出：iepro_demo（gitignore）
├── scripts/
│   └── can_setup.sh      # 启动 can0（可选辅助脚本）
├── src/
│   ├── main.c            # 入口：交互菜单或 CLI 分发
│   ├── demo.h
│   ├── common/
│   │   ├── iepro_hw.h
│   │   ├── menu_util.c/h
│   │   ├── gpio_util.c/h
│   │   ├── cli_util.c/h
│   │   ├── serial_port.c/h
│   │   └── metrics.c/h   # 示例 metrics JSON（MQTT 发布默认正文）
│   └── modules/
│       ├── serial_mod.c
│       ├── can_mod.c
│       ├── gpio_mod.c
│       ├── cellular_mod.c
│       ├── mqtt_mod.c
│       ├── http_mod.c
│       ├── modbus_mod.c
│       └── wdt_mod.c
└── deps/                 # 第三方库（见 deps/README.md）
    ├── buildDepends.sh
    ├── packages/source.txt
    ├── arm-linux-gnueabihf/    # 解压后的 DEPS_PREFIX（gitignore）
    └── prebuilt/
        └── arm-linux-gnueabihf.tar.gz
```

## 编译

在仓库根目录激活交叉编译环境并构建：

```bash
. scripts/env.toolchain.sh
make -C demo
```

首次构建时，若 `deps/arm-linux-gnueabihf/` 不存在，Makefile 会自动解压
`deps/prebuilt/arm-linux-gnueabihf.tar.gz`。

也可显式指定交叉前缀：

```bash
cd demo
make CROSS_COMPILE=arm-linux-gnueabihf-
```

输出：`build/iepro_demo`

第三方库（MQTT、Modbus、curl、OpenSSL）从 `deps/arm-linux-gnueabihf/` **静态链接**；
设备上无需设置 `LD_LIBRARY_PATH`。
手动解压 prebuilt 或从源码重建依赖，见 [`deps/README.md`](deps/README.md)。

## 运行

```bash
./build/iepro_demo              # 交互菜单
./build/iepro_demo --help       # CLI 总览
./build/iepro_demo <module>     # 某模块的 CLI 帮助
```

多数操作需要 **root**（GPIO、CAN、串口、蜂窝拨号、硬件看门狗 `/dev/watchdog`）。

### 操作说明（交互菜单）

- 在任意菜单按 **`0`** 返回上一级。
- 在阻塞循环中（接收、闪烁、发布等）按 **Ctrl+C** 停止循环并回到当前子菜单。
- 在菜单输入提示处按 **Ctrl+C** 与按 `0` 等效。

### 主菜单

（菜单项文字与程序输出一致，保留英文。）

```
 1) Serial  (RS232 / RS485)
 2) CAN     (SocketCAN)
 3) GPIO    (DI / DO / DIP / LED / Reset button)
 4) MQTT    (northbound publish)
 5) Cellular (SIM7600G-H-PCIE 4G)
 6) HTTP    (GET / POST test)
 7) Modbus  (RTU / TCP, master / slave)
 8) Watchdog (hardware /dev/watchdog)
 0) Exit
```

### 串口 Serial（`1`）

按提示选择端口 `1=RS232-1`、`2=RS485-1`、`3=RS485-2` 及波特率。

```
 1) Loop receive (Ctrl+C to stop)
 2) Loop send (Ctrl+C to stop)
 3) Loop echo (receive & reply, Ctrl+C to stop)
 0) Back
```

### CAN（`2`）

```
 1) Bring up can0 (default 250000 bps)
 2) Listen for one frame (3s timeout)
 3) Send test frame (ID 0x123)
 0) Back
```

### GPIO（`3`）

```
 1) Init all board GPIO (DI/DO/DIP/Reset)
 2) Monitor DI, DIP & Reset button (Ctrl+C to stop)
 3) Set DO high (Y1)
 4) Set DO low  (Y1)
 5) Run demo pulse on DO (Ctrl+C to stop)
 6) LED test (steady ON / OFF / fast blink)
 0) Back
```

### MQTT（`4`）

通过菜单（选项 1）、CLI `--broker` / `--topic`，或修改 `src/modules/mqtt_mod.c` 中的编译期默认值（`MQTT_DEFAULT_BROKER`、`MQTT_DEFAULT_CLIENT_ID` 及主题宏）。

```
 1) Configure MQTT connection parameters
 2) Connect (broker session + background loop)
 3) Disconnect
 4) Show current status
 5) Publish message
 0) Back
```

发布（菜单 5 或 CLI `publish`）：可输入自定义消息；直接回车或省略 `--message` 时发送 `metrics.c` 生成的示例 JSON（含 DI/DIP 读数）。

### 蜂窝 Cellular（`5`）

AT 口 `/dev/ttyUSB2`（SIM7600G-H-PCIE）；NDIS 拨号建立 `wwan0`（`AT$QCRMCALL`）。
**4G 模组电源**由 GPIO 69（OUT）控制，启动默认关闭；Demo 首次访问 AT 口时自动上电。
详见[《4G 联网示例》](../docs/zh-CN/03-4g-connectivity.md)。

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

**Connect (13)** APN 配置：

```
 1) Auto (3GPP, no APN)
 2) Custom APN
 0) Cancel
```

### HTTP（`6`）

使用 **libcurl**（支持 HTTPS；可选 `--ca` 进行 TLS 证书校验）。

```
 1) Set URL
 2) Set CA certificate path
 3) Show current configuration
 4) GET
 5) POST
 0) Back
```

默认 URL：`https://example.com`

### Modbus（`7`）

RTU（串口）或 TCP；主站（轮询/读/写）或从站（服务端）。

```
 1) Configure connection parameters
 2) Show current configuration
 3) Start worker (master poll / slave server)
 4) Stop worker
 5) Master: one-shot read
 6) Master: write holding register
 7) Slave: set holding register value
 0) Back
```

### 看门狗 Watchdog（`8`）

硬件看门狗设备节点 **`/dev/watchdog`**。打开并启用后，须由用户进程定期喂狗；超时未喂狗则系统自动重启。

- **默认超时**：60 秒（可配置）
- **喂狗间隔**：约为超时时间的 1/3
- **优雅停止**：向运行中的喂狗进程发 `SIGINT`（或 Ctrl+C），写入 magic close `V` 后关闭设备
- **触发重启**：向喂狗进程发 `SIGUSR1`，将超时设为 1 秒后停止喂狗，等待硬件复位

```
 1) Start keepalive (foreground)
 2) Stop running feeder
 3) Reboot via watchdog timeout
 0) Back
```

> 原设备侧 Python 脚本 `hardwareWDT.py` 已移植为本模块（`wdt_mod.c`）。生产环境建议将 `watchdog start` 配置为开机自启服务。

## CLI

CLI 子命令与交互菜单一一对应。循环类动作持续运行直至 **Ctrl+C**。

```text
Usage: iepro_demo [module action [options]]
       iepro_demo                         (interactive menu)

Modules:
  serial   recv|send|echo       --port 1|2|3 --baud N [--text STR]
  can      up|listen|send       [--bitrate N]
  gpio     init|monitor|do-high|do-low|pulse|led
           led: on|off|blink [--led 1|2|3|4]
  mqtt     connect|publish
  cellular version|firmware|imei|iccid|imsi|sim|csq|operator|
           netmode|reg|dial-status|cell|connect|disconnect|
           dhcp|ping|help|at [--cmd STR] [--apn A] [--user U] [--pass P]
  http     get|post [--url U] [--ca PATH] [--body STR]
  modbus   run|read|write
  watchdog start|stop|reboot [--timeout N]
```

运行 `iepro_demo <module>`（不带 action）可查看该模块的选项与示例。

### CLI 示例

```bash
# 串口 — RS485-1 @ 9600 回显
./iepro_demo serial echo --port 2 --baud 9600

# CAN — 启动 can0 并发送测试帧
./iepro_demo can up --bitrate 250000
./iepro_demo can send

# GPIO — 监视 DI/DIP 直至 Ctrl+C
./iepro_demo gpio monitor

# MQTT — 单次发布
./iepro_demo mqtt publish --broker 192.168.1.10 --port 1883 \
  --topic iepro/demo/data --message '{"temp":25}'
# 省略 --message 则发布 metrics.c 默认 JSON（含 DI/DIP）
./iepro_demo mqtt publish --broker 192.168.1.10 --topic iepro/demo/data

# 蜂窝 — 读取 IMEI 后拨号
./iepro_demo cellular imei
./iepro_demo cellular connect --apn internet

# HTTP — GET / POST
./iepro_demo http get --url https://example.com
./iepro_demo http post --url https://httpbin.org/post \
  --body '{"message":"hello from IEPro demo"}'

# Modbus — RS485-1 上 RTU 主站轮询
./iepro_demo modbus run --link rtu --role master --port 2 --baud 9600 \
  --unit-id 1 --start-addr 0 --count 10 --function 3 --poll-interval 5

# Modbus — TCP 从站监听 502 端口
./iepro_demo modbus run --link tcp --role slave --tcp-port 502 \
  --unit-id 1 --holding-regs 64

# 看门狗 — 前台喂狗（另开终端 stop / reboot）
./iepro_demo watchdog start --timeout 60
./iepro_demo watchdog stop
./iepro_demo watchdog reboot
```

## 硬件对照表

| 接口 | 设备节点 / GPIO |
|---|---|
| RS232-1 | `/dev/ttymxc5` |
| RS485-1 | `/dev/ttymxc1` |
| RS485-2 | `/dev/ttymxc2` |
| CAN | `can0`（默认 250000 bps） |
| 蜂窝 AT | `/dev/ttyUSB2` |
| 蜂窝数据 | `wwan0` |
| 4G 模组电源 | GPIO 69（OUT；1=开，0=关；启动默认关） |
| DI（X1） | GPIO 117 |
| DO（Y1） | GPIO 118 |
| 拨码开关 1 | GPIO 124 |
| 拨码开关 2 | GPIO 121 |
| Reset 按键 | GPIO 119 |
| POWER LED | —（硬件电源指示，上电即亮） |
| NET LED | GPIO 122 |
| RUN LED | GPIO 71 |
| WARN LED | GPIO 123 |
| 硬件看门狗 | `/dev/watchdog`（PID 文件 `/tmp/iepro_wdt.pid`） |

常量定义见 `src/common/iepro_hw.h`。

## CAN 辅助脚本

```bash
sh scripts/can_setup.sh          # can0 @ 250000 bps（默认）
sh scripts/can_setup.sh can0 500000
```
