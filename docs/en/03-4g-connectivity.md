# IE Pro 400 Global Standard — 4G Connectivity Example

English | [中文](../zh-CN/03-4g-connectivity.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## 1. Insert SIM Card

1. **Power off** the device.
2. Open the SIM card slot (location in [Datasheet](01-datasheet.md) §3.5).
3. Insert a standard SIM card in the direction marked on the slot (contacts facing the inner spring contact).
4. Push until the latch clicks, then power on.
5. Wait ~60 seconds, enable module power per §2, then confirm card recognition via AT commands (see §3, §4).

> **Note**: Hot-swapping the SIM card is not recommended — it may prevent the module from recognizing the card.

## 2. Enable 4G Module Power

IE Pro 400 Global Standard controls 4G module power via **GPIO 69** (output). **Power is off by default at boot** (module unpowered). Set GPIO 69 to **1** before AT commands, dial-up, or demo cellular features; write **0** to power off.

### 2.1 sysfs example

```bash
# export GPIO (if not already exported)
echo 69 > /sys/class/gpio/export

# set as output
echo out > /sys/class/gpio/gpio69/direction

# power on (1=on, 0=off)
echo 1 > /sys/class/gpio/gpio69/value

# wait for AT port (USB enumeration may take up to ~10 s)
for i in $(seq 1 20); do
  [ -e /dev/ttyUSB2 ] && break
  sleep 0.5
done
ls -l /dev/ttyUSB2
```

Write `0` to `value` to power off.

### 2.2 Demo auto power-on

The `iepro_demo` cellular module automatically enables GPIO 69 on first AT port open and polls for `/dev/ttyUSB2` (10 s timeout — see `demo/src/modules/cellular_mod.c`).

## 3. APN Configuration & Dial-Up

IE Pro 400 Global Standard uses a SIMCOM **SIM7600G-H-PCIE** module with **NDIS dial-up** via `AT$QCRMCALL`.

### 3.1 Common Carrier APN Examples

| Carrier | APN | Dial command example |
|---|---|---|
| China Mobile (public) | `cmiot` | `AT$QCRMCALL=1,1,,,,,"cmiot","none","none",3` |
| China Telecom (public LTE) | `ctlte` | `AT$QCRMCALL=1,1,,,,,"ctlte","none","none",3` |
| China Unicom (public 3GPP) | leave empty (auto) | `AT$QCRMCALL=1,1` |
| Private network SIM | provided by carrier | `AT$QCRMCALL=1,1,,,,,"<apn>","<user>","<pass>",3` |

> IoT SIM cards often require a dedicated APN — confirm with your carrier. Username/password are case-sensitive.

### 3.2 Pre-Dial Check Sequence

Connect to the module's AT debug port and run (enable power per §2 first):

```
AT+CFUN=0          # restart module (optional)
AT+CFUN=1
AT+CPIN?           # check SIM — expect +CPIN: READY
AT+CSQ             # signal strength
AT+CNSMOD=1        # enable network mode auto-report
AT+CNSMOD?         # current mode (8 = LTE)
AT+CEREG?          # LTE registration (0,1 or 0,5 = data available)
AT+CGREG?          # non-LTE registration
```

### 3.3 Start Dial-Up

**Public 3GPP mode (GSM/WCDMA/LTE)**:

```
AT$QCRMCALL=1,1
```

**China Mobile example**:

```
AT$QCRMCALL=1,1,,,,,"cmiot","none","none",3
```

**China Telecom example**:

```
AT$QCRMCALL=1,1,,,,,"ctlte","none","none",3
```

**Hang up**:

```
AT$QCRMCALL=0,1
```

**Query dial-up status**:

```
AT$QCRMCALL?
```

## 4. Check Connectivity Status

### 4.1 AT Self-Test

| Command | Key response | Meaning |
|---|---|---|
| `AT+CPIN?` | `READY` | SIM card OK |
| `AT+CPIN?` | `SIM PIN` / `SIM PUK` | Card locked — enter PIN/PUK |
| `AT+CSQ` | 0–31 (not 99) | Signal present; higher = stronger |
| `AT+CSQ` | `99,99` | No signal |
| `AT+CEREG?` | `0,1` or `0,5` | LTE registered; data service available |
| `AT+CGREG?` | `0,1` or `0,5` | Non-LTE registered |
| `AT+CNSMOD?` | second field `8` | Currently on LTE network |

> **Note**: In LTE mode, use `AT+CEREG?` to check data service availability; in non-LTE mode, use `AT+CGREG?`.

### 4.2 System-Level Verification

After dial-up succeeds, in an SSH session on the device:

```bash
# check network interfaces
ip addr show

# test Internet access
ping -c 4 8.8.8.8
```

## 5. Ping Test

```bash
ping -c 4 8.8.8.8
```

Expected result (no packet loss):

```
4 packets transmitted, 4 received, 0% packet loss
```

If packets are lost or DNS fails, see the connectivity section in the [FAQ](08-faq.md).

## 6. Reference Documents

- SIMCOM official document: *SIM7500_SIM7600 Linux NDIS Dial-Up Flow* (full AT command reference)

---

**Module info**: SIM7600G-H-PCIE; supports GSM/GPRS/EDGE, WCDMA/UMTS/HSPA+, LTE-FDD, LTE-TDD. Band details in [Datasheet](01-datasheet.md) §3.5.
