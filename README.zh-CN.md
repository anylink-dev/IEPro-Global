# IE Pro 400 GlobalStandard — 开发者资料

[English](README.md) | [中文](README.zh-CN.md)

> **状态**：V1.0 已基于产品内部资料正式发布。如有更新请参见[《版本说明》](docs/zh-CN/09-release-notes.md)。

面向 IE Pro 400 GlobalStandard 工业网关的开发者文档与参考代码。这是面向全球市场的通用版网关：设备出厂不预装任何厂商应用或平台，由客户自行在设备上开发部署应用程序。

## 文档列表

| 编号 | 文档 | 英文 | 中文 |
|---|---|---|---|
| 01 | 规格书 | [en](docs/en/01-datasheet.md) | [zh-CN](docs/zh-CN/01-datasheet.md) |
| 02 | 快速上手指南 | [en](docs/en/02-quickstart.md) | [zh-CN](docs/zh-CN/02-quickstart.md) |
| 03 | 4G联网示例 | [en](docs/en/03-4g-connectivity.md) | [zh-CN](docs/zh-CN/03-4g-connectivity.md) |
| 04 | 有线联网示例 | [en](docs/en/04-wired-connectivity.md) | [zh-CN](docs/zh-CN/04-wired-connectivity.md) |
| 05 | Demo开发示例 | [en](docs/en/05-demo-guide.md) | [zh-CN](docs/zh-CN/05-demo-guide.md) |
| 06 | 交叉编译工具说明 | [en](docs/en/06-cross-compile-toolchain.md) | [zh-CN](docs/zh-CN/06-cross-compile-toolchain.md) |
| 07 | 常见问题FAQ | [en](docs/en/07-faq.md) | [zh-CN](docs/zh-CN/07-faq.md) |
| 08 | 资料下载入口 | [en](docs/en/08-downloads.md) | [zh-CN](docs/zh-CN/08-downloads.md) |
| 09 | 版本说明 | [en](docs/en/09-release-notes.md) | [zh-CN](docs/zh-CN/09-release-notes.md) |

## Demo 源码

[`/demo`](demo) 目录下的源码统一为**纯英文**（注释、变量名，以及目录自带的 README），与你阅读的文档语言无关。详见 [`/demo/README.md`](demo/README.md)。

```
repo-root/
├── README.md              # 英文入口
├── README.zh-CN.md         # 本文件
├── LICENSE                 # Apache License 2.0
├── NOTICE                  # 版权与商标声明
├── CONTRIBUTING.md         # 维护指南（英文）
├── CONTRIBUTING.zh-CN.md   # 维护指南（中文）
├── scripts/
│   ├── setup_toolchain.sh  # 交叉编译工具链安装脚本
│   └── env.toolchain.sh
│   # （计划中）check_links.py + .github/workflows/check-links.yml
├── docs/
│   ├── en/                 # 英文文档
│   ├── zh-CN/              # 中文文档
│   └── assets/              # shared/en/zh-CN 截图与示意图
└── demo/                   # Demo 源码（纯 C，编译为 iepro_demo）
    ├── Makefile
    ├── src/
    │   ├── main.c
    │   ├── common/
    │   └── modules/
    └── scripts/
        └── can_setup.sh
```

## 语言策略

- **文档**（`docs/`）：中英文双语维护，`en/` 与 `zh-CN/` 目录下文件名保持完全一致，便于两个语言树一一对应。
- **代码**（`demo/`）：仅维护 **C 语言** Demo 源码；设备预装 Python 2.7.14 可供客户自研脚本使用，但本仓库 Demo 不使用 Python。

## 维护说明

完整指南见 [CONTRIBUTING.zh-CN.md](CONTRIBUTING.zh-CN.md)，要点：

- `docs/en/*` 与 `docs/zh-CN/*` 需保持同步，任一语言有实质性变更时请同步更新另一语言，或在 PR 中标注待同步。
- `en/` 与 `zh-CN/` 下的文件名必须保持一致。
- 不要把 Demo 源码复制进文档，统一链接到 `/demo`。
- **计划中：** `scripts/check_links.py` 与 CI（`.github/workflows/check-links.yml`）用于内部链接、中英文同步与占位符统计，尚未入库；目前请人工核对。

## License

本仓库采用 [Apache License 2.0](LICENSE)（SPDX-License-Identifier: Apache-2.0）。

文档与 Demo 源码可在该许可下使用、修改与再分发。**AnyLink**、**IE Pro** 及相关产品名称为商标，本许可不授予商标使用权。详见 [NOTICE](NOTICE)。
