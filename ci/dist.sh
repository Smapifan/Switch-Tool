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

echo "[dist] Building (verbose) ..."
make V=1

echo "[dist] Root after make:"
ls -la
echo "[dist] build/ after make:"
ls -la build || true

# Locate PKMswitch.nro robustly (root or build/)
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

# Find plugin output (until plugin Makefile produces a deterministic .bin)
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

# Layout doesn't matter: everything happens in the NRO directory and subdirs,
# but we still produce a clean folder for packing/copying.
echo "[dist] Assembling output folder..."
OUT="dist/PKMswitch"
PLUG="${OUT}/PKMswitch.plugin"

rm -rf dist
mkdir -p "${PLUG}"

cp -f "${NRO}" "${OUT}/PKMswitch.nro"
cp -f plugin/AssetLoader/manifest.json "${PLUG}/manifest.json"
cp -f "${PLUGIN_OUT}" "${PLUG}/AssetLoader.bin"

echo "[dist] Done:"
find dist -maxdepth 5 -type f -print
