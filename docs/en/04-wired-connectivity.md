# IE Pro 400 Global Standard — Wired Connectivity Example

English | [中文](../zh-CN/04-wired-connectivity.md)

**Doc version**: V1.0　**Date**: 2026-07-22

## 1. Ethernet Connection

1. The device provides **2 Ethernet ports** (WAN + LAN), both 10/100 M auto-negotiation RJ45.
2. Connect with a standard Ethernet cable (Cat5e or better):
   - **LAN port**: to a PC or switch on the same LAN for local management or field device connectivity.
   - **WAN port**: to an upstream router/switch for Internet access.
3. Check the link LED after connecting: steady on or blinking means link is up; off means no connection or cable fault.

## 2. IP Configuration

### 2.1 Factory Default Static IPs

The device ships with static IPs on both WAN and LAN — no DHCP required for direct access:

| Port | Default IP | Subnet mask |
|---|---|---|
| WAN | `192.168.100.126` | `255.255.255.0` |
| LAN | `192.168.101.204` | `255.255.255.0` |

For SSH management via the LAN port, set your PC to the same subnet (e.g. `192.168.101.100/24`), then:

```bash
ssh root@192.168.101.204
```

Account is `root`; the initial password is unique per device (generated from the product unique code). See [Quickstart Guide](02-quickstart.md) §5.2.

After login, check current interface configuration:

```bash
ip addr show
```

### 2.2 Change IP Configuration

After SSH login, use standard Linux commands to modify IPs temporarily or permanently:

```bash
# list interfaces
ip addr show

# temporary LAN IP change (lost on reboot — debugging only)
ip addr flush dev eth1
ip addr add 192.168.101.100/24 dev eth1
ip link set eth1 up
```

For persistent changes, edit the system network config file (path depends on firmware — commonly `/etc/network/interfaces` or `/etc/config/network`).

### 2.3 Custom Static IP Example

| Field | Example |
|---|---|
| IP address | 192.168.101.100 |
| Subnet mask | 255.255.255.0 |
| Gateway | 192.168.101.1 |
| DNS | 8.8.8.8 / 114.114.114.114 |

## 3. Ping Test

### 3.1 LAN Connectivity

```bash
# ping device LAN port from PC (factory default)
ping -c 4 192.168.101.204
```

### 3.2 Internet Connectivity

```bash
ping -c 4 8.8.8.8
```

### 3.3 DNS Resolution

```bash
ping -c 4 www.example.com
```

Expect 0% packet loss. If the public IP pings but hostnames fail, check DNS settings in §2.2.

## 4. Common Scenarios

| Scenario | Recommendation |
|---|---|
| Enterprise LAN | Use static IP + internal DNS; avoid subnet conflicts |
| Direct PC connection | Set PC to `192.168.101.x`, SSH to `192.168.101.204` |
| Wired + 4G dual link | Wired preferred for egress, 4G as backup; routing policy configured by customer |

## 5. Serial Port Test (Optional)

Serial device nodes for connecting field devices (e.g. Modbus):

| Interface | Device node |
|---|---|
| RS485-1 | `/dev/ttymxc1` |
| RS485-2 | `/dev/ttymxc2` |
| RS232-1 | `/dev/ttymxc5` |

Test RS485-1 at 9600 baud:

```bash
microcom -s 9600 /dev/ttymxc1
```

---

**Ethernet spec**: 2 × 10/100 M auto-negotiation (WAN + LAN). See [Datasheet](01-datasheet.md) §3.1.
