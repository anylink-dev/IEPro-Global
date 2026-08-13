# IE Pro 400 Global Standard — 4G Connectivity Example

English | [中文](../zh-CN/03-4g-connectivity.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## 1. Insert SIM Card

1. **Power off** the device.
2. Open the SIM card slot (location in [Datasheet](01-datasheet.md) §3.5).
3. Insert a standard SIM card in the direction marked on the slot (contacts facing the inner spring contact).
4. Push until the latch clicks, then power on.
5. Wait ~60 seconds, then confirm card recognition via AT commands (see §3).

> **Note**: Hot-swapping the SIM card is not recommended — it may prevent the module from recognizing the card.

## 2. APN Configuration & Dial-Up

IE Pro 400 Global Standard uses a SIMCOM **SIM7600G-H-PCIE** module with **NDIS dial-up** via `AT$QCRMCALL`.

### 2.1 Common Carrier APN Examples

| Carrier | APN | Dial command example |
|---|---|---|
| China Mobile (public) | `cmiot` | `AT$QCRMCALL=1,1,,,,,"cmiot","none","none",3` |
| China Telecom (public LTE) | `ctlte` | `AT$QCRMCALL=1,1,,,,,"ctlte","none","none",3` |
| China Unicom (public 3GPP) | leave empty (auto) | `AT$QCRMCALL=1,1` |
| Private network SIM | provided by carrier | `AT$QCRMCALL=1,1,,,,,"<apn>","<user>","<pass>",3` |

> IoT SIM cards often require a dedicated APN — confirm with your carrier. Username/password are case-sensitive.

### 2.2 Pre-Dial Check Sequence

Connect to the module's AT debug port and run:

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

### 2.3 Start Dial-Up

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

## 3. Check Connectivity Status

### 3.1 AT Self-Test

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

### 3.2 System-Level Verification

After dial-up succeeds, in an SSH session on the device:

```bash
# check network interfaces
ip addr show

# test Internet access
ping -c 4 8.8.8.8
```

## 4. Ping Test

```bash
ping -c 4 8.8.8.8
```

Expected result (no packet loss):

```
4 packets transmitted, 4 received, 0% packet loss
```

If packets are lost or DNS fails, see the connectivity section in the [FAQ](07-faq.md).

## 5. Reference Documents

- SIMCOM official document: *SIM7500_SIM7600 Linux NDIS Dial-Up Flow* (full AT command reference)

---

**Module info**: SIM7600G-H-PCIE; supports GSM/GPRS/EDGE, WCDMA/UMTS/HSPA+, LTE-FDD, LTE-TDD. Band details in [Datasheet](01-datasheet.md) §3.5.
