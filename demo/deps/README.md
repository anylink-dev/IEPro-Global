# Third-party dependencies

Place **prebuilt** headers and libraries for the target toolchain here.
Each library has a fixed layout so the top-level `Makefile` can pick them up
with optional feature flags.

## Layout (per library)

```
deps/<name>/
├── include/    # public headers
└── lib/        # .a and/or .so for arm-linux-gnueabihf (or your target)
```

## Supported libraries

| Directory   | Makefile flag  | Typical link flags        | Used by        |
|-------------|----------------|---------------------------|----------------|
| `mosquitto/`| `WITH_MQTT=1`  | `-lmosquitto`             | `mqtt_mod.c`   |
| `modbus/`   | `WITH_MODBUS=1`| `-lmodbus`                | (future module)|
| `curl/`     | `WITH_CURL=1`  | `-lcurl`                  | (future module)|
| `openssl/`  | `WITH_SSL=1`   | `-lssl -lcrypto`          | HTTPS / TLS    |

## Example: mosquitto

After cross-compiling or copying a prebuilt SDK:

```
deps/mosquitto/
├── include/
│   └── mosquitto.h
└── lib/
    ├── libmosquitto.so
    └── libmosquitto.so.1
```

Build:

```bash
make WITH_MQTT=1 CROSS_COMPILE=arm-linux-gnueabihf-
```

Deploy to device (one of):

- Copy `deps/mosquitto/lib/*.so*` to `/usr/lib` on the gateway, or
- Set `LD_LIBRARY_PATH` to the directory containing the `.so` files, or
- Place the `.so` files in the same directory as `iepro_demo`.

## Notes

- Match the **same ABI** as your cross compiler (`arm-linux-gnueabihf` for IE Pro 400).
- Large binary artifacts are usually **not** committed to Git; keep only this README
  and empty `include/` / `lib/` placeholders, or document download URLs in your
  internal build notes.
- System packages on the device (if installed) can be used instead by building
  without `deps/` contents and linking against the device sysroot or `-I`/`-L`
  paths from your toolchain.
