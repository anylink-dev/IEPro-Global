# Prebuilt Manifest

Current snapshot of `prebuilt/arm-linux-gnueabihf.tar.gz`.

| | |
|--|--|
| **Triplet** | `arm-linux-gnueabihf` |
| **Toolchain** | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` |
| **Build date** | 2026-08-17 |
| **Build** | `./buildDepends.sh --force-build --build-all --strip` → `--pack-prebuilt` |
| **SHA256** | `391ce34bb6c1a262ad3328d44c4ec203dd4718140a7c9bc74f01ce90f72572ab` |

```bash
cd demo/deps/prebuilt
sha256sum -c arm-linux-gnueabihf.tar.gz.sha256
```

---

## Libraries

| # | Library | Version | Artifacts (`lib/`, `include/`) |
|---|---------|---------|--------------------------------|
| 1 | libiconv | 1.17 | `libiconv.{so*,a}`; `iconv.h` |
| 2 | OpenSSL | 1.1.1k | `libssl`, `libcrypto` (`.so*`, `.a`); `openssl/` |
| 3 | Mosquitto | 2.0.18 | `libmosquitto`, `libmosquittopp` (`.so*`, `.a`); `mosquitto.h`, `mqtt_protocol.h`, `mosquittopp.h` |
| 4 | curl | 7.54.0 | `libcurl.{so*,a}`; `curl/` |
| 5 | libmodbus | 3.1.10 | `libmodbus.{so*,a}`; `modbus/` |

Shared and static libraries included; ELFs stripped. No `share/man`, `share/doc`, or `share/locale`. Mosquitto: client library only (no broker / `mosquitto_pub`).
