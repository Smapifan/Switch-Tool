#!/usr/bin/env sh
# ci/dist.sh — Build PKMswitch + AssetLoader plugin and produce the dist/ layout.
#
# Output:
#   dist/sdmc/switch/PKMswitch/PKMswitch.nro
#   dist/sdmc/switch/PKMswitch/PKMswitch.plugin/manifest.json
#   dist/sdmc/switch/PKMswitch/PKMswitch.plugin/AssetLoader.bin
#   Release/PKMswitch_<SHA>.zip
#
# Assets are intentionally NOT included in dist; they are downloaded at runtime
# by the AssetLoader plugin on first launch.
set -eux

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

# ── Fetch Dear ImGui (shared by main app and plugin) ──────────────────────────
IMGUI_TAG="${IMGUI_TAG:-v1.91.6}"
if [ ! -f "imgui/imgui.h" ]; then
    echo "[dist] Fetching Dear ImGui ${IMGUI_TAG}..."
    rm -rf imgui
    git clone --depth=1 --branch "${IMGUI_TAG}" https://github.com/ocornut/imgui.git imgui
fi

# ── Build main application ─────────────────────────────────────────────────────
echo "[dist] Building PKMswitch.nro..."
make

# ── Build AssetLoader plugin ───────────────────────────────────────────────────
echo "[dist] Building AssetLoader plugin..."
# imgui/ is at repo root; plugin Makefile references ../../imgui from its own dir
make -C plugin/AssetLoader

# ── Assemble dist/ layout ──────────────────────────────────────────────────────
echo "[dist] Assembling dist layout..."
PKMD="$ROOT_DIR/dist/sdmc/switch/PKMswitch"
PLUG="$PKMD/PKMswitch.plugin"

rm -rf "$ROOT_DIR/dist"
mkdir -p "$PKMD"
mkdir -p "$PLUG"

cp PKMswitch.nro                            "$PKMD/PKMswitch.nro"
cp plugin/AssetLoader/manifest.json         "$PLUG/manifest.json"
cp plugin/AssetLoader/build/AssetLoader.bin "$PLUG/AssetLoader.bin"

echo "[dist] dist/ layout:"
find "$ROOT_DIR/dist" -type f

# ── Create Release zip ─────────────────────────────────────────────────────────
echo "[dist] Creating Release zip..."
mkdir -p "$ROOT_DIR/Release"
ARCHIVE_BASE="PKMswitch_${GITHUB_SHA:-local}"

cd "$ROOT_DIR/dist"
if command -v zip >/dev/null 2>&1; then
    zip -r "$ROOT_DIR/Release/${ARCHIVE_BASE}.zip" sdmc
else
    echo "[dist] zip not found, using tar.gz instead"
    tar -czf "$ROOT_DIR/Release/${ARCHIVE_BASE}.tar.gz" sdmc
fi

echo "[dist] Done."
ls -lh "$ROOT_DIR/Release/"
