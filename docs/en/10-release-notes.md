# IE Pro 400 Global Standard — Release Notes

English | [中文](../zh-CN/10-release-notes.md)

**Doc version**: V1.2　**Date**: 2026-08-24

## 1. Applicable Models

| Model | Notes |
|---|---|
| IE Pro 400 Global Standard | Global edition; no AnyLink application or cloud platform pre-installed |

## 2. Firmware Version History

| Version | Release date | Model | Key changes | Notes |
|---|---|---|---|---|
| V1.0.0 | 2026-07-22 | IE Pro 400 Global Standard | First developer release | Standard Linux pre-installed; serial/CAN/GPIO interfaces exposed |

## 3. Demo Source Code Version History

| Version | Release date | Firmware version | Doc version | Changes |
|---|---|---|---|---|
| V1.0 | 2026-07-22 | V1.0.0 | V1.0 | Initial release: unified `iepro_demo` console (`demo/src/`); Serial, CAN, GPIO, Cellular (SIM7600G-H-PCIE / `AT$QCRMCALL`), and optional MQTT (`WITH_MQTT=1`) submodules; device nodes and GPIO numbers aligned with verified hardware |
| V1.1 | 2026-08-19 | V1.0.0 | V1.1 | Added HTTP (libcurl), Modbus RTU/TCP, and matching CLI subcommands; unified `demo/deps` prebuilt layout (v0.2); MQTT/HTTP/Modbus/curl/OpenSSL linked by default; MQTT publish defaults to sample metrics JSON from [`metrics.c`](../../demo/src/common/metrics.c) when the body is empty; GPIO board I/O init on CLI startup; 4G module power GPIO 69 (OUT, off by default at boot; demo auto-enables and waits for `/dev/ttyUSB2`); hardware watchdog module `wdt_mod.c` (`/dev/watchdog`, ported from `hardwareWDT.py`); operational docs in [`demo/README.md`](../../demo/README.md) / [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md); 05-demo-guide slimmed to integration notes (`DEMO_VERSION` remains `1.0.0`) |

The in-source version is `DEMO_VERSION` in `demo/src/main.c` (maps to V1.0 above; currently `1.0.0`).

## 4. Cross-Compilation Toolchain Version History

| Version | Release date | Firmware version | GCC version | Changes |
|---|---|---|---|---|
| gcc-linaro-5.5.0-2017.10 | 2017-10 | V1.0.0 | GCC 5.5.0 | Initial toolchain, `arm-linux-gnueabihf` |

## 5. Documentation Version History

| Document | Version | Date | Changes |
|---|---|---|---|
| All documents (EN/ZH) | V1.0 | 2026-07-22 | Populated from internal product specs, user manual, system interface guide, cross-compile guide, and SIMCOM dial-up documentation |
| 05 Demo Guide (EN/ZH), `demo/README.md`, `demo/README.zh-CN.md` | V1.1 | 2026-08-19 | HTTP/Modbus/CLI; MQTT default metrics publish; hardware watchdog `wdt_mod.c`; split operational vs integration docs; deps prebuilt v0.2 |
| `09-downloads` (EN/ZH) | V1.1 | 2026-08-19 | Header and repository layout aligned with V1.1 |
| 07 Third-Party MQTT Protocol (EN/ZH) | V1.0 | 2026-08-24 | Protocol Word specs, `IEPro-deploy.zip` (agent, web UI, Boa), and deploy steps |
| Documentation set (EN/ZH) | V1.2 | 2026-08-24 | Inserted 07 third-party protocol; FAQ / downloads / release notes renumbered to 08 / 09 / 10 |

## 6. Version Compatibility

- Demo **V1.1** (repository update) is compatible with firmware **V1.0.0** and above; `DEMO_VERSION` in source remains `1.0.0` until the next firmware-aligned demo release.
- Demo **V1.0** ships with developer documentation **V1.0** and firmware **V1.0.0**; compatible with firmware V1.0.0 and above.
- Cross-compilation toolchain `gcc-linaro-5.5.0-2017.10` (`arm-linux-gnueabihf`) matches firmware V1.0.0.
- Firmware below V1.0.0 may have driver interface differences causing Demo build or runtime failures.

## 7. Known Issues

| Version | Issue | Impact | Planned fix |
|---|---|---|---|
| — | None reported for V1.0.0 | — | — |

## 8. Mass-Production Alignment

The following matches final mass-production shipments of **IE Pro 400 Global Standard**:

| Item | Verified content |
|---|---|
| Package contents | [Quickstart Guide](02-quickstart.md) §1 |
| Factory firmware | **V1.0.0** (2026-07-22) |
| Published certificates | CE / EMC / RoHS / RED for IE Pro 400 Global Standard; see [Product Certificates](certificates.md) |
