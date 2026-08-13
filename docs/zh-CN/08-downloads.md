# IE Pro 400 Global Standard — 资料下载入口

[English](../en/08-downloads.md) | 中文

**文档版本**：V1.0　**日期**：2026-07-22

## 1. 统一资源页

开发者资料统一托管于本 Git 仓库，入口为仓库根目录 [README.zh-CN.md](../../README.zh-CN.md)。

## 2. 代码仓库

| 项目 | 说明 |
|---|---|
| GitHub 地址 | https://github.com/anylink-dev/IEPro-Global.git |
| 仓库内容 | 中英文开发者文档 + Demo 源码 |
| 访问方式 | 公开仓库，可直接 `git clone`（详见 §6） |
| 版本与 Tag | 见 §5 分支与 Tag 命名规范 |

```bash
git clone https://github.com/anylink-dev/IEPro-Global.git
```

### 仓库目录结构

```
repo-root/
├── README.md              # 英文入口
├── README.zh-CN.md        # 中文入口
├── docs/
│   ├── en/                 # 英文文档
│   └── zh-CN/              # 中文文档
└── demo/                   # Demo 源码（编译为 iepro_demo）
    ├── Makefile
    ├── src/
    │   ├── main.c
    │   ├── common/
    │   └── modules/
    └── scripts/
        └── can_setup.sh
```

## 3. 资料清单

| 编号 | 文档/资源 | 格式 | 状态 |
|---|---|---|---|
| 01 | [规格书](01-datasheet.md)（中/英文） | Markdown | V1.0 已发布 |
| 02 | [快速上手指南](02-quickstart.md) | Markdown | V1.0 已发布 |
| 03 | [4G 联网示例](03-4g-connectivity.md) | Markdown | V1.0 已发布 |
| 04 | [有线联网示例](04-wired-connectivity.md) | Markdown | V1.0 已发布 |
| 05 | [Demo 开发示例](05-demo-guide.md) | Markdown + 源码 | V1.0 已发布 |
| 06 | [交叉编译工具说明](06-cross-compile-toolchain.md) | Markdown | V1.0 已发布 |
| 07 | [常见问题 FAQ](07-faq.md) | Markdown | V1.0 已发布 |
| 08 | 资料下载入口（本文件） | Markdown | — |
| 09 | [版本说明](09-release-notes.md) | Markdown | V1.0 已发布 |
| — | 交叉编译工具链 | Linaro GCC 5.5.0 tar 包 | [镜像下载](https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz) |
| — | Demo 源码 | Git（见 `/demo`） | 已随本仓库提供 |
| — | 产品认证证书（CE/EMC/RoHS/RED） | PDF | [证书索引](../certificates/README.md) |

## 4. 交叉编译工具链下载

| 项目 | 信息 |
|---|---|
| 工具链 | `gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf` |
| 下载地址 | https://mirrors.dotsrc.org/armbian-dl/_toolchain/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz |
| 官方目录（直链可能失效） | https://releases.linaro.org/components/toolchain/binaries/5.5-2017.10/arm-linux-gnueabihf/ |
| 一键安装 | `sh scripts/setup_toolchain.sh`（见 [`toolchain/README.zh-CN.md`](../../toolchain/README.zh-CN.md) / [English](../../toolchain/README.md)） |
| 安装说明 | 见[《交叉编译工具说明》](06-cross-compile-toolchain.md) |

## 5. 分支与 Tag 命名规范

本仓库遵循业界常见的 **GitHub Flow + 语义化版本（SemVer 2.0）** 约定。

### 5.1 分支命名

| 分支 | 用途 |
|---|---|
| `main` | 稳定主干，与最新正式发布 Tag 保持一致 |
| `feature/<描述>` | 功能开发（从 `main` 拉出，经 Pull Request 合并回 `main`） |
| `release/<版本号>` | 发布准备（如 `release/1.1.0`） |
| `hotfix/<描述>` | 紧急修复（从 `main` 或对应 Tag 拉出，修复后合并并打 Tag） |

### 5.2 Tag 命名与版本对应

| 类型 | 格式 | 示例 | 说明 |
|---|---|---|---|
| Git Tag | `v<major>.<minor>.<patch>` | `v1.0.0` | 使用 **Annotated Tag** 标记正式发布；在 GitHub **Releases** 附变更说明 |
| 固件版本 | `V<major>.<minor>.<patch>` | `V1.0.0` | 与 Git Tag 一一对应（`V1.0.0` ↔ `v1.0.0`） |
| 文档版本 | `V<major>.<minor>` | `V1.0` | 随对应固件/Tag 发布包一并发布 |
| Demo 版本 | `DEMO_VERSION`（`demo/src/main.c`） | `1.0.0` | 与文档 V1.0 同步发布（`1.0.0` ↔ Demo V1.0） |

检出指定正式版本：

```bash
git clone https://github.com/anylink-dev/IEPro-Global.git
cd IEPro-Global
git checkout v1.0.0
```

> 版本历史与兼容性见[《版本说明》](09-release-notes.md)。

## 6. 访问权限与贡献流程

| 权限级别 | 适用对象 | 获取方式 |
|---|---|---|
| **只读（Read）** | 所有开发者 | 默认公开，无需申请；`git clone` 或 Fork 即可 |
| **贡献（Pull Request）** | 改进文档或 Demo 的任何人 | Fork → `feature/*` 分支修改 → 向 `main` 发起 PR → Maintainer 审核。范围与约定见 [CONTRIBUTING.zh-CN.md](../../CONTRIBUTING.zh-CN.md) |
| **写入（Write）及以上** | 需直接推送分支的合作伙伴/内部成员 | 邮件 [developer@anylink.io](mailto:developer@anylink.io) 说明公司、用途及 GitHub 账号；通常 **3～5 个工作日**内审核 |

欢迎就文档、Demo 与图片资源提交 PR。涉及硬件规格、认证或固件相关变更时，请先开 Issue 或发邮件沟通，勿未经约定提交大规模 PR。

**申请写入权限邮件需包含**：公司名称、联系人、GitHub 用户名、申请原因（如联合开发、长期维护分支）。

## 7. 更新订阅

推荐通过以下渠道跟踪文档、Demo 与固件配套资料更新：

| 渠道 | 操作 | 适用场景 |
|---|---|---|
| **GitHub Watch → Releases** | 打开 [仓库](https://github.com/anylink-dev/IEPro-Global) → Watch → Custom → 勾选 **Releases** | 正式版本发布通知（**推荐**） |
| **GitHub Releases RSS** | 订阅 `https://github.com/anylink-dev/IEPro-Global/releases.atom` | 集成到 RSS 阅读器或内部监控系统 |
| **版本说明文档** | 查阅 [09-release-notes.md](09-release-notes.md) | 查看完整变更历史与兼容性说明 |
| **邮件订阅（可选）** | 发信至 [developer@anylink.io](mailto:developer@anylink.io)，主题注明「订阅 IEPro 版本更新」 | 重大版本发布邮件提醒（无独立邮件列表时由技术支持人工通知） |

| 其他 |
|---|
| 技术支持：[anylink.io](https://anylink.io) / [developer@anylink.io](mailto:developer@anylink.io) |
