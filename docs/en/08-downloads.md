# IE Pro 400 Global Standard — Downloads

English | [中文](../zh-CN/08-downloads.md)

**Doc version**: V1.1　**Date**: 2026-08-19

## 1. Unified Resource Page

All developer resources are hosted in this Git repository. Entry point: [README.md](../../README.md).

## 2. Code Repository

| Item | Details |
|---|---|
| GitHub URL | https://github.com/anylink-dev/IEPro-Global.git |
| Contents | Bilingual developer documentation + Demo source code |
| Access | Public repository — `git clone` directly (see §6) |
| Versions & tags | See §5 Branch and tag conventions |

```bash
git clone https://github.com/anylink-dev/IEPro-Global.git
```

### Repository Layout

```
repo-root/
├── README.md              # English entry
├── README.zh-CN.md        # Chinese entry
├── docs/
│   ├── en/                 # English documentation
│   ├── zh-CN/              # Chinese documentation
│   └── assets/
└── demo/                   # Demo source (builds iepro_demo)
    ├── Makefile
    ├── README.md           # operational guide (EN)
    ├── README.zh-CN.md     # operational guide (ZH)
    ├── deps/               # third-party libs (prebuilt + buildDepends.sh)
    ├── src/
    │   ├── main.c
    │   ├── common/         # cli_util, gpio_util, metrics, ...
    │   └── modules/        # serial, can, gpio, cellular, mqtt, http, modbus, watchdog
    └── scripts/
        └── can_setup.sh
```

## 3. Resource List

| # | Document / Resource | Format | Status |
|---|---|---|---|
| 01 | [Datasheet](01-datasheet.md) (EN/ZH) | Markdown | V1.0 released |
| 02 | [Quickstart Guide](02-quickstart.md) | Markdown | V1.0 released |
| 03 | [4G Connectivity Example](03-4g-connectivity.md) | Markdown | V1.0 released |
| 04 | [Wired Connectivity Example](04-wired-connectivity.md) | Markdown | V1.0 released |
| 05 | [Demo Development Guide](05-demo-guide.md) | Markdown + source | V1.1 released |
| 06 | [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md) | Markdown | V1.0 released |
| 07 | [FAQ](07-faq.md) | Markdown | V1.0 released |
| 08 | Downloads (this file) | Markdown | — |
| 09 | [Release Notes](09-release-notes.md) | Markdown | V1.1 released |
| — | Cross-compilation toolchain | Linaro GCC 5.5.0 tar | [Mirror download](https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz) |
| — | Demo source code | Git (see `/demo`) | Included in this repo |
| — | Product certificates (CE/EMC/RoHS/RED) | Markdown + PDF | [Certificate index](certificates.md) |

## 4. Cross-Compilation Toolchain Download

| Item | Info |
|---|---|
| Toolchain | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` |
| Download | https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |
| Official index (direct link may fail) | https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/ |
| One-step install | `sh scripts/setup_toolchain.sh` (see [`toolchain/README.md`](../../toolchain/README.md) / [中文](../../toolchain/README.zh-CN.md)) |
| Install guide | See [Cross-Compilation Toolchain Guide](06-cross-compile-toolchain.md) |

## 5. Branch and Tag Conventions

This repository follows common **GitHub Flow + Semantic Versioning (SemVer 2.0)** practices.

### 5.1 Branch naming

| Branch | Purpose |
|---|---|
| `main` | Stable default branch; aligned with the latest release tag |
| `feature/<description>` | Feature work (branch from `main`, merge back via Pull Request) |
| `release/<version>` | Release preparation (e.g. `release/1.1.0`) |
| `hotfix/<description>` | Urgent fixes (branch from `main` or a tag, merge and tag when done) |

### 5.2 Tags and version mapping

| Type | Format | Example | Notes |
|---|---|---|---|
| Git tag | `v<major>.<minor>.<patch>` | `v1.0.0` | Use **annotated tags** for official releases; publish notes on GitHub **Releases** |
| Firmware | `V<major>.<minor>.<patch>` | `V1.0.0` | Maps 1:1 to the Git tag (`V1.0.0` ↔ `v1.0.0`) |
| Documentation | `V<major>.<minor>` | `V1.0` | Shipped with the matching firmware/tag release |
| Demo | `DEMO_VERSION` in `demo/src/main.c` | `1.0.0` | Binary version string; repo feature track V1.1 (2026-08-19) adds HTTP/Modbus/CLI while `DEMO_VERSION` stays `1.0.0` until next firmware-aligned release |

Check out a specific release:

```bash
git clone https://github.com/anylink-dev/IEPro-Global.git
cd IEPro-Global
git checkout v1.0.0
```

> Full history and compatibility: [Release Notes](09-release-notes.md).

## 6. Access and contribution

| Level | Who | How to obtain |
|---|---|---|
| **Read** | All developers | Public by default — `git clone` or Fork; no application required |
| **Contribute (Pull Request)** | Anyone improving docs or Demo | Fork → work on `feature/*` → open PR to `main` → Maintainer review. Scope and conventions: [CONTRIBUTING.md](../../CONTRIBUTING.md) |
| **Write and above** | Partners / team members who need direct push | Email [developer@anylink.io](mailto:developer@anylink.io) with company, purpose, and GitHub username; review typically within **3–5 business days** |

PRs are welcome for documentation, Demo, and asset fixes. For hardware specs, certifications, or firmware-related changes, open an Issue or email first — do not submit large unsolicited PRs.

**Write-access requests should include**: company name, contact person, GitHub username, and reason (e.g. joint development, long-term branch maintenance).

## 7. Stay updated

Recommended channels for documentation, Demo, and firmware-related updates:

| Channel | How | Best for |
|---|---|---|
| **GitHub Watch → Releases** | Open the [repository](https://github.com/anylink-dev/IEPro-Global) → Watch → Custom → enable **Releases** | Official release notifications (**recommended**) |
| **GitHub Releases RSS** | Subscribe to `https://github.com/anylink-dev/IEPro-Global/releases.atom` | RSS readers or internal monitoring |
| **Release notes doc** | Read [09-release-notes.md](09-release-notes.md) | Full changelog and compatibility |
| **Email opt-in (optional)** | Email [developer@anylink.io](mailto:developer@anylink.io) with subject “Subscribe to IEPro release updates” | Major-release email alerts when no mailing list is available |

| Other |
|---|
| Technical support: [anylink.io](https://anylink.io) / [developer@anylink.io](mailto:developer@anylink.io) |
