#!/usr/bin/env bash
set -euo pipefail
sudo apt update
sudo apt install -y build-essential meson ninja-build pkg-config libgtk-3-dev libwebkit2gtk-4.0-dev clang-format
echo "✅ Dev deps installed."
