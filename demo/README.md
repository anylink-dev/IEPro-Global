# IE Pro 400 Global Standard — Demo Console

All demos are **C-only** and build into a **single executable** `build/iepro_demo`.
Run it on the device and pick a module from the interactive console menu.

See [`docs/en/05-demo-guide.md`](../docs/en/05-demo-guide.md) or
[`docs/zh-CN/05-demo-guide.md`](../docs/zh-CN/05-demo-guide.md) for details.

> **Note**: The device ships with **Python 2.7.14**. This demo suite is C-only.

## Directory layout

```
demo/
├── Makefile              # build rules
├── README.md
├── build/                # output: iepro_demo (gitignored)
├── scripts/
│   └── can_setup.sh      # bring up can0 (optional helper)
├── src/
│   ├── main.c            # entry point and main menu
│   ├── demo.h            # module declarations
│   ├── common/           # shared helpers and hardware map
│   │   ├── iepro_hw.h
│   │   ├── menu_util.c/h
│   │   ├── gpio_util.c/h
│   │   └── metrics.c/h   # sample metrics JSON (used by MQTT)
│   └── modules/          # feature modules (one file per menu item)
│       ├── serial_mod.c
│       ├── can_mod.c
│       ├── gpio_mod.c
│       ├── cellular_mod.c
│       └── mqtt_mod.c
└── deps/                 # third-party libs (see deps/README.md)
    ├── buildDepends.sh
    ├── packages/source.txt
    ├── arm-linux-gnueabihf/    # DEPS_PREFIX after extract (gitignored)
    └── prebuilt/
        └── arm-linux-gnueabihf.tar.gz
```

## Build

From the repository root, activate the cross toolchain and extract prebuilt dependencies:

```bash
. scripts/env.toolchain.sh
cd demo/deps
./buildDepends.sh --extract-prebuilt
cd ../demo
make
```

Or pass the cross prefix explicitly:

```bash
cd demo
make CROSS_COMPILE=arm-linux-gnueabihf-
```

Output: `build/iepro_demo`

Third-party libraries (MQTT, Modbus, curl, OpenSSL) are **linked statically** from
`deps/arm-linux-gnueabihf/`; no `LD_LIBRARY_PATH` is required on the device.
To rebuild dependencies from source, see [`deps/README.md`](deps/README.md).

## Run

```bash
./build/iepro_demo
```

Most operations require **root** (GPIO, CAN, serial ports, cellular dial-up).

### Navigation

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

Requires broker settings in `src/modules/mqtt_mod.c`
(`MQTT_BROKER`, `MQTT_DEVICE_ID`).

```
 1) Publish one sample message
 2) Run publish loop (10s interval, Ctrl+C to stop)
 0) Back
```

Payload includes DI/DIP readings and sample metrics JSON.

### Cellular (`5`)

SIM7600G-H-PCIE over AT port `/dev/ttyUSB2`; NDIS dial-up on `wwan0` via `AT$QCRMCALL`.
See [`docs/zh-CN/03-4g-connectivity.md`](../docs/zh-CN/03-4g-connectivity.md).

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

## Hardware map

| Interface | Device node / GPIO |
|---|---|
| RS232-1 | `/dev/ttymxc5` |
| RS485-1 | `/dev/ttymxc1` |
| RS485-2 | `/dev/ttymxc2` |
| CAN | `can0` (default 250000 bps) |
| Cellular AT | `/dev/ttyUSB2` |
| Cellular data | `wwan0` |
| DI (X1) | GPIO 117 |
| DO (Y1) | GPIO 118 |
| DIP switch 1 | GPIO 124 |
| DIP switch 2 | GPIO 121 |
| Reset button | GPIO 119 |
| POWER LED | — (hardware; on when powered) |
| NET LED | GPIO 122 |
| RUN LED | GPIO 71 |
| WARN LED | GPIO 123 |

Constants are defined in `src/common/iepro_hw.h`.

## CAN helper script

```bash
sh scripts/can_setup.sh          # can0 @ 250000 bps (default)
sh scripts/can_setup.sh can0 500000
```
