#!/usr/bin/env sh
set -eux

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

IMGUI_TAG="${IMGUI_TAG:-v1.91.6}"
if [ ! -f imgui/imgui.h ]; then
  echo "[dist] Fetching Dear ImGui ${IMGUI_TAG}..."
  rm -rf imgui
  git clone --depth=1 --branch "${IMGUI_TAG}" https://github.com/ocornut/imgui.git imgui
fi

echo "[dist] Building PKMswitch (verbose)..."
make V=1

echo "[dist] Root after make:"
ls -la
echo "[dist] build/ after make:"
ls -la build || true

# Locate PKMswitch.nro robustly
NRO=""
if [ -f "PKMswitch.nro" ]; then
  NRO="PKMswitch.nro"
elif [ -f "build/PKMswitch.nro" ]; then
  NRO="build/PKMswitch.nro"
else
  NRO="$(find . -maxdepth 3 -name 'PKMswitch.nro' -print -quit || true)"
fi

if [ -z "${NRO}" ] || [ ! -f "${NRO}" ]; then
  echo "[dist] ERROR: PKMswitch.nro not found after make!"
  exit 1
fi

echo "[dist] Building AssetLoader plugin..."
make -C plugin/AssetLoader

# Locate plugin output (prefer .bin if it exists, else accept .nro for now)
PLUGIN_OUT=""
if [ -f "plugin/AssetLoader/build/AssetLoader.bin" ]; then
  PLUGIN_OUT="plugin/AssetLoader/build/AssetLoader.bin"
elif [ -f "plugin/AssetLoader/build/AssetLoader.nro" ]; then
  PLUGIN_OUT="plugin/AssetLoader/build/AssetLoader.nro"
else
  PLUGIN_OUT="$(find plugin/AssetLoader -maxdepth 3 -name 'AssetLoader.*' -print -quit || true)"
fi

if [ -z "${PLUGIN_OUT}" ] || [ ! -f "${PLUGIN_OUT}" ]; then
  echo "[dist] ERROR: AssetLoader output not found."
  ls -la plugin/AssetLoader || true
  ls -la plugin/AssetLoader/build || true
  exit 1
fi

echo "[dist] Creating release bundle folder (no sdmc layout)..."
rm -rf dist
mkdir -p dist/PKMswitch/PKMswitch.plugin

cp -f "${NRO}" dist/PKMswitch/PKMswitch.nro
cp -f plugin/AssetLoader/manifest.json dist/PKMswitch/PKMswitch.plugin/manifest.json
cp -f "${PLUGIN_OUT}" dist/PKMswitch/PKMswitch.plugin/AssetLoader.bin

echo "[dist] Bundle ready:"
find dist -maxdepth 4 -type f -print
