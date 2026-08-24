# IE Pro 400 Global Standard — FAQ

English | [中文](../zh-CN/08-faq.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## 1. SIM Card Not Recognized

**Symptom**: `AT+CPIN?` returns nothing or ERROR.

Troubleshooting:

1. **Power off**, re-insert the SIM card in the correct orientation (contacts facing the inner spring contact).
2. Enable 4G module power per [4G Connectivity Example](03-4g-connectivity.md) §2 (set GPIO 69 to 1).
3. Verify the SIM works in another device (e.g. a phone) to rule out a damaged card.
4. Run `AT+CPIN?` and check the response:
   - `+CPIN: READY`: card recognized — check APN/network (see §2).
   - `+CPIN: SIM PIN`: card locked — enter PIN to unlock.
   - No response / ERROR: slot contact issue or module fault — contact support.
5. Check the slot spring contact for deformation.
6. Wait at least 60 seconds after power-on before checking — the module needs initialization time.

## 2. APN Error

**Symptom**: SIM recognized but no data connection established.

Troubleshooting:

1. Confirm 4G module power is enabled per [4G Connectivity Example](03-4g-connectivity.md) §2 (GPIO 69 = 1) and `/dev/ttyUSB2` is present.
2. Confirm APN, username, and password are correct (case-sensitive). See [4G Connectivity Example](03-4g-connectivity.md) §3.1 for common examples.
3. IoT SIM cards often require a dedicated APN — confirm with your carrier.
4. Run `AT+CEREG?` (LTE) or `AT+CGREG?` (non-LTE) to confirm registration (`0,1` or `0,5`).
5. Confirm the SIM plan includes data service and is not suspended.
6. Retry dial-up: `AT$QCRMCALL=0,1` then `AT$QCRMCALL=1,1`.

## 3. Cannot Reach the Internet (4G / Wired)

**Symptom**: Network appears connected but ping to the Internet fails.

Troubleshooting:

1. Check local link first:
   - Wired: confirm link LED is on; run `ip addr show` to verify an IP was assigned.
   - 4G: confirm GPIO 69 is powered on and `/dev/ttyUSB2` exists (see [4G Connectivity Example](03-4g-connectivity.md) §2); run `AT+CSQ` for signal strength, `AT+CEREG?` for registration status.
2. Layered diagnosis:
   ```
   ping <gateway-ip>     # local link
   ping 8.8.8.8           # Internet (IP direct)
   ping www.example.com   # DNS resolution
   ```
3. If the gateway is reachable but the public Internet is not, check upstream routing/firewall or 4G public-IP restrictions.
4. If only hostnames fail, check DNS settings ([Wired Connectivity Example](04-wired-connectivity.md) §2.2).

## 4. Cannot SSH into the Device

**Symptom**: `ssh root@192.168.101.204` times out or is refused.

Troubleshooting:

1. Confirm the PC IP is on the same subnet as the LAN port (factory default LAN: `192.168.101.204`; set PC to `192.168.101.x`).
2. `ping 192.168.101.204` first to verify link connectivity.
3. Confirm the device has finished booting (~60–90 seconds).
4. Confirm the correct password: SSH account is `root`; the initial password is unique per device — obtain it from the manufacturer using the product unique code (see §9).
5. Check PC firewall/security software blocking SSH (port 22).
6. Try serial Console login (115200 baud — see [Quickstart Guide](02-quickstart.md) §5.3).
7. If you changed the password with `passwd` and forgot it, contact the manufacturer with the product unique code (see §6, §9).

## 5. Interface Wiring Issues

**Symptom**: No data on serial/CAN/DI-DO interfaces.

| Interface | Common cause | Fix |
|---|---|---|
| RS232/RS485 | Baud/parity/stop-bit mismatch | Match parameters on both ends; confirm device nodes (`ttymxc1`/`ttymxc2`/`ttymxc5`) |
| RS485 | Reversed A/B polarity | Check wiring; add termination resistors on long buses |
| CAN | Missing termination, baud rate mismatch, interface down | Bring up `can0`; match bus bitrate (commonly 250000 bps); 120 Ω at each bus end |
| DI (X1) | Wrong wiring | Passive input (dry contact) — short to GND to trigger (GPIO 117) |
| DO (Y1) | Load exceeds rating | Passive output (dry contact) — check datasheet §3.4; add a relay if needed |

## 6. Reset Button & Factory Reset

### Reset button (GPIO 119)

The device provides a physical Reset button readable via sysfs GPIO: **pressed=1, released=0**. In the demo, use the GPIO menu item **Monitor DI, DIP & Reset button** for live state (see [`demo/README.md`](../../demo/README.md)).

**Factory firmware does not** implement button actions such as long-press factory reset. Implement custom button logic in your application if needed.

### Factory reset

**When**: Forgotten password, unrecoverable misconfiguration.

Factory firmware **does not** support factory reset via the Reset button. Contact the manufacturer or dealer with the device **product unique code**. After reset, typical defaults include:

- WAN IP: `192.168.100.126`
- LAN IP: `192.168.101.204`
- SSH password: restored to the factory initial password (derived from the product unique code)

**Warning**: factory reset clears all custom configuration — back up first.

## 7. CAN Interface Shows No Data

**Symptom**: `candump can0` produces no output.

Troubleshooting:

1. Confirm the interface is up: `ip link show can0` should show `UP` (CAN module is factory-installed).
2. Confirm baud rate matches other nodes on the bus (commonly 250000 or 500000).
3. Run a loopback test: `ip link set can0 type can loopback on` (see [Demo Development Guide](05-demo-guide.md) §3.2 or [`demo/README.md`](../../demo/README.md)).

## 8. Cross-Compilation Toolchain Download Fails

**Symptom**: The official Linaro `.tar.xz` URL redirects to a contact page, or `wget`/`curl` fails.

Fix:

1. Use the repo setup script (tries mirrors automatically):
   ```bash
   sh scripts/setup_toolchain.sh
   . scripts/env.toolchain.sh
   ```
2. Download manually from a mirror — see [`toolchain/README.md`](../../toolchain/README.md) ([中文](../../toolchain/README.zh-CN.md)).
3. Full install steps: **Quick Start** in the [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md) ([中文](../zh-CN/06-cross-compile-toolchain.md)).

## 9. How to Obtain the Initial SSH Password

Each device ships with a **product unique code** (printed on the nameplate or shipping label). The initial SSH password is generated by the manufacturer from this code and is bound to that specific unit.

1. Find the product unique code on the device nameplate or packaging.
2. Provide the code to the manufacturer or dealer to obtain the corresponding initial SSH password.
3. Log in with `ssh root@192.168.101.204` (PC must be on the same subnet as the LAN port).
4. After login, use `passwd` to change the password.

> If you changed the password with `passwd` and forgot it, contact the manufacturer with the product unique code for assistance (see §6, §9). The Reset button does not trigger factory reset.

## 10. Hardware Watchdog

**Symptom**: Cannot open `/dev/watchdog`, or unexpected reboots after enabling the watchdog.

Notes:

1. **root** is required to access `/dev/watchdog`.
2. Once enabled, a process must keep feeding the watchdog — use `iepro_demo watchdog start` (see [`demo/README.md`](../../demo/README.md) and [Demo Development Guide](05-demo-guide.md) §3.8).
3. If feeding stops without a magic close `V`, the system reboots when the timer expires — expected protective behavior.
4. Stop from another terminal: `iepro_demo watchdog stop` (sends `SIGINT` to the PID in `/tmp/iepro_wdt.pid`).
5. Trigger reboot via watchdog: `iepro_demo watchdog reboot` (sends `SIGUSR1`).

## 11. Contact Support

If the above steps don't resolve the issue, provide: device model, firmware version ([Release Notes](10-release-notes.md)), symptom description, and relevant logs/screenshots.

| Channel | Info |
|---|---|
| Website | [anylink.io](https://anylink.io) |
| Email | [developer@anylink.io](mailto:developer@anylink.io) |
