# codex.tasklist.md — Suggested First Tasks

1. **Content Filter (EasyList)**
   - Add a small CLI (`tools/compile_filters.cpp`) that takes an Adblock list and
     compiles it to a WebKit content filter store at `resources/filters/store/` with name `adblock`.
   - Wire startup to load `adblock` if available; log to stdout when active.

2. **Tabs**
   - Wrap `WebKitWebView` in a simple tab container (GtkNotebook).
   - Middle‑click on tab to close; Ctrl+T to open a new tab with `about:blank`.

3. **Downloads UI**
   - Show a GTK Popover from a toolbar button listing active/completed downloads with progress.
   - “Open file” and “Show in folder” actions when finished.

4. **Preferences**
   - Dialog with toggles: JavaScript, WebGL, Do Not Track, Adblock enabled.
   - Persist to a small JSON in `~/.config/litebrowser/config.json`.

5. **Build & CI**
   - Add GitHub Actions workflow to build on Ubuntu LTS and run `tests/smoke.sh`.
