#!/usr/bin/env bash
# gen_icon.sh – regenerate assets/icon.png (1024×1024 Pokéball) via ImageMagick.
# Requires: ImageMagick 7+ (magick) or ImageMagick 6 (convert).
# Usage: bash tools/gen_icon.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
OUT="${REPO_ROOT}/assets/icon.png"
SIZE=1024

# Inline SVG: Pokéball design
SVG=$(cat << 'SVG_EOF'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
  <!-- Black border circle -->
  <circle cx="50" cy="50" r="48" fill="#141414"/>
  <!-- Top half: red -->
  <path d="M2,50 A48,48 0 0,1 98,50 Z" fill="#dc1e1e"/>
  <!-- Bottom half: white -->
  <path d="M2,50 A48,48 0 0,0 98,50 Z" fill="#f0f0f0"/>
  <!-- Middle band -->
  <rect x="2" y="45" width="96" height="10" fill="#141414"/>
  <!-- Centre band fill -->
  <rect x="2" y="46.5" width="96" height="7" fill="#e6e6e6"/>
  <!-- Centre button border -->
  <circle cx="50" cy="50" r="14" fill="#141414"/>
  <!-- Centre button fill -->
  <circle cx="50" cy="50" r="11" fill="#e6e6e6"/>
</svg>
SVG_EOF
)

mkdir -p "${REPO_ROOT}/assets"

if command -v magick &>/dev/null; then
    echo "${SVG}" | magick -background none svg:- -resize "${SIZE}x${SIZE}" "${OUT}"
elif command -v convert &>/dev/null; then
    echo "${SVG}" | convert -background none svg:- -resize "${SIZE}x${SIZE}" "${OUT}"
else
    echo "Error: ImageMagick (magick or convert) not found." >&2
    exit 1
fi

echo "Generated ${OUT} (${SIZE}x${SIZE})"
