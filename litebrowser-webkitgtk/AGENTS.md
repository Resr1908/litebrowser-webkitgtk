# AGENTS.md — Guidance for ChatGPT Codex

**Repo purpose:** Lightweight WebKitGTK browser for Ubuntu (C++17, GTK3). Keep it small; no heavy frameworks unless requested.

## Build & test

```bash
meson setup build
meson compile -C build
./tests/smoke.sh
```

- Use `pkg-config` for `gtk+-3.0` and `webkit2gtk-4.0`.
- C++ standard: **C++17**.
- Optimise for size/simplicity; prefer **no extra runtime deps**.

## Style

- Run `clang-format` (LLVM style) on all `.cpp/.hpp`.
- Avoid global state; use small helpers/static lambdas.
- Keep functions <150 LOC; split UI glue vs callbacks.

## Tasks the agent may do

- Implement tabs via `GtkNotebook` with a plus button.
- Add a download popover with progress for active downloads.
- Add content‑filter support: compile EasyList into WebKit filter store:
  - Use `webkit_user_content_filter_store_save()` to compile.
  - Save under `resources/filters/store/` with name `adblock`.
- Add a preferences dialog (JS on/off, WebGL, Do Not Track).
- Add keyboard shortcuts cheat sheet in Help menu.

## Guardrails

- Do **not** switch stack to Qt/CEF without an explicit issue.
- Do **not** add tracking/telemetry.
- Keep binary under ~20 MB stripped if possible.
