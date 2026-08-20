# IE Pro 400 Global Standard — Demo Development Guide

English | [中文](../zh-CN/05-demo-guide.md)

**Doc version**: V1.1　**Date**: 2026-08-19  
**Runtime**: Linux on device with **Python 2.7.14** pre-installed. All demos are **C** and build into a single executable `iepro_demo` with an interactive console menu and matching CLI (see [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md)).

Runnable source lives in [`demo/`](../../demo).  
**Menus, CLI syntax, and hardware map** — canonical reference: [`demo/README.md`](../../demo/README.md).

---

## 0. Prerequisites

| Item | Details |
|---|---|
| Target system | Linux, ARM Cortex-A7, `arm-linux-gnueabihf` |
| Languages | C/C++ (cross-compiled, used by demos); Python 2.7.14 (pre-installed, optional for customer scripts) |
| Executable | `demo/build/iepro_demo` (unified entry: menu or CLI) |
| Permissions | Access to `/dev/tty*`, `can0`, GPIO, `/dev/watchdog`, and cellular dial-up typically requires **root** |
| Device nodes & GPIO | See [Hardware map](../../demo/README.md#hardware-map) in `demo/README.md` |

---

## 1. Quick Start

```bash
. scripts/env.toolchain.sh
make -C demo
./demo/build/iepro_demo
```

Prebuilt dependencies auto-extract on first build — see [`demo/deps/README.md`](../../demo/deps/README.md).

Interactive menu: run with no arguments. CLI: `./demo/build/iepro_demo --help` or `./demo/build/iepro_demo <module>`.

---

## 2. Source Layout

```
demo/
├── Makefile
├── build/iepro_demo
├── scripts/can_setup.sh
└── src/
    ├── main.c                # menu router + CLI dispatch
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

| Source file | Submodule |
|---|---|
| [`demo/src/main.c`](../../demo/src/main.c) | Main menu entry and CLI dispatch |
| [`demo/src/common/cli_util.c`](../../demo/src/common/cli_util.c) | Shared CLI parser and module routing |
| [`demo/src/modules/serial_mod.c`](../../demo/src/modules/serial_mod.c) | Serial |
| [`demo/src/modules/can_mod.c`](../../demo/src/modules/can_mod.c) | CAN |
| [`demo/src/modules/gpio_mod.c`](../../demo/src/modules/gpio_mod.c) | GPIO |
| [`demo/src/modules/cellular_mod.c`](../../demo/src/modules/cellular_mod.c) | Cellular (SIM7600G-H-PCIE) |
| [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) | MQTT northbound |
| [`demo/src/modules/http_mod.c`](../../demo/src/modules/http_mod.c) | HTTP GET/POST (libcurl) |
| [`demo/src/modules/modbus_mod.c`](../../demo/src/modules/modbus_mod.c) | Modbus RTU/TCP |
| [`demo/src/modules/wdt_mod.c`](../../demo/src/modules/wdt_mod.c) | Hardware watchdog (`/dev/watchdog`) |
| [`demo/src/common/gpio_util.c`](../../demo/src/common/gpio_util.c) | Shared GPIO helpers |
| [`demo/src/common/metrics.c`](../../demo/src/common/metrics.c) | Sample metrics JSON (default MQTT publish body) |

---

## 3. Module Development Notes

Operational menus and CLI are documented in [`demo/README.md`](../../demo/README.md). Below are integration and testing notes for developers.

### 3.1 Serial

Quick RS485-1 test with `microcom`:

```bash
microcom -s 9600 /dev/ttymxc1
```

| Parameter | Values |
|---|---|
| Baud rate | 600 / 9600 / 19200 / 38400 / 57600 / 115200 / 256000 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

> **Note**: 256000 bps uses Linux `termios2` with custom baud (`BOTHER`); standard rates use predefined flags.

### 3.2 CAN

Outside the demo submodule, use the helper script or can-utils:

```bash
sh demo/scripts/can_setup.sh can0 250000
candump can0
cansend can0 123#1122334455667788
```

> **Note**: The CAN module is factory-installed. Configure termination and bitrate to match your bus.

### 3.3 GPIO

Manual DI test via sysfs:

```bash
echo 117 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio117/direction
cat /sys/class/gpio/gpio117/value
```

### 3.4 MQTT

Uses **libmosquitto** from `demo/deps/arm-linux-gnueabihf/` (linked by default).

Edit `MQTT_DEFAULT_BROKER`, `MQTT_DEFAULT_CLIENT_ID`, and topic macros in [`demo/src/modules/mqtt_mod.c`](../../demo/src/modules/mqtt_mod.c) before deployment, configure via the interactive menu, or pass `--broker` / `--topic` on the CLI. An empty publish body (menu Enter or omitted `--message`) sends sample metrics JSON from [`metrics.c`](../../demo/src/common/metrics.c).

Command-line broker test (device or host):

```bash
mosquitto_pub -h <broker_ip> -p 1883 -t "iepro/<device_id>/data" -m '{"temp":25.3}'
mosquitto_sub -h <broker_ip> -p 1883 -t "iepro/<device_id>/cmd"
```

Suggested topic naming:

| Purpose | Topic example |
|---|---|
| Data upload | `iepro/<device_id>/data` |
| Command downlink | `iepro/<device_id>/cmd` |
| Status / heartbeat | `iepro/<device_id>/status` |

> Topic structure is a suggestion only — customers may define their own conventions.

### 3.5 Cellular

SIM7600G-H-PCIE on AT port `/dev/ttyUSB2`; NDIS dial-up on `wwan0` via `AT$QCRMCALL`.
Module power is controlled by **GPIO 69** (OUT); off by default at boot — enable before use (handled automatically by the demo).
See [4G Connectivity Example](03-4g-connectivity.md) for dial-up, APN, and troubleshooting.

### 3.6 HTTP

Uses **libcurl** (statically linked). Supports HTTPS; pass `--ca` on the CLI or set a CA path in the menu for TLS verification. Omit CA to skip verify (lab only).

### 3.7 Modbus

Uses **libmodbus**. RTU shares the same serial ports as the Serial submodule (`/dev/ttymxc1`–`mxc5`); TCP uses standard sockets. Master supports periodic poll (`run`) and one-shot `read`/`write`; slave exposes holding registers configurable via menu or CLI.

See [`demo/README.md` — CLI examples](../../demo/README.md#cli-examples) for RTU/TCP command lines.

### 3.8 Hardware Watchdog

The device exposes the standard Linux hardware watchdog at **`/dev/watchdog`**. Once enabled, a user process must periodically feed it (`WDIOC_KEEPALIVE`); the system reboots if the timer expires.

Demo module `wdt_mod.c` (ported from `hardwareWDT.py`) provides:

| Action | Menu / CLI | Notes |
|---|---|---|
| Start feeding | Menu 1 / `watchdog start [--timeout N]` | Default timeout 60 s; feed interval ≈ timeout / 3 |
| Stop | Menu 2 / `watchdog stop` | Sends `SIGINT` to the feeder; magic close `V` before closing the device |
| Trigger reboot | Menu 3 / `watchdog reboot` | Sends `SIGUSR1`; sets timeout to 1 s and stops feeding |

The running feeder PID is stored in `/tmp/iepro_wdt.pid` for cross-terminal `stop`/`reboot`.

> **Warning**: Once enabled without a graceful stop, stopping feeds will reset the system. For production, run `iepro_demo watchdog start` as a boot service.

### 3.9 CLI

`iepro_demo` runs in **menu mode** (no arguments) or **CLI mode** (`iepro_demo <module> <action> …`). Actions mirror the interactive menus. Full syntax and examples: [`demo/README.md` — CLI](../../demo/README.md#cli).

---

## 4. Extension Tips

- Add business logic in `demo/src/modules/*_mod.c`; keep `main.c` as menu/CLI router only.
- Reuse [`gpio_util.c`](../../demo/src/common/gpio_util.c) for shared GPIO operations.
- Maintain hardware constants in [`iepro_hw.h`](../../demo/src/common/iepro_hw.h).
- Third-party libs: [`demo/deps/README.md`](../../demo/deps/README.md).

---

## 5. Related Documentation

| Document | Content |
|---|---|
| [`demo/README.md`](../../demo/README.md) | Build, menus, CLI, hardware map ([中文](../../demo/README.zh-CN.md)) |
| [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md) | Toolchain setup and deployment |
| [4G Connectivity Example](03-4g-connectivity.md) | Cellular dial-up and AT commands |
| [`demo/deps/README.md`](../../demo/deps/README.md) | Prebuilt dependencies |
