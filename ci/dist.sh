#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

echo "[dist] Tooling info:"
echo "[dist] pwd=$(pwd)"
echo "[dist] whoami=$(whoami || true)"
echo "[dist] uname=$(uname -a || true)"
echo "[dist] which make=$(which make || true)"
make --version || true
echo "[dist] DEVKITPRO=${DEVKITPRO:-<unset>}"
echo "[dist] DEVKITARM=${DEVKITARM:-<unset>}"
echo "[dist] PATH=$PATH"

IMGUI_TAG="${IMGUI_TAG:-v1.91.6}"
if [ ! -f imgui/imgui.h ]; then
  echo "[dist] Fetching Dear ImGui ${IMGUI_TAG}..."
  rm -rf imgui
  git clone --depth=1 --branch "${IMGUI_TAG}" https://github.com/ocornut/imgui.git imgui
fi

echo "[dist] Smoke: show first 40 lines of Makefile:"
nl -ba Makefile | sed -n '1,40p'

echo "[dist] Make dry-run (make -n):"
make -n || true

echo "[dist] Make with verbose debug (make --debug=v):"
make --debug=v || true

echo "[dist] Now running actual build:"
make

echo "[dist] Root after make:"
ls -la

echo "[dist] build/ after make (if exists):"
ls -la build || true

# Locate PKMswitch.nro robustly
NRO="$(find . -maxdepth 3 -name 'PKMswitch.nro' -print -quit || true)"
if [ -z "${NRO}" ] || [ ! -f "${NRO}" ]; then
  echo "[dist] ERROR: PKMswitch.nro not found after make!"
  echo "[dist] find outputs:"
  find . -maxdepth 2 -type f -name '*.nro' -o -name '*.elf' -o -name '*.nacp' -print || true
  exit 1
fi

echo "[dist] Building AssetLoader plugin..."
make -C plugin/AssetLoader

echo "[dist] Locating AssetLoader output..."
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

echo "[dist] Creating bundle folder (no sdmc layout)..."
rm -rf dist
mkdir -p dist/PKMswitch/PKMswitch.plugin
cp -f "${NRO}" dist/PKMswitch/PKMswitch.nro
cp -f plugin/AssetLoader/manifest.json dist/PKMswitch/PKMswitch.plugin/manifest.json
cp -f "${PLUGIN_OUT}" dist/PKMswitch/PKMswitch.plugin/AssetLoader.bin

echo "[dist] Bundle ready:"
find dist -maxdepth 4 -type f -print
