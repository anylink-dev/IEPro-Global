# IE Pro 400 Global Standard — Demo Console

English | [中文](README.zh-CN.md)

All demos are **C-only** and build into a **single executable** `build/iepro_demo`.

**This file** is the operational reference (build, menus, CLI, hardware map).  
For integration notes and extension tips, see [`docs/en/05-demo-guide.md`](../docs/en/05-demo-guide.md) or [`docs/zh-CN/05-demo-guide.md`](../docs/zh-CN/05-demo-guide.md).

> **Note**: The device ships with **Python 2.7.14**. This demo suite is C-only.

## Directory layout

```
demo/
├── Makefile              # build rules; auto-extracts prebuilt deps on first build
├── README.md             # English operational guide
├── README.zh-CN.md       # Chinese operational guide
├── build/                # output: iepro_demo (gitignored)
├── scripts/
│   └── can_setup.sh      # bring up can0 (optional helper)
├── src/
│   ├── main.c            # entry point: interactive menu or CLI dispatch
│   ├── demo.h
│   ├── common/
│   │   ├── iepro_hw.h
│   │   ├── menu_util.c/h
│   │   ├── gpio_util.c/h
│   │   ├── cli_util.c/h
│   │   ├── serial_port.c/h
│   │   └── metrics.c/h   # sample metrics JSON (default MQTT publish body)
│   └── modules/
│       ├── serial_mod.c
│       ├── can_mod.c
│       ├── gpio_mod.c
│       ├── cellular_mod.c
│       ├── mqtt_mod.c
│       ├── http_mod.c
│       ├── modbus_mod.c
│       └── wdt_mod.c
└── deps/                 # third-party libs (see deps/README.md)
    ├── buildDepends.sh
    ├── packages/source.txt
    ├── arm-linux-gnueabihf/    # DEPS_PREFIX after extract (gitignored)
    └── prebuilt/
        └── arm-linux-gnueabihf.tar.gz
```

## Build

From the repository root, activate the cross toolchain and build:

```bash
. scripts/env.toolchain.sh
make -C demo
```

On the first build, if `deps/arm-linux-gnueabihf/` is missing, the Makefile
automatically extracts `deps/prebuilt/arm-linux-gnueabihf.tar.gz`.

Or pass the cross prefix explicitly:

```bash
cd demo
make CROSS_COMPILE=arm-linux-gnueabihf-
```

Output: `build/iepro_demo`

Third-party libraries (MQTT, Modbus, curl, OpenSSL) are **linked statically** from
`deps/arm-linux-gnueabihf/`; no `LD_LIBRARY_PATH` is required on the device.
To extract prebuilt deps manually or rebuild from source, see [`deps/README.md`](deps/README.md).

## Run

```bash
./build/iepro_demo              # interactive menu
./build/iepro_demo --help       # CLI overview
./build/iepro_demo <module>     # module-specific CLI help
```

Most operations require **root** (GPIO, CAN, serial ports, cellular dial-up, hardware watchdog `/dev/watchdog`).

### Navigation (interactive menu)

- Press **`0`** at any menu to go back to the previous level.
- Press **Ctrl+C** during a blocking loop (receive, blink, publish, etc.) to stop the loop
  and return to the current sub-menu.
- Press **Ctrl+C** at a menu prompt to go back one level (same as `0`).

### Main menu

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

### Serial (`1`)

Pick port `1=RS232-1`, `2=RS485-1`, `3=RS485-2` and baud rate when prompted.

```
 1) Loop receive (Ctrl+C to stop)
 2) Loop send (Ctrl+C to stop)
 3) Loop echo (receive & reply, Ctrl+C to stop)
 0) Back
```

### CAN (`2`)

```
 1) Bring up can0 (default 250000 bps)
 2) Listen for one frame (3s timeout)
 3) Send test frame (ID 0x123)
 0) Back
```

### GPIO (`3`)

```
 1) Init all board GPIO (DI/DO/DIP/Reset)
 2) Monitor DI, DIP & Reset button (Ctrl+C to stop)
 3) Set DO high (Y1)
 4) Set DO low  (Y1)
 5) Run demo pulse on DO (Ctrl+C to stop)
 6) LED test (steady ON / OFF / fast blink)
 0) Back
```

### MQTT (`4`)

Configure via menu (option 1), CLI `--broker` / `--topic`, or edit compile-time defaults in
`src/modules/mqtt_mod.c` (`MQTT_DEFAULT_BROKER`, `MQTT_DEFAULT_CLIENT_ID`, topic macros).

```
 1) Configure MQTT connection parameters
 2) Connect (broker session + background loop)
 3) Disconnect
 4) Show current status
 5) Publish message
 0) Back
```

Publish (menu 5 or CLI `publish`): enter a custom message, or press Enter / omit
`--message` to send sample metrics JSON (DI/DIP readings via `metrics.c`).

### Cellular (`5`)

SIM7600G-H-PCIE over AT port `/dev/ttyUSB2`; NDIS dial-up on `wwan0` via `AT$QCRMCALL`.
**4G module power** is controlled by GPIO 69 (OUT); off by default at boot — the demo powers on automatically on first AT access.
See [`docs/en/03-4g-connectivity.md`](../docs/en/03-4g-connectivity.md) ([中文](../docs/zh-CN/03-4g-connectivity.md)).

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

### HTTP (`6`)

Uses **libcurl** (HTTPS supported; optional `--ca` for TLS verify).

```
 1) Set URL
 2) Set CA certificate path
 3) Show current configuration
 4) GET
 5) POST
 0) Back
```

Default URL: `https://example.com`

### Modbus (`7`)

RTU (serial) or TCP; master (poll/read/write) or slave (server).

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

### Watchdog (`8`)

Hardware watchdog device **`/dev/watchdog`**. Once enabled, a user process must feed it periodically; the system reboots if the timer expires.

- **Default timeout**: 60 seconds (configurable)
- **Feed interval**: about one-third of the timeout
- **Graceful stop**: send `SIGINT` to the feeder (or Ctrl+C); writes magic close `V` before closing the device
- **Trigger reboot**: send `SIGUSR1` to the feeder; sets timeout to 1 s and stops feeding until hardware reset

```
 1) Start keepalive (foreground)
 2) Stop running feeder
 3) Reboot via watchdog timeout
 0) Back
```

> The on-device Python script `hardwareWDT.py` has been ported to `wdt_mod.c`. For production, run `watchdog start` as a boot service.

## CLI

CLI actions mirror the interactive menus. Loop actions run until **Ctrl+C**.

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

Run `iepro_demo <module>` without an action to see options and examples.

### CLI examples

```bash
# Serial — echo on RS485-1 @ 9600
./iepro_demo serial echo --port 2 --baud 9600

# CAN — bring up can0 and send a test frame
./iepro_demo can up --bitrate 250000
./iepro_demo can send

# GPIO — monitor DI/DIP until Ctrl+C
./iepro_demo gpio monitor

# MQTT — publish once
./iepro_demo mqtt publish --broker 192.168.1.10 --port 1883 \
  --topic iepro/demo/data --message '{"temp":25}'
# omit --message to publish default metrics JSON (DI/DIP via metrics.c)
./iepro_demo mqtt publish --broker 192.168.1.10 --topic iepro/demo/data

# Cellular — read IMEI, then dial up
./iepro_demo cellular imei
./iepro_demo cellular connect --apn internet

# HTTP — GET / POST
./iepro_demo http get --url https://example.com
./iepro_demo http post --url https://httpbin.org/post \
  --body '{"message":"hello from IEPro demo"}'

# Modbus — RTU master poll on RS485-1
./iepro_demo modbus run --link rtu --role master --port 2 --baud 9600 \
  --unit-id 1 --start-addr 0 --count 10 --function 3 --poll-interval 5

# Modbus — TCP slave on port 502
./iepro_demo modbus run --link tcp --role slave --tcp-port 502 \
  --unit-id 1 --holding-regs 64

# Watchdog — foreground feed (stop/reboot from another terminal)
./iepro_demo watchdog start --timeout 60
./iepro_demo watchdog stop
./iepro_demo watchdog reboot
```

## Hardware map

| Interface | Device node / GPIO |
|---|---|
| RS232-1 | `/dev/ttymxc5` |
| RS485-1 | `/dev/ttymxc1` |
| RS485-2 | `/dev/ttymxc2` |
| CAN | `can0` (default 250000 bps) |
| Cellular AT | `/dev/ttyUSB2` |
| Cellular data | `wwan0` |
| 4G module power | GPIO 69 (OUT; 1=on, 0=off; off by default at boot) |
| DI (X1) | GPIO 117 |
| DO (Y1) | GPIO 118 |
| DIP switch 1 | GPIO 124 |
| DIP switch 2 | GPIO 121 |
| Reset button | GPIO 119 |
| POWER LED | — (hardware; on when powered) |
| NET LED | GPIO 122 |
| RUN LED | GPIO 71 |
| WARN LED | GPIO 123 |
| Hardware watchdog | `/dev/watchdog` (PID file `/tmp/iepro_wdt.pid`) |

Constants are defined in `src/common/iepro_hw.h`.

## CAN helper script

```bash
sh scripts/can_setup.sh          # can0 @ 250000 bps (default)
sh scripts/can_setup.sh can0 500000
```
