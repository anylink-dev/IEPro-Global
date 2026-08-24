# IE Pro 400 Global Standard — 第三方 MQTT 协议

[English](../en/07-third-party-protocol.md) | 中文

**文档版本**：V1.0　**日期**：2026-08-24

这是一份面向第三方网关的 MQTT 数据接入协议，以及配套的部署包。Agent 是在网关上实现该协议的程序，Web 界面用于配置与下发。用途是让 IE Pro 对接**客户自己的平台**（或任何实现同一套约定的 MQTT 服务）。

设备出厂仍不预装业务应用。只有需要这条北向通道时才安装本包。

## 1. 协议说明

| 文档 | 版本 |
|---|---|
| [紫清云第三方网关 MQTT 数据接入协议](../third-party-protocol/紫清云第三方网关MQTT数据接入协议-v1.2.0.docx) | v1.2.0 |

主题、报文与鉴权以 Word 协议正文为准。平台侧按同一套约定实现 MQTT 服务即可；本页说明如何在网关上获取并安装配套的 agent 与界面。

## 2. 部署包

| 项目 | 说明 |
|---|---|
| 部署包 | [IEPro-deploy.zip](../third-party-protocol/deploy/IEPro-deploy.zip) |
| 版本 | 1.0.0 |

本包安装 agent 与 Web 界面。部署脚本**无需参数**。

### 2.1 部署

将 `IEPro-deploy.zip` 上传到网关 **`/opt`**（该目录一般已存在），然后解压并部署：

```sh
unzip /opt/IEPro-deploy.zip -d /opt && sh /opt/deploy/deploy.sh
```

看到 `IEPro deploy done` 即完成。请以 root 执行。脚本会覆盖 `/opt/agent` 和 `/opt/sdmwebs`。

网关需有 `unzip`、`python`、`sh`，根文件系统可写。

### 2.2 包内结构

```
deploy/
├── DEPLOY.md
├── deploy.sh
├── agent/
│   ├── deploy.sh
│   └── agent-iepro.zip
└── sdmwebs/
    ├── deploy.py
    ├── sdmwebs.zip
    └── boa-1.1.2.zip
```

### 2.3 部署之后

1. 在网关 LAN 侧打开 Web 界面。
2. 登录：`admin` / `admin`。
3. 在界面中配置 MQTT 相关参数，按需修改后下发。
4. 添加 Modbus 通道、设备和数据项后下发到 agent。
5. 用 watchdog 启动 agent（见下节）。

### 2.4 用 watchdog 启动 agent

部署脚本只安装文件，不会自动拉起 agent。在网关上执行：

```sh
cd /opt/agent
python watchdog.py >/dev/null 2>&1 &
```

`watchdog.py` 会启动 `agentbasic`，并在进程退出后自动拉起。

查看是否已运行：

```sh
ps | grep watchdog
ps | grep agentbasic
```

需要重启 agent 时，先停再启：

```sh
killall watchdog.py
killall agentbasic
cd /opt/agent
python watchdog.py >/dev/null 2>&1 &
```

### 2.5 安装路径

| 路径 | 内容 |
|---|---|
| `/opt/agent/` | Agent 与运行时配置 |
| `/opt/sdmwebs/web/` | Web 界面 |
| `/etc/boa/` | Boa Web 服务 |

## 3. 相关文档

- [资料下载入口](09-downloads.md) — 仓库结构与克隆方式
- [Demo 开发示例](05-demo-guide.md) — C Demo 中的 MQTT 辅助（`mqtt_mod.c`），是板级示例，不是本协议 agent
