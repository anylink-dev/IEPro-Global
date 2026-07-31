# Image Assets Convention

This directory holds images referenced by the documentation in `../en`
and `../zh-CN`. English-only maintainer notes; the images themselves
may of course show Chinese UI text where relevant.

## Layout

```
docs/assets/
├── shared/     # Language-neutral images: hardware photos, port/pin
│               # diagrams, wiring diagrams — anything without UI text
├── en/         # Screenshots of the English device UI
└── zh-CN/      # Screenshots of the Chinese device UI
```

Use `shared/` whenever possible — most hardware photos and interface
diagrams don't need a separate version per language. Only put an image
in `en/` or `zh-CN/` if it actually contains language-specific UI text
(e.g. a screenshot of the web management console).

## Naming Convention

Match the doc number and a short slug, so it's obvious which doc an
image belongs to:

```
<doc-number>-<slug>.png
```

Examples:

- `shared/02-front-panel-photo.png` / `02-front-panel-diagram.png` — front panel (referenced from `02-quickstart.md`)
- `shared/02-terminal-block-photo.png` / `02-terminal-block-diagram.png` — terminal block (referenced from `02-quickstart.md`)
- `en/02-web-login.png` / `zh-CN/02-web-login.png` — the same login
  screen, captured once per UI language

## Referencing Images from Docs

```markdown
![Interface panel](../assets/shared/02-interface-panel.png)
![Login screen](../assets/en/02-web-login.png)
```

## Status

Product photos and interface diagrams are published under `shared/` (see `01-datasheet.md` §1 and `02-quickstart.md` §3.1).
