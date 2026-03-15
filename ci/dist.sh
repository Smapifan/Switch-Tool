#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

# ── 1. Ensure Dear ImGui v1.91.6 is present ──────────────────────────────────
if [ ! -d imgui ]; then
  echo "Fetching Dear ImGui v1.91.6..."
  git clone --depth=1 --branch v1.91.6 \
      https://github.com/ocornut/imgui.git imgui
fi

# ── 2. Build main NRO ────────────────────────────────────────────────────────
echo "Building PKMswitch.nro..."
make

# ── 3. Locate PKMswitch.nro ──────────────────────────────────────────────────
NRO=""
if [ -f "PKMswitch.nro" ]; then
  NRO="PKMswitch.nro"
elif [ -f "build/PKMswitch.nro" ]; then
  NRO="build/PKMswitch.nro"
else
  NRO=$(find . -maxdepth 2 -name "PKMswitch.nro" | head -n 1)
fi

if [ -z "$NRO" ]; then
  echo "ERROR: PKMswitch.nro not found after make!" >&2
  ls -la || true
  ls -la build || true
  exit 1
fi
echo "Found NRO: $NRO"

# ── 4. Build AssetLoader plugin ──────────────────────────────────────────────
echo "Building AssetLoader plugin..."
make -C plugin/AssetLoader

# ── 5. Locate plugin output ──────────────────────────────────────────────────
PLUGIN_BIN=""
if [ -f "plugin/AssetLoader/build/AssetLoader.bin" ]; then
  PLUGIN_BIN="plugin/AssetLoader/build/AssetLoader.bin"
elif [ -f "plugin/AssetLoader/build/AssetLoader.nro" ]; then
  PLUGIN_BIN="plugin/AssetLoader/build/AssetLoader.nro"
else
  PLUGIN_BIN=$(find plugin/AssetLoader -maxdepth 3 \( -name "AssetLoader.bin" -o -name "AssetLoader.nro" \) | head -n 1)
fi

if [ -z "$PLUGIN_BIN" ]; then
  echo "ERROR: AssetLoader plugin output not found after make!" >&2
  ls -la plugin/AssetLoader/build || true
  exit 1
fi
echo "Found plugin: $PLUGIN_BIN"

# ── 6. Assemble SD layout ────────────────────────────────────────────────────
SD_DIR="dist/sdmc/switch/PKMswitch"
PLUGIN_DIR="${SD_DIR}/PKMswitch.plugin"

rm -rf dist
mkdir -p "$SD_DIR"
mkdir -p "$PLUGIN_DIR"

cp "$NRO"             "${SD_DIR}/PKMswitch.nro"
cp "$PLUGIN_BIN"      "${PLUGIN_DIR}/AssetLoader.bin"
cp "plugin/AssetLoader/manifest.json" "${PLUGIN_DIR}/manifest.json"

# ── 7. Summary ───────────────────────────────────────────────────────────────
echo "dist layout:"
find dist
