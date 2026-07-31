# Contributing / Maintenance Guide

[English](CONTRIBUTING.md) | [中文](CONTRIBUTING.zh-CN.md)

This repository is **public** under [Apache License 2.0](LICENSE). It publishes IE Pro 400 GlobalStandard **developer documentation** and **reference Demo source** — not device firmware or AnyLink platform software.

## Who this guide is for

| Audience | Workflow |
|---|---|
| **Readers / integrators** | `git clone` or download; no contribution required |
| **External contributors** | Fork → `feature/*` branch → Pull Request to `main` (overview: [Downloads §6](docs/en/08-downloads.md) / [中文](docs/zh-CN/08-downloads.md)) |
| **Maintainers & partners (Write access)** | Push to branches per team process; follow the conventions below |

### Pull Request scope

**Welcome via PR** (maintainer review required):

- Documentation fixes and improvements (`docs/en/`, `docs/zh-CN/`, `README.md`, `README.zh-CN.md`)
- Demo corrections and small enhancements (`demo/`)
- Image assets (`docs/assets/`)

**Not via unsolicited PR** — open a [GitHub Issue](https://github.com/anylink-dev/IEPro/issues) or email [developer@anylink.io](mailto:developer@anylink.io) first:

- Hardware specs, certifications, or firmware behavior changes
- Large structural doc rewrites or new numbered documents without prior agreement

**Direct Write access** (push without PR) is by invitation only; see [Downloads §6](docs/en/08-downloads.md).

The sections below apply to anyone opening a PR or maintaining the repo.

## Adding or Updating a Document

1. Documents live in `docs/en/` and `docs/zh-CN/`, one file per
   language, with **identical filenames** in both directories (e.g.
   `10-something-new.md` in both trees). This 1:1 mapping is what lets
   readers switch languages and lets tooling verify both trees stay in
   sync.
2. When you add a new numbered document, also add a row for it to:
   - `README.md` and `README.zh-CN.md` (the documentation index table)
   - `docs/en/08-downloads.md` and `docs/zh-CN/08-downloads.md` (the
     resource index table)
3. Use relative markdown links for any cross-document reference —
   `[Quickstart Guide](02-quickstart.md)`, not a bare mention of the
   document's name in plain text. See existing docs for the pattern.
4. Run the link checker before committing (see below).

## Keeping English and Chinese in Sync

- When you materially change one language's version of a document,
  update the other language's version in the same PR if possible. If
  that's not feasible, flag it explicitly in the PR description (e.g.
  "zh-CN not yet updated — tracked in #123") so it isn't silently
  forgotten.
- Structural changes (new sections, reordered content, new tables)
  should land in both languages together. Wording-only polish in one
  language does not require an immediate mirror update.

## Code (`/demo`)

- All source under `/demo` is **English-only** — comments, variable
  names, commit messages touching this directory, and `/demo/README.md`
  itself. Do not add a translated copy of any demo file.
- Documentation in either language should link to `/demo` rather than
  embedding full code listings. Short inline snippets for illustration
  are fine; full runnable programs belong in `/demo` only.

## Images / Screenshots

- Follow the convention in `docs/assets/README.md`: language-neutral
  images (hardware photos, wiring diagrams) go in `docs/assets/shared/`;
  UI screenshots that contain on-screen text go in `docs/assets/en/` or
  `docs/assets/zh-CN/` as appropriate.
- Name images `<doc-number>-<slug>.png` so it's clear which document
  they belong to.

## Placeholders

- Real hardware/firmware values are marked `[TBD]` (English docs) or
  `[待补充]` (Chinese docs). Do not remove a placeholder marker unless
  you're replacing it with a verified real value — do not guess or
  approximate specs.
- Before a release, search `docs/` for `[TBD]` and `[待补充]` and
  replace or resolve each marker (a checker script is **planned** but
  not yet available).

## Link Checking *(planned)*

`scripts/check_links.py` and `.github/workflows/check-links.yml` are
**planned** but not yet in the repository. They will validate internal
links, EN/ZH-CN filename parity, and placeholder counts on PRs touching
`docs/**`, `README.md`, or `README.zh-CN.md`.

Until then, manually verify relative links and keep `docs/en/` and
`docs/zh-CN/` in sync when opening a PR.
