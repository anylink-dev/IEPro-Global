# IE Pro 400 Global Standard 规格书

[English](../en/01-datasheet.md) | 中文

**文档编号**：DS-IEP400-GS-001　**版本**：V1.0　**日期**：2026-07-22　**密级**：面向客户/开发者公开

---

## 1. 产品概述

IE Pro 400 Global Standard 是紫清科技 IE Pro 系列面向海外及广域物联网场景的通用工业网关，采用工业级硬件设计，预装标准 Linux 操作系统。设备出厂**不预装 AnyLink 应用及云平台**，以「硬件平台 + 标准系统接口」方式交付，支持客户基于设备自行开发、部署采集、控制及北向通信应用。

- **目标用户**：设备集成商、行业解决方案开发者
- **典型场景**：远程设备接入、工业现场数据采集（Modbus 等）、车载/移动场景、边缘数据处理、南向 DI/DO 联动控制
- **与国内版 IEPro 的差异**：
  - 不预装 AnyLink 应用及云平台，客户自行集成软件栈
  - 4G 模组：SIM7600G-H-PCIE（全球频段，见 §3.5）
  - 面向全球市场，产品型号为 Global Standard

![IE Pro 400 Global Standard](../assets/shared/01-product-overview.png)

## 2. 硬件规格

### 2.1 处理器与存储

| 项目 | 参数 |
|---|---|
| CPU | 528 MHz ARM Cortex-A7 |
| RAM | 512 MB DDR2 SDRAM |
| Flash/存储 | 4 GB NAND Flash；支持 TF 卡扩展 |
| 操作系统 | Linux（厂商预装标准发行版） |
| RTC | 内置 RTC，备用电池掉电保持 |

### 2.2 外观与结构

| 项目 | 参数 |
|---|---|
| 尺寸（长×宽×高） | 39 × 106 × 136 mm |
| 重量 | 约 507 g |
| 安装方式 | DIN 导轨卡槽固定 |
| 防护等级 | IP51 |
| 外壳材质 | 工业级工程塑料外壳 |

### 2.3 电源

| 项目 | 参数 |
|---|---|
| 输入电压范围 | 9～36 V DC（宽压输入） |
| 额定功耗 | 8 W |
| 电源接口类型 | 接线端子 |
| 防反接/浪涌保护 | 9～36 V 耐压范围；PPTC 自恢复保险丝；过流保护；雷击浪涌 ±4 kV |

### 2.4 环境参数

| 项目 | 参数 |
|---|---|
| 工作温度 | -40 ℃ ～ +85 ℃ |
| 存储温度 | 参见产品铭牌 |
| 工作湿度 | 5% ～ 95% RH（无凝露） |
| 抗震性 | 10～25 Hz（X/Y/Z 方向 2G / 30 分钟） |
| 冷却方式 | 自然风冷 |
| 电磁兼容 | 群脉冲 ±4 kV，空气放电 8 kV，符合 EN55022 |

## 3. 接口规格

### 3.1 网络接口

| 接口 | 数量 | 说明 |
|---|---|---|
| 以太网（WAN/LAN） | 2 路 | 10/100 M 自适应，RJ45；出厂默认 WAN `192.168.100.126`、LAN `192.168.101.204` |
| 4G/蜂窝网络 | 1 路 | SIM7600G-H-PCIE 模组（频段见 §3.5） |
| Wi-Fi | 不支持 | — |

### 3.2 串口

| 接口 | 数量 | 电气标准 | 说明 |
|---|---|---|---|
| RS232 | 1 路 | RS-232 | 波特率 600～256000；设备节点 `/dev/ttymxc5` |
| RS485 | 2 路 | RS-485 | 波特率 600～256000；硬件自动方向控制（无需软件切换 DE/RE）；设备节点 `/dev/ttymxc1`（RS485-1）、`/dev/ttymxc2`（RS485-2） |

### 3.3 CAN 接口

| 项目 | 参数 |
|---|---|
| 数量 | 1 路 |
| 协议标准 | CAN 2.0 |
| 波特率范围 | 5 Kbps ～ 1 Mbps |
| 系统接口名 | `can0`（出厂已安装 CAN 模块） |
| 端接电阻 | 视现场总线拓扑配置（长距离/多节点总线建议在总线两端各接 120 Ω 终端电阻） |

### 3.4 数字量 I/O

| 项目 | 参数 |
|---|---|
| DI 数量 | 1 路（X1） |
| DI 类型 | **无源输入**（干接点）；与 GND 短接为 1，断开为 0；GPIO 117 |
| DO 数量 | 1 路（Y1） |
| DO 类型 | **无源输出**（干接点开关输出）；GPIO 118 |
| 拨码开关 | 2 路（GPIO 121、GPIO 124） |

### 3.5 蜂窝网络（4G）规格

| 项目 | 参数 |
|---|---|
| 模组型号 | SIM7600G-H-PCIE（SIMCOM） |
| 制式 | GSM/GPRS/EDGE、WCDMA/UMTS/HSPA+、LTE-FDD、LTE-TDD |
| LTE-FDD 频段 | B1/B2/B3/B4/B5/B7/B8/B12/B13/B18/B19/B20/B25/B26/B28/B66 |
| LTE-TDD 频段 | B34/B38/B39/B40/B41 |
| WCDMA/UMTS/HSPA+ 频段 | B1/B2/B4/B5/B6/B8/B19 |
| GSM/GPRS/EDGE | 850/900/1800/1900 MHz |
| SIM 卡槽 | 1 路（标准 SIM 卡槽） |
| 天线接口 | 外置 4G 天线（发货含天线） |
| 模组电源控制 | GPIO 69（OUT）；**启动默认关闭**（0=关，1=开）；使用 4G 前须先上电，详见[《4G 联网示例》](03-4g-connectivity.md) §2 |
| 拨号方式 | NDIS 拨号（`AT$QCRMCALL`） |

### 3.6 其他接口

| 接口 | 说明 |
|---|---|
| USB 2.0 | 2 路 | USB 2.0 主机接口 |
| SD/TF 卡槽 | 支持 TF 卡存储扩展 |
| 调试口（Console） | 串口调试（参数见[《快速上手指南》](02-quickstart.md)） |
| 指示灯 | POWER（电源，上电即亮）；NET（GPIO 122，网络状态）、RUN（GPIO 71，运行状态）、WARN（GPIO 123，警告状态）；GPIO 指示灯：点亮=1，熄灭=0 |
| 按键 | Reset（GPIO 119）：按压=1，松开=0；出厂固件仅提供 GPIO 状态读取，不含恢复出厂等业务功能，具体逻辑由客户自行开发 |
| 硬件看门狗 | `/dev/watchdog`（Linux 标准看门狗字符设备；超时未喂狗则系统复位） |

## 4. 软件与开发能力

| 项目 | 说明 |
|---|---|
| 固件版本（本规格书对应） | V1.0.0（详见[《版本说明》](09-release-notes.md)） |
| 开放接口 | 串口（termios）、CAN（SocketCAN）、DI/DO（sysfs GPIO）、硬件看门狗（`/dev/watchdog`）、MQTT 北向、HTTP、Modbus；菜单与 CLI 见 [`demo/README.zh-CN.md`](../../demo/README.zh-CN.md)；集成说明见[《Demo 开发示例》](05-demo-guide.md) |
| 交叉编译支持 | ARM `arm-linux-gnueabihf` 工具链，详见[《交叉编译工具说明》](06-cross-compile-toolchain.md) |
| 是否预装业务平台 | **否**——设备出厂不预装 AnyLink 应用及云平台，由客户自行开发部署 |
| 支持的开发语言/SDK | C/C++（交叉编译，推荐）；设备预装 **Python 2.7.14**（本仓库 Demo 均为 C，不使用 Python） |

## 5. 认证与合规

| 项目 | 说明 |
|---|---|
| 适用型号 | IE Pro 400 Global Standard |
| CE | 证书编号 KSEM2510003048 — [下载 PDF](../certificates/eu/ce/CE-KSEM2510003048-certificate.pdf) |
| EMC | 证书编号 LCS200114008AE — [下载 PDF](../certificates/eu/emc/EMC-LCS200114008AE-certificate.pdf) |
| RoHS | 证书编号 SHA19-251135-01 — [下载 PDF](../certificates/eu/rohs/RoHS-SHA19-251135-01.pdf) |
| 无线电（RED） | 证书编号 SUES2510002159 — [下载 PDF](../certificates/eu/radio-safety/RED-SUES2510002159-certificate.pdf) |
| 完整检测报告 | 未收录于本仓库；请邮件联系 [developer@anylink.io](mailto:developer@anylink.io?subject=IEPro%E8%AE%A4%E8%AF%81%E8%AF%81%E4%B9%A6%E7%94%B3%E8%AF%B7) |
| 其他区域认证 | FCC 等按目标销售区域另行确认；4G 模组为 SIM7600G-H-PCIE |
| 出口合规 | 客户自行确认目标市场的进口与无线电合规要求 |

证书索引见 [`docs/certificates/README.md`](../certificates/README.md)。

## 6. 订购信息

| 型号 | 描述 | 备注 |
|---|---|---|
| IE Pro 400 Global Standard | 标准配置：主机 + 4G 天线 + 导轨卡扣 + 接线端子 | 不预装厂商业务平台 |

---

**发货配件**：4G 天线、DIN 导轨卡扣、接线端子。

> **量产确认**：标准配置及发货配件与 IE Pro 400 Global Standard 最终量产发货产品一致。
