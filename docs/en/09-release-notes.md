# IE Pro 400 GlobalStandard — Release Notes

English | [中文](../zh-CN/09-release-notes.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## 1. Applicable Models

| Model | Notes |
|---|---|
| IE Pro 400 GlobalStandard | Global edition; no AnyLink application or cloud platform pre-installed |

## 2. Firmware Version History

| Version | Release date | Model | Key changes | Notes |
|---|---|---|---|---|
| V1.0.0 | 2026-07-22 | IE Pro 400 GlobalStandard | First developer release | Standard Linux pre-installed; serial/CAN/GPIO interfaces exposed |

## 3. Demo Source Code Version History

| Version | Release date | Firmware version | Doc version | Changes |
|---|---|---|---|---|
| V1.0 | 2026-07-22 | V1.0.0 | V1.0 | Initial release: unified `iepro_demo` console (`demo/src/`); Serial, CAN, GPIO, Cellular (SIM7600 / `AT$QCRMCALL`), and optional MQTT (`WITH_MQTT=1`) submodules; device nodes and GPIO numbers aligned with verified hardware |

The in-source version is `DEMO_VERSION` in `demo/src/main.c` (maps to V1.0 above; currently `1.0.0`).

## 4. Cross-Compilation Toolchain Version History

| Version | Release date | Firmware version | GCC version | Changes |
|---|---|---|---|---|
| gcc-linaro-5.5.0-2017.10 | 2017-10 | V1.0.0 | GCC 5.5.0 | Initial toolchain, `arm-linux-gnueabihf` |

## 5. Documentation Version History

| Document | Version | Date | Changes |
|---|---|---|---|
| All documents (EN/ZH) | V1.0 | 2026-07-22 | Populated from internal product specs, user manual, system interface guide, cross-compile guide, and SIMCOM dial-up documentation |

## 6. Version Compatibility

- Demo **V1.0** ships with developer documentation **V1.0** and firmware **V1.0.0**; compatible with firmware V1.0.0 and above.
- Cross-compilation toolchain `gcc-linaro-5.5.0-2017.10` (`arm-linux-gnueabihf`) matches firmware V1.0.0.
- Firmware below V1.0.0 may have driver interface differences causing Demo build or runtime failures.

## 7. Known Issues

| Version | Issue | Impact | Planned fix |
|---|---|---|---|
| V1.0.0 | CAN module requires hardware soldering; may not be installed by default | Customers using CAN interface | Documented in user guide |
