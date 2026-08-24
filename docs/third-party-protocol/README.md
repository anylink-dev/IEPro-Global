# Third-party protocol package

This directory holds **language-neutral binaries** for the third-party
gateway MQTT data-access protocol: the published Word specs and the
agent + web UI deploy zip. Customers implement the same contract on
**their own platform**. English-only maintainer notes.

User-facing indexes (protocol summary and deploy steps):

- [English](../en/07-third-party-protocol.md)
- [中文](../zh-CN/07-third-party-protocol.md)

## Layout

```
docs/third-party-protocol/
├── README.md
├── AnyLink-Cloud-Third-Party-Gateway-MQTT-Data-Access-Protocol-v1.2.0.docx
├── 紫清云第三方网关MQTT数据接入协议-v1.2.0.docx
└── deploy/
    └── IEPro-deploy.zip    # agent + web UI
```

Do not add `__pycache__` or unpacked `deploy/` trees from a local install.
Keep the `.docx` filenames as published (EN ASCII / ZH title).
