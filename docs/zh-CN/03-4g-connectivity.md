# IE Pro 400 Global Standard — 4G 联网示例

[English](../en/03-4g-connectivity.md) | 中文

**文档版本**：V1.0　**日期**：2026-07-22

## 1. 插 SIM 卡

1. 设备**断电**。
2. 打开 SIM 卡槽（位置见[规格书](01-datasheet.md) §3.5）。
3. 按卡槽丝印方向插入标准 SIM 卡，金属触点朝向卡槽内侧弹片。
4. 推入卡槽至卡扣锁定，重新上电。
5. 等待约 60 秒，须先完成第 2 节模组电源上电，再通过 AT 指令确认卡已识别（见第 3、4 节）。

> **注意**：不建议热插拔 SIM 卡，可能导致模组无法正确识别。

## 2. 开启 4G 模组电源

IE Pro 400 Global Standard 通过 **GPIO 69**（输出）控制 4G 模组电源。**网关启动后默认为关闭**（模组无电）。进行 AT 指令、拨号或 Demo 蜂窝功能前，须先将 GPIO 69 置为 **1** 上电；置 **0** 可关闭电源。

### 2.1 sysfs 操作示例

```bash
# 导出 GPIO（若尚未导出）
echo 69 > /sys/class/gpio/export

# 设为输出
echo out > /sys/class/gpio/gpio69/direction

# 上电（1=开，0=关）
echo 1 > /sys/class/gpio/gpio69/value

# 等待 AT 口出现（上电后 USB 枚举需要时间，最长约 10 秒）
for i in $(seq 1 20); do
  [ -e /dev/ttyUSB2 ] && break
  sleep 0.5
done
ls -l /dev/ttyUSB2
```

关闭电源时将 `value` 写 `0` 即可。

### 2.2 Demo 自动上电

`iepro_demo` 蜂窝模块在首次打开 AT 口时会自动执行 GPIO 69 上电，并轮询等待 `/dev/ttyUSB2` 出现（超时 10 秒，见 `demo/src/modules/cellular_mod.c`）。

## 3. APN 设置与拨号

IE Pro 400 Global Standard 采用 SIMCOM **SIM7600G-H-PCIE** 模组，通过 **NDIS 拨号**（`AT$QCRMCALL`）建立数据连接。

### 3.1 常用运营商 APN 示例

| 运营商 | APN | 拨号命令示例 |
|---|---|---|
| 中国移动（公网） | `cmiot` | `AT$QCRMCALL=1,1,,,,,"cmiot","none","none",3` |
| 中国电信（公网 LTE） | `ctlte` | `AT$QCRMCALL=1,1,,,,,"ctlte","none","none",3` |
| 中国联通（公网 3GPP） | 留空（自动） | `AT$QCRMCALL=1,1` |
| 专网卡 | 由运营商提供 | `AT$QCRMCALL=1,1,,,,,"<apn>","<user>","<pass>",3` |

> 物联网卡通常需要专用 APN，请向运营商确认。用户名/密码区分大小写。

### 3.2 拨号前检查流程

通过 AT 调试口连接模组后，按以下顺序检查（须先完成 §2 电源上电）：

```
AT+CFUN=0          # 重启模组（可选）
AT+CFUN=1
AT+CPIN?           # 查询 SIM 卡，正常返回 +CPIN: READY
AT+CSQ             # 查询信号强度
AT+CNSMOD=1        # 开启网络制式自动上报
AT+CNSMOD?         # 查询当前网络制式（8=LTE）
AT+CEREG?          # LTE 模式下查询注册状态（返回 0,1 或 0,5 表示可用）
AT+CGREG?          # 非 LTE 模式下查询注册状态
```

### 3.3 发起拨号

**公网 3GPP 模式（GSM/WCDMA/LTE）**：

```
AT$QCRMCALL=1,1
```

**中国移动示例**：

```
AT$QCRMCALL=1,1,,,,,"cmiot","none","none",3
```

**中国电信示例**：

```
AT$QCRMCALL=1,1,,,,,"ctlte","none","none",3
```

**挂断拨号**：

```
AT$QCRMCALL=0,1
```

**查询拨号状态**：

```
AT$QCRMCALL?
```

## 4. 联网状态查看

### 4.1 AT 指令自检

| 指令 | 关键返回值 | 含义 |
|---|---|---|
| `AT+CPIN?` | `READY` | SIM 卡正常 |
| `AT+CPIN?` | `SIM PIN` / `SIM PUK` | 卡被锁，需输入 PIN/PUK |
| `AT+CSQ` | 0–31（非 99） | 有信号；数值越大信号越好 |
| `AT+CSQ` | `99,99` | 无信号 |
| `AT+CEREG?` | `0,1` 或 `0,5` | LTE 网络已注册，可进行数据业务 |
| `AT+CGREG?` | `0,1` 或 `0,5` | 非 LTE 网络已注册 |
| `AT+CNSMOD?` | 第二位为 `8` | 当前注册在 LTE 网络 |

> **注意**：LTE 模式下请用 `AT+CEREG?` 判断数据业务是否可用；非 LTE 模式请用 `AT+CGREG?`。

### 4.2 系统层面验证

拨号成功后，在设备 SSH 会话中执行：

```bash
# 查看网络接口
ip addr show

# 测试出网
ping -c 4 8.8.8.8
```

## 5. Ping 测试

```bash
ping -c 4 8.8.8.8
```

正常结果示例：

```
4 packets transmitted, 4 received, 0% packet loss
```

若出现丢包或无法解析域名，请参见[《常见问题 FAQ》](07-faq.md)中"无法联网"相关章节。

## 6. 参考文档

- SIMCOM 官方文档：`SIM7500_SIM7600 Linux NDIS 拨号流程`（模组 AT 指令完整说明）

---

**模组信息**：SIM7600G-H-PCIE；支持 GSM/GPRS/EDGE、WCDMA/UMTS/HSPA+、LTE-FDD、LTE-TDD。频段详见[规格书](01-datasheet.md) §3.5。
