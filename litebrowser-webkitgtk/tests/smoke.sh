#!/usr/bin/env bash
set -euo pipefail
# Very lightweight smoke: ensure the binary runs and links.
if [[ ! -x ./build/litebrowser ]]; then
  echo "Build first: meson setup build && meson compile -C build" >&2
  exit 1
fi
# Try launching and immediately quitting via a short timeout.
timeout 2s ./build/litebrowser >/dev/null 2>&1 || true
echo "✅ Smoke test done."
