# LiteBrowser (WebKitGTK) — Starter

A **lean but capable** Ubuntu browser scaffold using **C++17 + GTK3 + WebKit2GTK**.

- Multi‑process rendering (WebKit2).
- Hardware acceleration, JS, WebGL enabled by default.
- Simple UI: Back / Forward / Reload / URL bar.
- Basic pop‑up suppression (open in same view).
- Download handling to `~/Downloads`.
- Optional content filter hook (Adblock‑style compiled filter if present).

> This is a good base if you want “small but serious”. If you later need Chromium‑level features (Widevine, full PWAs), consider QtWebEngine/CEF.

---

## Quick start (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential meson ninja-build pkg-config libgtk-3-dev libwebkit2gtk-4.0-dev clang-format
```

Build & run:

```bash
meson setup build
meson compile -C build
./build/litebrowser https://example.com
```

### Dev helpers

Format C++:

```bash
clang-format -i src/*.cpp
```

Run the app with a blank page:

```bash
./build/litebrowser
```

---

## Controls & tips

- **Address bar**: Type a URL and press **Enter**. Bare domains are prefixed with `https://`.
- **Keys**:  
  - `Alt+Left` / `Alt+Right` — Back/Forward  
  - `Ctrl+R` — Reload  
  - `Ctrl+L` — Focus URL bar
- **Downloads**: Saved to `~/Downloads` (overridable via `LITEBROWSER_DOWNLOAD_DIR` env var).
- **Content filter (optional)**: If a compiled WebKit content filter named `"adblock"` exists in `resources/filters/store/`, it will be loaded at startup. See `AGENTS.md` for how Codex can compile one from EasyList.

---

## Project layout

```
.
├── AGENTS.md
├── meson.build
├── README.md
├── resources/
│   └── filters/
│       └── README.txt
├── scripts/
│   └── dev_setup_ubuntu.sh
├── src/
│   ├── main.cpp
│   └── meson.build
├── tests/
│   └── smoke.sh
└── .clang-format
```

---

## Roadmap (suggested)
1. Tabs (GtkNotebook or simple Stack).
2. Downloads UI (progress bar) + “Show in folder”.
3. Settings pane (content blocking, JS, privacy).
4. Content filters: compile EasyList to WebKit filter store.
5. Profile management (ephemeral vs persistent).
6. GTK4 migration once dependencies are settled on your target Ubuntu.

See **`codex.tasklist.md`** for ready‑to‑run Codex tasks.
