# IE Pro 400 Global Standard — Demo Development Guide

English | [中文](../zh-CN/05-demo-guide.md)

**Doc version**: V1.0　**Date**: 2026-07-22
**Runtime**: Linux on device with **Python 2.7.14** pre-installed. All demos are **C** and build into a single executable `iepro_demo` (Demo V1.0) with an interactive console menu for each submodule (see [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md)).

Runnable source code lives in [`/demo`](../../demo) (English-only). See [`/demo/README.md`](../../demo/README.md).

## 0. Prerequisites

| Item | Details |
|---|---|
| Target system | Linux, ARM Cortex-A7, `arm-linux-gnueabihf` |
| Languages | C/C++ (cross-compiled, used by demos); Python 2.7.14 (pre-installed, optional for customer scripts) |
| Executable | `demo/build/iepro_demo` (unified entry with interactive main menu) |
| Permissions | Access to `/dev/tty*`, `can0`, GPIO, and cellular dial-up typically requires **root** |

### Device Node & GPIO Map

| Logical name | System interface | Notes |
|---|---|---|
| RS485-1 | `/dev/ttymxc1` | Baud rate 600–256000; hardware automatic direction control |
| RS485-2 | `/dev/ttymxc2` | Baud rate 600–256000; hardware automatic direction control |
| RS232-1 | `/dev/ttymxc5` | Baud rate 600–256000 |
| CAN | `can0` | Default 250000 bps; factory-installed CAN module |
| Cellular AT | `/dev/ttyUSB2` | SIM7600G-H-PCIE AT port |
| Cellular data | `wwan0` | NDIS dial-up data interface |
| DI (X1) | GPIO 117 | Passive input (dry contact); short to GND = 1 |
| DO (Y1) | GPIO 118 | Passive output (dry contact) |
| DIP switch 1 | GPIO 124 | ON=1, OFF=0 |
| DIP switch 2 | GPIO 121 | ON=1, OFF=0 |
| Reset button | GPIO 119 | Pressed=1, released=0 |
| POWER LED | — | Hardware power indicator; on when powered (not GPIO-controlled) |
| NET LED | GPIO 122 | On=1 |
| RUN LED | GPIO 71 | On=1 |
| WARN LED | GPIO 123 | On=1 |

Constants are defined in [`demo/src/common/iepro_hw.h`](../../demo/src/common/iepro_hw.h).

## 1. Build and Run

```bash
# Activate cross toolchain (recommended, from repo root)
. scripts/env.toolchain.sh

# Cross-compile
make -C demo

# Include MQTT submodule (requires libs under demo/deps/mosquitto/)
make -C demo WITH_MQTT=1

# Run on device
./iepro_demo
```

Or pass the prefix explicitly: `make -C demo CROSS_COMPILE=arm-linux-gnueabihf-`

### Navigation

- Press **`0`** at any menu to go back one level.
- Press **Ctrl+C** during a loop to stop and return to the current sub-menu.
- Press **Ctrl+C** at a menu prompt (same as `0`).

Main menu after startup:

```
 1) Serial  (RS232 / RS485)
 2) CAN     (SocketCAN)
 3) GPIO    (DI / DO / DIP / LED / Reset button)
 4) MQTT    (northbound publish)
 5) Cellular (SIM7600G-H-PCIE 4G)
 0) Exit
```

### Source Layout

```
demo/
├── Makefile
├── build/iepro_demo          # build output
├── scripts/can_setup.sh      # CAN helper script
└── src/
    ├── main.c                # main menu entry
    ├── demo.h
    ├── common/               # iepro_hw.h, menu_util, gpio_util, metrics
    └── modules/              # feature modules
        ├── serial_mod.c
        ├── can_mod.c
        ├── gpio_mod.c
        ├── cellular_mod.c
        └── mqtt_mod.c
```

| Source file | Submodule |
|---|---|
| [`demo/src/main.c`](../../demo/src/main.c) | Main menu entry |
| [`demo/src/modules/serial_mod.c`](../../demo/src/modules/serial_mod.c) | Serial |
| [`demo/src/modules/can_mod.c`](../../demo/src/modules/can_mod.c) | CAN |
| [`demo/src/modules/gpio_mod.c`](../../demo/src/modules/gpio_mod.c) | GPIO |
| [`demo/src/modules/cellular_mod.c`](../../demo/src/modules/cellular_mod.c) | Cellular (SIM7600G-H-PCIE) |
| [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) | MQTT northbound |
| [`demo/src/common/gpio_util.c`](../../demo/src/common/gpio_util.c) | Shared GPIO helpers |
| [`demo/src/common/metrics.c`](../../demo/src/common/metrics.c) | Sample metrics JSON (used by MQTT) |

## 2. Serial Submodule

Select `1` from the main menu. Choose port `1=RS232-1`, `2=RS485-1`, `3=RS485-2` and baud rate when prompted.

```
 1) Loop receive (Ctrl+C to stop)
 2) Loop send (Ctrl+C to stop)
 3) Loop echo (receive & reply, Ctrl+C to stop)
 0) Back
```

Quick RS485-1 test with `microcom`:

```bash
microcom -s 9600 /dev/ttymxc1
```

### Common Serial Parameters

| Parameter | Values |
|---|---|
| Baud rate | 600 / 9600 / 19200 / 38400 / 57600 / 115200 / 256000 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

> **Note**: 256000 bps uses Linux `termios2` with custom baud (`BOTHER`); standard rates use predefined flags.

Select `2` from the main menu.

```
 1) Bring up can0 (default 250000 bps)
 2) Listen for one frame (3s timeout)
 3) Send test frame (ID 0x123)
 0) Back
```

You can also use the helper script or can-utils outside the submodule:

```bash
sh demo/scripts/can_setup.sh can0 250000
candump can0
cansend can0 123#1122334455667788
```

> **Note**: The CAN module is factory-installed. Configure termination and bitrate to match your bus.

## 4. GPIO Submodule (DI/DO)

Select `3` from the main menu.

```
 1) Init all board GPIO (DI/DO/DIP/Reset)
 2) Monitor DI, DIP & Reset button (Ctrl+C to stop)
 3) Set DO high (Y1)
 4) Set DO low  (Y1)
 5) Run demo pulse on DO (Ctrl+C to stop)
 6) LED test (steady ON / OFF / fast blink)
 0) Back
```

Manual DI test:

```bash
echo 117 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio117/direction
cat /sys/class/gpio/gpio117/value
```

## 5. MQTT Northbound Submodule

Select `4` from the main menu. Uses **libmosquitto** (build with `WITH_MQTT=1`).

Edit `MQTT_BROKER` and `MQTT_DEVICE_ID` in [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) before deployment (currently placeholder values).

```
 1) Publish one sample message
 2) Run publish loop (10s interval, Ctrl+C to stop)
 0) Back
```

The published payload includes DI/DIP readings and sample JSON from `metrics.c`.

### Command-Line Test (mosquitto_pub / mosquitto_sub)

```bash
mosquitto_pub -h <broker_ip> -p 1883 -t "iepro/<device_id>/data" -m '{"temp":25.3}'
mosquitto_sub -h <broker_ip> -p 1883 -t "iepro/<device_id>/cmd"
```

### Suggested Topic Naming

| Purpose | Topic example |
|---|---|
| Data upload | `iepro/<device_id>/data` |
| Command downlink | `iepro/<device_id>/cmd` |
| Status / heartbeat | `iepro/<device_id>/status` |

> Topic structure is a suggestion only — customers may define their own conventions.

## 6. Cellular Submodule

Select `5` from the main menu. Communicates with SIM7600G-H-PCIE on AT port `/dev/ttyUSB2`; NDIS dial-up on `wwan0` via `AT$QCRMCALL`. See [4G Connectivity Example](03-4g-connectivity.md).

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

**Connect (13)** APN profile:

```
 1) Auto (3GPP, no APN)
 2) Custom APN
 0) Cancel
```

Option **17** lists common AT commands; option **18** is a custom AT console (`AT` prefix optional).

## 7. Extension Tips

- Add business logic in `demo/src/modules/*_mod.c`; keep `main.c` as menu router only.
- Reuse [`gpio_util.c`](../../demo/src/common/gpio_util.c) for shared GPIO operations.
- Maintain hardware constants in [`iepro_hw.h`](../../demo/src/common/iepro_hw.h).
- See the [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md) for build and deployment.
