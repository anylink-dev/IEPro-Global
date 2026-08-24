# IE Pro 400 Global Standard — Third-Party MQTT Protocol

English | [中文](../zh-CN/07-third-party-protocol.md)

**Doc version**: V1.0　**Date**: 2026-08-24

A published MQTT data-access protocol for a third-party gateway, plus a matching deploy package. The agent is the on-gateway program that implements this protocol; the web UI is used to configure and apply settings. Use it so the IE Pro can talk to **the customer’s own platform** (or any broker that implements the same contract).

The device still ships without a pre-installed application. Install this package only when you want this northbound channel.

## 1. Protocol specification

| Document | Version |
|---|---|
| [AnyLink Cloud Third-Party Gateway MQTT Data Access Protocol](../third-party-protocol/AnyLink-Cloud-Third-Party-Gateway-MQTT-Data-Access-Protocol-v1.2.0.docx) | v1.2.0 |

Topic, payload, and authentication rules are defined in the Word file above. A platform team implements the same contract on their MQTT service; this page covers how to obtain and install the agent and UI on the gateway.

## 2. Deploy package

| Item | Details |
|---|---|
| Package | [IEPro-deploy.zip](../third-party-protocol/deploy/IEPro-deploy.zip) |
| Version | 1.0.0 |

The zip installs the agent and the web UI. The deploy script takes **no arguments**.

### 2.1 Deploy

Upload `IEPro-deploy.zip` to **`/opt`** on the gateway (`/opt` is normally already present), then unzip and deploy:

```sh
unzip /opt/IEPro-deploy.zip -d /opt && sh /opt/deploy/deploy.sh
```

Done when the console prints `IEPro deploy done`. Run as root. Existing `/opt/agent` and `/opt/sdmwebs` are replaced.

The gateway needs `unzip`, `python`, `sh`, and a writable root filesystem.

### 2.2 Package layout

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

### 2.3 After deploy

1. Open the web UI from a PC on the gateway LAN.
2. Sign in: `admin` / `admin`.
3. Configure MQTT parameters in the UI, then apply them.
4. Add Modbus channels, devices, and data items, then apply to the agent.
5. Start the agent with watchdog (see below).

### 2.4 Start the agent with watchdog

The deploy script installs files only; it does not start the agent. On the gateway:

```sh
cd /opt/agent
python watchdog.py >/dev/null 2>&1 &
```

`watchdog.py` starts `agentbasic` and restarts it if the process exits.

Check that it is running:

```sh
ps | grep watchdog
ps | grep agentbasic
```

To restart the agent, stop then start:

```sh
killall watchdog.py
killall agentbasic
cd /opt/agent
python watchdog.py >/dev/null 2>&1 &
```

### 2.5 Installed paths

| Path | Content |
|---|---|
| `/opt/agent/` | Agent and runtime config |
| `/opt/sdmwebs/web/` | Web UI |
| `/etc/boa/` | Boa web server |

## 3. Related documents

- [Downloads](09-downloads.md) — repository layout and how to clone
- [Demo Development Guide](05-demo-guide.md) — C demo MQTT helper (`mqtt_mod.c`); a board-level example, not this protocol agent
