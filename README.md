# IE Pro 400 Global Standard — Developer Resources

[English](README.md) | [中文](README.zh-CN.md)

> **Status**: V1.0 base release (2026-07-22). Repository update **V1.1** (2026-08-19): HTTP/Modbus/CLI, unified `demo/deps` prebuilt v0.2, bilingual demo operational guides. See [Release Notes](docs/en/09-release-notes.md).

Developer documentation and reference code for the IE Pro 400 Global Standard
industrial gateway. This is the general-purpose global
edition: the device ships without any vendor application or platform —
customers build and deploy their own software on top of it.

## Documentation

| # | Document | EN | 中文 |
|---|---|---|---|
| 01 | Datasheet | [en](docs/en/01-datasheet.md) | [zh-CN](docs/zh-CN/01-datasheet.md) |
| 02 | Quickstart Guide | [en](docs/en/02-quickstart.md) | [zh-CN](docs/zh-CN/02-quickstart.md) |
| 03 | 4G Connectivity Example | [en](docs/en/03-4g-connectivity.md) | [zh-CN](docs/zh-CN/03-4g-connectivity.md) |
| 04 | Wired Connectivity Example | [en](docs/en/04-wired-connectivity.md) | [zh-CN](docs/zh-CN/04-wired-connectivity.md) |
| 05 | Demo Development Guide | [en](docs/en/05-demo-guide.md) | [zh-CN](docs/zh-CN/05-demo-guide.md) |
| 06 | Cross-Compilation Toolchain Guide | [en](docs/en/06-cross-compile-toolchain.md) | [zh-CN](docs/zh-CN/06-cross-compile-toolchain.md) |
| 07 | FAQ | [en](docs/en/07-faq.md) | [zh-CN](docs/zh-CN/07-faq.md) |
| 08 | Downloads | [en](docs/en/08-downloads.md) | [zh-CN](docs/zh-CN/08-downloads.md) |
| 09 | Release Notes | [en](docs/en/09-release-notes.md) | [zh-CN](docs/zh-CN/09-release-notes.md) |

## Demo Source Code

[`/demo`](demo) — **source code is English-only** (comments and identifiers).
Operational guides: [`demo/README.md`](demo/README.md) (EN) |
[`demo/README.zh-CN.md`](demo/README.zh-CN.md) (中文), regardless of which
documentation language you're reading.

```
repo-root/
├── README.md              # this file
├── README.zh-CN.md
├── LICENSE                 # Apache License 2.0
├── NOTICE                  # copyright and trademark notice
├── CONTRIBUTING.md         # maintainer guide (EN)
├── CONTRIBUTING.zh-CN.md   # maintainer guide (ZH)
├── scripts/
│   ├── setup_toolchain.sh  # cross-compilation toolchain helper
│   └── env.toolchain.sh
│   # (planned) check_links.py + .github/workflows/check-links.yml
├── docs/
│   ├── en/                 # English documentation
│   ├── zh-CN/              # Chinese documentation
│   ├── assets/              # shared/en/zh-CN screenshots & diagrams
│   └── certificates/        # published certificate PDFs
└── demo/                   # Demo source (C-only, builds iepro_demo)
    ├── Makefile
    ├── README.md           # operational guide (EN)
    ├── README.zh-CN.md     # operational guide (ZH)
    ├── deps/               # third-party libs (prebuilt + buildDepends.sh)
    ├── src/
    │   ├── main.c
    │   ├── common/
    │   └── modules/
    └── scripts/
        └── can_setup.sh
```

## Language Policy

- **Documentation** (`docs/`): maintained in both English and
  Chinese, filenames identical across `en/` and `zh-CN/` so the two
  trees stay 1:1 mappable.
- **Code** (`demo/`): **C-only** demos; source comments and identifiers in English.
  Operational READMEs (`demo/README.md`, `demo/README.zh-CN.md`) are bilingual.
  The device ships with Python 2.7.14 for optional customer scripting.

## Contributing / Maintenance

See [CONTRIBUTING.md](CONTRIBUTING.md) for the full guide. In short:

- Keep `docs/en/*` and `docs/zh-CN/*` in sync — when one language
  changes materially, update the other or flag it in the PR.
- Filenames must stay identical across `en/` and `zh-CN/`.
- Do not duplicate demo source into the docs; link to `/demo` instead.
- **Planned:** `scripts/check_links.py` and CI (`.github/workflows/check-links.yml`) for internal links, EN/ZH parity, and placeholder counts — not yet in the repo; review manually for now.

## License

Licensed under the [Apache License, Version 2.0](LICENSE) (SPDX-License-Identifier: Apache-2.0).

Documentation and demo source in this repository may be used, modified, and
redistributed under those terms. **AnyLink**, **IE Pro**, and related product
names are trademarks; this license does not grant trademark rights. See
[NOTICE](NOTICE) for attribution details.
