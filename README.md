# PKMswitch

**PKMswitch** is a Nintendo Switch homebrew NRO application – a PKHeX-style Pokémon save-file editor built with [Dear ImGui](https://github.com/ocornut/imgui) and SDL2 on top of [devkitPro/libnx](https://github.com/switchbrew/libnx).

> **Status:** UI foundation, multi-screen flow, all editor tabs, asset download/update plugin, user selection, game selection with automatic save backup, IDs.json discovery, and i18n multi-language support are all implemented.  
> Individual save-format parsers for each game (the actual byte-level Pokémon data) are architecture-ready placeholders awaiting game-specific implementations.

---

## Table of contents

1. [Features](#features)
2. [Installation on your Switch](#installation-on-your-switch)
3. [AssetLoader plugin](#assetloader-plugin)
4. [Plugin / data-pack specification](#plugin--data-pack-specification)
5. [Building from source](#building-from-source)
6. [CI / GitHub Actions](#ci--github-actions)
7. [Project structure](#project-structure)

---

## Features

| Feature | Details |
|---|---|
| **Applet-mode detection** | On first launch from the Album applet the app shows a **warning-only** screen: *"This application must be started in Application Mode with full RAM access."* No continue button – only Exit. |
| **User selection** | Enumerates all Switch user accounts via `accountListAllUsers`. Only games that have save data for the chosen user are shown. |
| **Game selection with backup** | Lists only Pokémon games with save data for the selected user. Every time a game is selected, a ZIP backup is automatically created in `backup/` (format: `<username> YYYY_MM_DD_HH_mm_ss.zip`). |
| **AssetLoader plugin** | Companion plugin (`plugin/AssetLoader/`) that downloads the full `assets/` bundle on first run, checks for updates on every launch using a remote `Version.txt`, and shows animated progress bars (*Download Data* + *Initialise data*). Requires Wi-Fi on first run. |
| **Main editor – 10 tabs** | **Box** · **Party** · **Items** · **Trainer** · **Raids** (SwSh/SV) · **Donuts** (ZA) · **Events** · **Pokédex** · **Encounter DB** · **Event DB** – exactly mirroring PKHeX's layout. |
| **Pokémon editor (left panel)** | Editable: Species, Level, Nature, Ability, Item, IVs (0–31 each), EVs (0–252 each), 4 moves, PID, TID, SID. Legality indicator (placeholder; architecture is ready for PKHeX-compatible checks). |
| **Items tab** | Categorised item list with per-item count editor, delete button, and "Give all items" with custom quantity. |
| **Trainer tab** | Name, TID/SID, Money, Badges, game-specific fields (BP for SwSh/SV, LP for Legends: Arceus). |
| **Raids tab** | Raid injector with star-count selector and an Event-raid mode. |
| **Donuts tab** | Legends Z-A dimensional Donut management. |
| **Events tab** | Mystery Gift / Wonder Card importer. |
| **Pokédex tab** | Searchable Pokédex placeholder table. |
| **Encounter DB tab** | Encounter table with species / level / location / rate columns and an Inject button. |
| **Event DB tab** | Notable Pokémon distribution list (Manaphy Egg, Shiny Koraidon, …) with per-row Inject. |
| **IDs.json discovery** | Recursively scans `assets/*/IDs.json` through `assets/*/*/*/*/*/IDs.json` (5 levels deep). Generates a human-readable `IDs.txt` alongside each JSON. |
| **i18n** | Per-language JSON files in `assets/i18n/`: `default.json` (en), `de.json`, `fr.json`, `es.json`, `pt.json`. System language is detected automatically; falls back to English. |
| **Terms & Conditions screen** | Must scroll to end before Accept is enabled. Accepted flag persisted to `sdmc:/switch/PKMswitch/.terms_accepted`. |

---

## Installation on your Switch

### 1. Download the release ZIP

Download the latest `PKMswitch_*.zip` from the [Releases](https://github.com/Smapifan/Switch-Tool/releases) page
(or from the **PKMswitch-Release** CI artifact).

### 2. Copy to SD card

Extract the ZIP and copy the **`sdmc/`** folder to the **root of your Switch SD card**:

```
SD card root/
└── switch/
    └── PKMswitch/
        ├── PKMswitch.nro            ← the application
        └── PKMswitch.plugin/
            ├── manifest.json        ← data plugin manifest
            └── AssetLoader.bin      ← asset download plugin
```

> **No pre-installed assets required.** On first launch with Wi-Fi connected,
> the AssetLoader plugin will automatically download the required assets and
> return you to PKMswitch.

### 3. First-launch flow

| Condition | Behaviour |
|---|---|
| **Wi-Fi connected (first run)** | PKMswitch launches AssetLoader.bin to download assets, then restarts automatically |
| **Wi-Fi NOT connected (first run)** | Error screen: connect to Wi-Fi and restart |
| **Subsequent launches** | AssetLoader.bin checks for asset updates, then PKMswitch starts normally |

### 4. Launch in full-RAM mode (required)

1. Start any game.
2. Hold **R** while selecting PKMswitch from the homebrew menu overlay.

---

## AssetLoader plugin

The `plugin/AssetLoader/` directory contains the companion plugin source code.

### Behaviour

| Phase | Condition | Screen |
|---|---|---|
| **Check for updates** | Always | Fetches remote `Version.txt` from GitHub |
| **Download Data** | First run or outdated assets | Progress bar |
| **Initialise data** | Always after download (or if up-to-date) | Progress bar |

- On **first run** (no local `assets/Version.txt`) Wi-Fi is **required**. The app shows an error if Wi-Fi is unavailable.
- On subsequent runs, if Wi-Fi is absent, the update check is skipped and existing local assets are used.

### URLs (hardcoded in `source/asset_loader.hpp`)

```
REMOTE_VERSION_URL = https://raw.githubusercontent.com/Smapifan/Switch-Tool/main/assets/Version.txt
REMOTE_ASSETS_ZIP_URL = https://github.com/Smapifan/Switch-Tool/releases/latest/download/assets.zip
```

### `assets/Version.txt` format

```
Version: 1.0
Published on: 2025-01-01
Downloaded on: 2025/06/15/14/32/10
```

---

## Plugin / data-pack specification

### `PKMswitch.plugin/manifest.json`

```json
{
  "pluginId":      "my_plugin",
  "formatVersion": 1,
  "version":       1,
  "author":        "YourName",
  "games": ["010003F003A34000"],
  "content": ["strings", "icons.pokemon", "icons.items"]
}
```

---

## Building from source

### Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started) with **devkitA64** and **libnx**
- devkitPro packages: `switch-sdl2`, `switch-mesa`, `switch-libdrm-nouveau`, `switch-curl`
  ```bash
  sudo dkp-pacman -S switch-sdl2 switch-mesa switch-libdrm-nouveau switch-curl
  ```
- Dear ImGui (fetched automatically during build)

### Build everything (NRO + plugin + dist layout)

```bash
# 1. Clone
git clone https://github.com/Smapifan/Switch-Tool.git
cd Switch-Tool

# 2. Build app + plugin and assemble dist/ layout
bash ci/dist.sh
```

This produces:
```
dist/sdmc/switch/PKMswitch/
├── PKMswitch.nro
└── PKMswitch.plugin/
    ├── manifest.json
    └── AssetLoader.bin
Release/PKMswitch_local.zip
```

### Build steps individually

```bash
# Fetch Dear ImGui (shared by main app and plugin)
make fetch-imgui

# Build main NRO
make

# Build AssetLoader plugin
make -C plugin/AssetLoader
```

---

## CI / GitHub Actions

The workflow in `.github/workflows/build.yml`:

1. Uses the prebuilt **ghcr.io/smapifan/devkita64-pkmswitch** Docker image (SDL2, curl pre-installed; no `pkg.devkitpro.org` contact needed at build time).
2. Runs `ci/dist.sh` which fetches Dear ImGui, builds the NRO, builds the plugin, and assembles `dist/`.
3. Zips `dist/sdmc/` to `Release/PKMswitch_<SHA>.zip`.
4. Uploads the zip as the **PKMswitch-Release** artifact.

> `assets/` is intentionally **not** included in the dist package; assets are downloaded at runtime by the AssetLoader plugin.

---

## Project structure

```
Switch-Tool/
├── .github/
│   ├── docker/devkita64-pkmswitch/Dockerfile   ← prebuilt CI image
│   └── workflows/
│       ├── build.yml          ← CI: build + dist zip (no pkg.devkitpro.org needed)
│       └── build-image.yml    ← CI: rebuild Docker image
├── ci/
│   ├── dist.sh                ← build script: produces dist/ layout + Release zip
│   └── dkp-pacman-install.sh  ← retry-aware dkp-pacman helper
├── assets/                    ← embedded as RomFS in PKMswitch.nro (i18n, icons)
│   ├── Version.txt            ← remote version reference (NOT shipped in dist)
│   ├── i18n/
│   │   ├── default.json       ← English (fallback)
│   │   ├── de.json            ← German
│   │   ├── fr.json            ← French
│   │   ├── es.json            ← Spanish
│   │   └── pt.json            ← Portuguese
│   └── icons/ / items/ / pokemon/
├── plugin/
│   └── AssetLoader/
│       ├── Makefile           ← builds AssetLoader.bin (NRO format, .bin extension)
│       ├── manifest.json      ← plugin manifest (copied to dist)
│       └── source/main.cpp   ← standalone asset download app; relaunches PKMswitch
├── source/
│   ├── main.cpp               ← entry point; WLAN gate + chain-launch plugin
│   ├── app_state.hpp          ← AppState & AppScreen enum (incl. NO_ASSETS)
│   ├── games.hpp
│   ├── i18n.hpp / i18n.cpp
│   ├── plugin_check.hpp / .cpp
│   ├── asset_loader.hpp / .cpp
│   ├── ids_loader.hpp / .cpp
│   ├── save_backup.hpp / .cpp
│   └── ui/
│       ├── screen_plugin_error.*
│       ├── screen_no_assets.*   ← "no Wi-Fi on first run" error screen
│       ├── screen_terms.*
│       ├── screen_applet.*
│       ├── screen_user.*
│       ├── screen_game.*
│       └── screen_main.*
├── icon.png
├── Makefile
└── README.md
```

### dist/ layout (produced by `ci/dist.sh`)

```
dist/sdmc/switch/PKMswitch/
├── PKMswitch.nro                    ← main app (i18n embedded as RomFS)
└── PKMswitch.plugin/
    ├── manifest.json                ← data plugin manifest
    └── AssetLoader.bin              ← asset downloader (invoked on first run / update)
```

> `assets/` is **not** included in dist — it is downloaded at runtime.

