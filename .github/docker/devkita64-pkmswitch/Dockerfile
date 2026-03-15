FROM devkitpro/devkita64:latest

SHELL ["/bin/bash", "-lc"]

# Install required Switch portlibs without a full system upgrade.
# Clean the package cache afterwards to keep the image small.
RUN set -euxo pipefail \
  && dkp-pacman -Sy --noconfirm \
  && dkp-pacman -S --needed --noconfirm switch-sdl2 switch-mesa \
  && (dkp-pacman -Scc --noconfirm || true)
