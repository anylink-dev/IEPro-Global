# 维护与贡献指南

[English](CONTRIBUTING.md) | [中文](CONTRIBUTING.zh-CN.md)

本仓库在 [Apache License 2.0](LICENSE) 下**公开**，发布 IE Pro 400 Global Standard **开发者文档**与**参考 Demo 源码**——不包含设备固件或 AnyLink 平台软件。

## 适用对象

| 对象 | 方式 |
|---|---|
| **读者 / 集成开发者** | `git clone` 或下载使用，无需贡献 |
| **外部贡献者** | Fork → `feature/*` 分支 → 向 `main` 发起 Pull Request（流程概览见[《资料下载入口》§6](docs/zh-CN/08-downloads.md) / [English](docs/en/08-downloads.md)） |
| **维护者与合作伙伴（Write 权限）** | 按团队流程直接推送分支；遵循下文约定 |

### Pull Request 范围

**欢迎通过 PR 提交**（需 Maintainer 审核）：

- 文档修正与改进（`docs/en/`、`docs/zh-CN/`、`README.md`、`README.zh-CN.md`）
- Demo 修正与小改进（`demo/`）
- 图片资源（`docs/assets/`）

**请勿未经沟通直接提 PR**——请先开 [GitHub Issue](https://github.com/anylink-dev/IEPro-Global/issues) 或邮件联系 [developer@anylink.io](mailto:developer@anylink.io)：

- 硬件规格、认证信息或固件行为变更
- 大规模文档重构或新增编号文档（未经事先约定）

**直接 Write 权限**（免 PR 推送）仅受邀开放；见[《资料下载入口》§6](docs/zh-CN/08-downloads.md)。

下文约定适用于所有提交 PR 或日常维护仓库的人员。

## 新增/修改文档

1. 文档分别存放于 `docs/en/` 与 `docs/zh-CN/`，两个目录下**文件名必须完全一致**（例如新增 `10-something-new.md` 时两边都要建）。这种一一对应关系是语言切换和后续工具校验同步性的基础。
2. 新增编号文档时，同步在以下位置补充索引行：
   - `README.md` 与 `README.zh-CN.md`（文档索引表）
   - `docs/en/08-downloads.md` 与 `docs/zh-CN/08-downloads.md`（资料清单表）
3. 文档间的相互引用统一使用相对路径超链接，如 `[快速上手指南](02-quickstart.md)`，不要用纯文字提及文档名。参考现有文档的写法。
4. 提交前运行链接校验脚本（见下文）。

## 保持中英文同步

- 对某一语言文档做出实质性修改时，尽量在同一 PR 中同步更新另一语言版本；如暂时无法同步，请在 PR 描述中明确标注（如"zh-CN 待更新，见 #123"），避免遗漏。
- 结构性变更（新增章节、调整顺序、新增表格）应两种语言同步落地；仅措辞润色则不强制立即同步。

## 代码（`/demo`）

- `/demo` 目录下所有源码均为**纯英文**——注释、变量名、涉及该目录的提交说明、以及 `/demo/README.md` 本身。不要为任何 Demo 文件添加翻译副本。
- 中英文文档都应链接到 `/demo`，而不是内嵌完整代码。简短的示意性代码片段可以内嵌，但完整可运行程序只保留在 `/demo` 一处。

## 图片/截图

- 遵循 `docs/assets/README.md` 中的约定：语言无关的图片（硬件照片、接线图）放入 `docs/assets/shared/`；含界面文字的截图按语言放入 `docs/assets/en/` 或 `docs/assets/zh-CN/`。
- 图片命名为 `<文档编号>-<简短描述>.png`，便于识别归属文档。

## 占位符

- 真实的硬件/固件数据用 `[TBD]`（英文文档）或 `[待补充]`（中文文档）标注。不要在没有经过验证的真实数据时删除占位符，更不要凭猜测填入近似值。
- 发布前请在 `docs/` 中搜索 `[TBD]` 与 `[待补充]` 并逐项处理（校验脚本**计划中**，尚未提供）。

## 链接校验（计划中）

`scripts/check_links.py` 与 `.github/workflows/check-links.yml` **尚在计划中**，尚未入库。落地后将用于校验内部链接、`docs/en/` 与 `docs/zh-CN/` 文件名对应关系，以及占位符统计（针对涉及 `docs/**`、`README.md`、`README.zh-CN.md` 的 PR）。

在此之前，提交 PR 时请人工检查相对链接，并保持中英文文档同步。
