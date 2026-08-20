# IE Pro 400 Global Standard — 版本说明

[English](../en/09-release-notes.md) | 中文

**文档版本**：V1.1　**日期**：2026-08-19

## 1. 适用型号

| 型号 | 备注 |
|---|---|
| IE Pro 400 Global Standard | 不预装 AnyLink 应用及云平台 |

## 2. 固件版本历史

| 版本号 | 发布日期 | 适用型号 | 主要变更 | 备注 |
|---|---|---|---|---|
| V1.0.0 | 2026-07-22 | IE Pro 400 Global Standard | 首个开发者版本发布 | 预装标准 Linux，开放串口/CAN/GPIO 接口 |

## 3. Demo 示例代码版本历史

| 版本号 | 发布日期 | 对应固件版本 | 对应文档版本 | 变更内容 |
|---|---|---|---|---|
| V1.0 | 2026-07-22 | V1.0.0 | V1.0 | 初始发布：统一控制台 `iepro_demo`（`demo/src/`）；子模块 Serial、CAN、GPIO、Cellular（SIM7600G-H-PCIE / `AT$QCRMCALL`）、MQTT（可选 `WITH_MQTT=1`）；设备节点与 GPIO 编号已对齐实测硬件 |
| V1.1 | 2026-08-19 | V1.0.0 | V1.1 | 新增 HTTP（libcurl）、Modbus RTU/TCP 及对应 CLI 子命令；统一 `demo/deps` prebuilt 布局（v0.2）；MQTT/HTTP/Modbus/curl/OpenSSL 默认链接；MQTT 发布消息体留空时默认发送 [`metrics.c`](../../demo/src/common/metrics.c) 示例 JSON；CLI 启动时初始化板载 GPIO；4G 模组电源 GPIO 69（OUT，启动默认关，Demo 自动上电并等待 `/dev/ttyUSB2`）；硬件看门狗模块 `wdt_mod.c`（`/dev/watchdog`，移植自 `hardwareWDT.py`）；操作说明迁至 [`demo/README.md`](../../demo/README.md) / [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md)；05-demo-guide 精简为集成说明（`DEMO_VERSION` 仍为 `1.0.0`） |

源码内版本号见 `demo/src/main.c` 中的 `DEMO_VERSION`（与上表 V1.0 对应，当前为 `1.0.0`）。

## 4. 交叉编译工具链版本历史

| 版本号 | 发布日期 | 对应固件版本 | GCC 版本 | 变更内容 |
|---|---|---|---|---|
| gcc-linaro-5.5.0-2017.10 | 2017-10 | V1.0.0 | GCC 5.5.0 | 初始工具链，`arm-linux-gnueabihf` |

## 5. 文档版本历史（本系列开发者资料）

| 文档 | 版本 | 日期 | 变更说明 |
|---|---|---|---|
| 全部文档（中英文） | V1.0 | 2026-07-22 | 基于产品规格书、使用手册、系统接口说明、交叉编译介绍、SIMCOM 拨号流程等内部资料正式填充 |
| 05 Demo 指南（中英文）、`demo/README.md`、`demo/README.zh-CN.md` | V1.1 | 2026-08-19 | HTTP/Modbus/CLI；MQTT 默认 metrics 发布；硬件看门狗 `wdt_mod.c`；操作与集成文档拆分；deps prebuilt v0.2 |
| `08-downloads`（中英文） | V1.1 | 2026-08-19 | 页眉与仓库目录结构与 V1.1 对齐 |

## 6. 版本兼容性说明

- Demo **V1.1**（仓库更新）兼容固件 **V1.0.0** 及以上；源码中 `DEMO_VERSION` 暂保持 `1.0.0`，待下次与固件对齐的 Demo 发布时再递增。
- Demo 示例 **V1.0** 与开发者文档 **V1.0**、固件 **V1.0.0** 同步发布，兼容固件 V1.0.0 及以上版本。
- 交叉编译工具链 `gcc-linaro-5.5.0-2017.10`（`arm-linux-gnueabihf`）与固件 V1.0.0 匹配。
- 低于 V1.0.0 的固件可能因驱动接口差异导致 Demo 编译或运行异常。

## 7. 已知问题

| 版本 | 问题描述 | 影响范围 | 计划修复版本 |
|---|---|---|---|
| — | V1.0.0 暂无已知问题 | — | — |

## 8. 量产发货一致性确认

以下信息与 **IE Pro 400 Global Standard** 最终量产发货产品一致：

| 项目 | 确认内容 |
|---|---|
| 随箱清单 | 见[《快速上手指南》](02-quickstart.md) §1 |
| 出厂固件 | **V1.0.0**（2026-07-22） |
| 公开证书 | CE / EMC / RoHS / RED，适用型号 IE Pro 400 Global Standard；见[《产品认证证书》](certificates.md) |
