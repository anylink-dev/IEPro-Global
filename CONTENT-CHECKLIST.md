# 内容待补充清单 — IE Pro 400 Global Standard 开发者资料

**用途**：本文件面向内部团队，汇总当前仓库中所有 `[待补充]/[TBD]` 占位符，按主题归类为可执行的待办事项，方便分工认领。不属于对外发布的9份文档之一。

**当前状态**：`docs/` 正文已无 `[待补充]`/`[TBD]` 占位符。修改意见第 1–4、10 条已落地；第 5–9 条按决定跳过。MQTT 占位配置（`mqtt_mod.c`）待后续示例补齐时再改。

**使用建议**：
- 按"谁最清楚这块数据"分工：硬件规格 → 硬件/结构工程师；联网与固件行为 → 固件/驱动工程师；工具链 → SDK/构建负责人；下载入口与权限 → IT/DevOps；LICENSE → 法务。
- 每完成一处，请**同步更新中英文两个版本**（`docs/en/*` 与 `docs/zh-CN/*` 文件名一一对应，具体流程见 [CONTRIBUTING.zh-CN.md](CONTRIBUTING.zh-CN.md)）。
- 发布前在 `docs/` 中搜索 `[TBD]`/`[待补充]` 确认无残留；`scripts/check_links.py` 与 CI 校验**计划中**，落地后启用。

---

## 1. 硬件规格（`docs/*/01-datasheet.md`）

- [x] 文档编号 → DS-IEP400-GS-001 / V1.0 / 2026-07-22
- [x] 产品概述：典型应用场景、与国内版 IEPro 的差异（频段/预装软件/认证标准）
- [x] 处理器与存储：CPU 型号/主频/架构、RAM、Flash 容量、OS/内核版本 → 内核具体版本号未列出
- [x] 外观与结构：尺寸、重量、安装方式、防护等级（IP等级）、外壳材质
- [x] 电源：输入电压范围、额定功耗、接口类型、防反接/浪涌保护
- [x] 环境参数：工作/存储温度范围、工作湿度、抗震防尘等级 → 存储温度写「参见铭牌」、未单独列防尘等级
- [x] 网络接口：以太网口数量与速率、4G 模组型号、是否支持 Wi-Fi
- [x] 串口：RS232/RS485 数量、电气标准、波特率范围、是否支持自动方向控制 → RS485 硬件自动方向控制
- [x] CAN 接口：数量、协议标准（CAN2.0A/B等）、波特率范围、端接电阻是否内置/可拨码 → 1 路 CAN 2.0，出厂已安装模块；端接电阻为现场外接方案，非内置/拨码
- [x] 数字量 I/O：DI/DO 数量、电气类型（干接点/湿接点/继电器/晶体管）、电压与负载范围 → X1 无源输入、Y1 无源输出（干接点）
- [x] 4G 规格：模组型号、制式（LTE FDD/TDD）、**支持频段需覆盖全部目标销售区域**、SIM 卡槽数量与类型、天线接口类型、模组电源 GPIO 69（OUT，启动默认关）→ SIM7600G-H-PCIE；LTE-FDD/TDD、WCDMA、GSM 频段见规格书 §3.5
- [x] 其他接口：USB、SD/TF 卡槽、Console 调试口、指示灯含义对照表、按键功能（含 Reset）、硬件看门狗 `/dev/watchdog` → USB 2.0 × 2 路；看门狗见规格书 §3.6、05-demo-guide §3.8
- [x] 软件能力：对应固件版本号、支持的开发语言/SDK
- [x] 认证合规：安规认证（CE/FCC/RoHS等）、无线电认证（按销售目标国家/地区）、出口合规说明 → CE/EMC/RoHS/RED 合格证见 `docs/certificates/`；完整报告联系 developer@anylink.io
- [x] 订购信息：标准配置说明

## 2. 快速上手指南（`docs/*/02-quickstart.md`）

- [x] 开箱清单实际数量：电源适配器/端子、4G 天线根数、导轨安装配件
- [x] 技术支持联系方式（用于清单不符时联系）→ anylink.io
- [x] 电源电压范围、接线极性说明 → 9～36 V DC
- [x] 指示灯正常/异常状态含义 → POWER（电源，上电即亮）；RUN / NET / WARN（GPIO 71/122/123），与前面板丝印一致
- [x] 设备启动耗时 → 60～90 秒
- [x] 设备接口面板示意图/照片 → `docs/assets/shared/02-front-panel-*.png`、`02-terminal-block-*.png`（见 `02-quickstart` §3.1）
- [x] SIM 卡插拔工具、插入方向（金属触点朝向）→ §4 插卡
- [x] LAN 口编号、设备默认管理 IP → WAN `192.168.100.126`、LAN `192.168.101.204`
- [x] Web 管理界面默认账号密码 → 不适用，设备无 Web 管理界面
- [x] Console 口线材规格、波特率、默认账号密码 → USB 转 TTL / 115200 / `root`（一机一密）
- [x] Web 管理界面菜单路径（如"网络状态"具体位置）→ 不适用，设备无 Web 管理界面

## 3. 4G 联网示例（`docs/*/03-4g-connectivity.md`）

- [x] SIM 卡插入方向（与第2项保持一致）
- [x] 4G 模组电源控制（GPIO 69 OUT；启动默认关；1=开 0=关）→ §2；规格书 §3.5
- [x] APN 设置菜单路径 → 通过 AT 指令 / Demo Cellular 菜单 13（无 Web UI）
- [x] 是否有内置常用运营商 APN 预设示例 → §3.1 运营商 APN 表
- [x] 拨号配置文件实际路径（若不是标准 mmcli/ModemManager 方案）→ NDIS `AT$QCRMCALL`，AT 口 `/dev/ttyUSB2`
- [x] 联网状态查看菜单路径 → §4 AT 自检 + `ip addr` / `ping`
- [x] 双 SIM 卡切换 / 有线-4G 双备份配置说明（如支持，需要单独文档或章节链接）→ 硬件仅单 SIM；有线+4G 备份见 `04-wired-connectivity.md` §4

## 4. 有线联网示例（`docs/*/04-wired-connectivity.md`）

- [x] DHCP/静态 IP 配置菜单路径 → SSH 下 `ip addr` 临时修改及配置文件说明（§2.2）
- [x] 建议的备用 DNS 地址 → 8.8.8.8 / 114.114.114.114

## 5. Demo 开发示例（`docs/*/05-demo-guide.md` + `demo/` 源码）

- [x] 目标系统内核版本、libc 类型 → Linux / ARM Cortex-A7 / `arm-linux-gnueabihf` / glibc（见 `05-demo-guide.md` §0）
- [x] 是否支持 Python 等脚本语言运行时 → **已确认：设备预装 Python 2.7.14；本仓库 Demo 全部为 C**
- [x] `docs/*/05-demo-guide.md` 与当前 Demo 同步（主菜单含 Cellular、无 Collect；源码路径 `demo/src/`）
- [ ] 逐一确认并替换以下代码中的占位配置：
  - [x] `demo/src/modules/serial_mod.c`（设备节点路径）
  - [x] `demo/src/modules/can_mod.c`、`demo/scripts/can_setup.sh`（接口名称）
  - [x] `demo/src/modules/gpio_mod.c`（GPIO 编号）
  - [x] `demo/src/modules/cellular_mod.c`（GPIO 69 模组电源、`/dev/ttyUSB2`、NDIS 拨号、`wwan0`）
  - [x] `demo/src/modules/wdt_mod.c`（`/dev/watchdog` 喂狗 start/stop/reboot）
  - [ ] `demo/src/modules/mqtt_mod.c`（`MQTT_DEFAULT_BROKER`、`MQTT_DEFAULT_CLIENT_ID` 等；可通过菜单/CLI 配置，部署前改为实际值）

## 6. 交叉编译工具说明（`docs/*/06-cross-compile-toolchain.md`）

- [x] 目标 CPU 架构、libc 版本、交叉编译前缀（`CROSS_COMPILE`）→ ARM Cortex-A7 / glibc / `arm-linux-gnueabihf-`
- [x] 目标内核版本、参考 SDK 版本 → Linux（厂商预装）/ `gcc-linaro-5.5.0-2017.10`
- [x] 工具链下载地址（含镜像与 `scripts/setup_toolchain.sh`）
- [x] 工具链 SHA256 校验值 → 不提供；通过在线镜像或 `setup_toolchain.sh` 下载
- [x] 支持的主机操作系统版本、磁盘空间需求 → Ubuntu 18.04/20.04/22.04 x86_64，≥ 2 GB
- [x] 设备是否预置 gdbserver（用于远程调试）→ 未预置
- [x] 设备日志路径（当前文档仅泛述 `dmesg` / `journalctl`，未指定固定路径）→ 无固定应用日志路径，由客户自行定义
- [x] 底层驱动定制开发的商务/技术支持申请渠道 → anylink.io

## 7. 第三方 MQTT 协议（`docs/*/07-third-party-protocol.md`）

- [x] 协议 Word 与部署包入口 → 见 `docs/en/07-third-party-protocol.md` / `docs/zh-CN/07-third-party-protocol.md`
- [x] 语言无关资源 → `docs/third-party-protocol/`（Word + `IEPro-deploy.zip`）

## 8. 常见问题 FAQ（`docs/*/08-faq.md`）

- [x] SIM 卡插入方向（与第2、3项保持一致）
- [x] 技术支持联系方式 → anylink.io
- [x] Web 管理界面端口号（http/https）→ 不适用，设备无 Web 管理界面
- [x] Reset 按键说明（GPIO 119 状态读取；出厂固件不含长按恢复出厂）→ 见规格书 §3.6、FAQ §6
- [x] 技术支持邮箱 → developer@anylink.io（无独立工单/论坛系统，官网 anylink.io）

## 9. 资料下载入口（`docs/*/09-downloads.md`）

- [x] 开发者门户/官网统一入口 URL → 仓库 `README.zh-CN.md` / anylink.io
- [x] GitHub 仓库地址、访问方式（公开 / 需申请账号）→ https://github.com/anylink-dev/IEPro-Global.git（公开仓库）
- [x] 分支/Tag 命名规范（与固件版本的对应关系）→ §5 GitHub Flow + SemVer（`v1.0.0` ↔ 固件 `V1.0.0`）
- [x] 交叉编译工具链安装包下载地址 → §4 镜像链接 + `setup_toolchain.sh`
- [x] 访问权限申请流程：申请渠道、审批周期、权限级别说明 → §6 只读公开 / PR 贡献 / Write 邮件申请（3～5 工作日）
- [x] 更新订阅渠道：GitHub Watch/Release 通知、邮件列表 → §7 Watch Releases、RSS、邮件 opt-in

## 10. 版本说明（`docs/*/10-release-notes.md`）

- [x] 是否存在子型号/区域版本差异 → IE Pro 400 Global Standard
- [x] 首个固件版本号、发布日期、主要变更说明 → V1.0.0 / 2026-07-22
- [x] Demo 源码版本历史 → V1.0（2026-07-22）初始发布；V1.1（2026-08-19）HTTP/Modbus/CLI + deps v0.2；`DEMO_VERSION` 仍为 `1.0.0`
- [x] 交叉编译工具链版本历史（含对应 GCC 版本）
- [x] 版本兼容性说明（如 Demo vX.Y 与固件 vA.B 的对应关系）
- [x] 已知问题列表 → V1.0.0 暂无已知问题

## 11. 截图与示意图（`docs/assets/`）

- [x] 设备接口面板照片 → `docs/assets/shared/02-front-panel-*.png`、`02-terminal-block-*.png`
- [x] Web 登录/管理界面截图 → 不适用，设备无 Web 管理界面
- [x] 文档配图 → `01-product-overview.png`（规格书 §1）、`02-quickstart` §3.1 接口图已引用

## 12. 法务与合规

- [x] `LICENSE` 具体条款（待法务确定适用协议）→ Apache License 2.0（Copyright 2026 AnyLink）
- [x] 各文档"密级/Classification"字段是否准确、是否需要额外的保密分级 → 全部为「面向客户/开发者公开 / Public」，与 Apache 2.0 公开仓库一致

## 13. 量产发货一致性（修改意见第 10 条）

- [x] 随箱清单与最终发货产品一致 → 见 `02-quickstart.md` §1（已加量产确认说明）
- [x] 出厂固件版本 V1.0.0 与最终发货产品一致 → 见 `10-release-notes.md` §8
- [x] 公开证书（CE/EMC/RoHS/RED）适用于最终发货产品 → 见 `docs/en/certificates.md` / `docs/zh-CN/certificates.md`

---

*最后更新：2026-08-13。*
