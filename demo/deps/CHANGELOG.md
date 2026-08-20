# Changelog

Release notes for `prebuilt/arm-linux-gnueabihf.tar.gz`.  
Versions and checksum for the **current** snapshot: [`MANIFEST.md`](MANIFEST.md).

---

## v0.2 — 2026-08-19

Repacked prebuilt snapshot (`--pack-prebuilt`); library versions unchanged.

- Exclude `bin/` from the tarball (staging copy; client libraries only).
- SHA256: `cefdceb282a27daec5f35b402978fbeafa3bdd04bbf4cf11fed77aed4d35a768`

| Library | Version |
|---------|---------|
| libiconv | 1.17 |
| OpenSSL | 1.1.1k |
| Eclipse Mosquitto | 2.0.18 |
| curl | 7.54.0 |
| libmodbus | 3.1.10 |

---

## v0.1 — 2026-08-17

Initial unified deps layout: `buildDepends.sh`, prebuilt snapshot, Makefile static linking.

| Library | Version |
|---------|---------|
| libiconv | 1.17 |
| OpenSSL | 1.1.1k |
| Eclipse Mosquitto | 2.0.18 |
| curl | 7.54.0 |
| libmodbus | 3.1.10 |
