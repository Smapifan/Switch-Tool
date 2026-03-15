# PKMswitch

**PKMswitch** is a Nintendo Switch homebrew NRO application – a Pokémon save-file editor shell built with [Dear ImGui](https://github.com/ocornut/imgui) and SDL2 on top of [devkitPro/libnx](https://github.com/switchbrew/libnx).

> **Status:** UI foundation & plugin system complete – individual editor tabs (Box, Party, Items, Trainer, Pokédex) are placeholders awaiting game-specific save-format implementations.

---

## Table of contents

1. [Features](#features)
2. [Installation on your Switch](#installation-on-your-switch)
3. [Plugin / data-pack specification](#plugin--data-pack-specification)
4. [Building from source](#building-from-source)
5. [CI / GitHub Actions](#ci--github-actions)
6. [Project structure](#project-structure)

---

## Features

| Feature | Details |
|---|---|
| **Plugin-required first boot** | App checks for `PKMswitch.plugin/` next to the `.nro`; shows an error screen with the expected path if it is missing. |
| **Terms & Conditions screen** | User must scroll to the end of the terms before the "Accept" button is enabled (scroll-to-end requirement). |
| **Applet-mode detection** | Detects whether the app is running with limited RAM (Album applet mode) and prompts the user to relaunch with full RAM. |
| **Main editor shell** | Tab bar with placeholders: Box · Party · Items · Trainer · Pokédex · Misc. |
| **i18n** | All UI text lives in a single file: `assets/i18n.json`. System language is detected automatically. Supported: `en` `de` `fr` `es` `it` `ja`. |
| **Icon** | `icon.png` (256 × 256) is wired into the NRO via `--icon`. |

---

## Installation on your Switch

### 1. Copy the NRO

Place `PKMswitch.nro` anywhere on your SD card that the homebrew menu can reach, for example:

```
sdmc:/switch/PKMswitch/PKMswitch.nro
```

### 2. Install the data plugin (required)

The data plugin **must** be placed in the **same directory** as `PKMswitch.nro`:

```
sdmc:/switch/PKMswitch/
├── PKMswitch.nro          ← the application
└── PKMswitch.plugin/      ← the data plugin (required)
    ├── manifest.json      ← plugin descriptor (required)
    ├── strings.json       ← game-string translations (optional)
    └── icons/             ← sprite assets (optional)
        ├── pokemon/       ← <species>_<form>.png  (e.g. 0006_000.png)
        ├── items/         ← <id>.png              (e.g. 00001.png)
        ├── moves/         ← <id>.png
        └── types/         ← <id>.png
```

> The app will **not start** (shows an error screen) if `PKMswitch.plugin/` or its `manifest.json` is missing.

### 3. Launch

Launch PKMswitch from the homebrew menu.

For **full RAM mode** (recommended):
1. Start any game.
2. Hold **R** while selecting PKMswitch from the homebrew menu overlay.

---

## Plugin / data-pack specification

### Directory layout

```
PKMswitch.plugin/
├── manifest.json          ← REQUIRED
├── strings.json           ← optional: game strings (all languages in one file)
└── icons/
    ├── pokemon/<species>_<form>.png
    ├── items/<id>.png
    ├── moves/<id>.png
    └── types/<id>.png
```

Species / form IDs use zero-padded decimal: `0006_001.png` = Charizard form 1.  
Item / move / type IDs use zero-padded 5-digit decimal: `00001.png`.

### `manifest.json` schema

```json
{
  "pluginId":      "my_lgpe_plugin",
  "formatVersion": 1,
  "version":       1,
  "author":        "YourName",
  "games": [
    "010003F003A34000"
  ],
  "content": [
    "strings",
    "icons.pokemon",
    "icons.items",
    "icons.moves",
    "icons.types"
  ]
}
```

| Field | Type | Required | Description |
|---|---|---|---|
| `pluginId` | string | **yes** | Unique identifier for this plugin |
| `formatVersion` | integer | **yes** | Must be `1` for the current spec |
| `version` | integer | **yes** | Your plugin's data version |
| `author` | string | no | Plugin author name |
| `games` | string[] | no | Title-IDs this plugin targets |
| `content` | string[] | no | List of provided asset types |

The app validates `pluginId` (non-empty) and `formatVersion == 1`; all other fields are informational.

### `strings.json` format (optional)

```json
{
  "en": {
    "species.0001": "Bulbasaur",
    "move.0001":    "Pound",
    "item.0001":    "Master Ball"
  },
  "de": {
    "species.0001": "Bisasam"
  }
}
```

The app falls back to English if a key is missing in the active language, and shows the raw key if missing from English too.

---

## Building from source

### Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started) with **devkitA64** and **libnx**
- devkitPro package: `switch-sdl2`  
  ```bash
  sudo dkp-pacman -S switch-sdl2 switch-mesa switch-libdrm-nouveau
  ```
- Dear ImGui source (fetched with the helper target, see below)

### Steps

```bash
# 1. Clone this repository
git clone https://github.com/Smapifan/Switch-Tool.git
cd Switch-Tool

# 2. Fetch Dear ImGui (not tracked in git)
make fetch-imgui
# or manually:
# git clone --depth=1 --branch v1.91.6 https://github.com/ocornut/imgui.git imgui

# 3. Build
make

# Output: PKMswitch.nro
```

### Clean

```bash
make clean
```

---

## CI / GitHub Actions

The workflow in `.github/workflows/build.yml`:

1. Checks out the repository using the **devkitpro/devkita64** Docker image.
2. Installs `switch-sdl2`, `switch-mesa`, `switch-libdrm-nouveau` via `dkp-pacman`.
3. Clones Dear ImGui at the pinned tag.
4. Runs `make`.
5. Uploads `PKMswitch.nro` as the **PKMswitch-nro** artifact.

The artifact is available for download from the Actions tab after every successful build.

---

## Project structure

```
Switch-Tool/
├── .github/workflows/build.yml   ← CI workflow
├── assets/
│   └── i18n.json                 ← ALL UI strings (single source of truth)
├── source/
│   ├── main.cpp                  ← entry point, SDL2+ImGui init, main loop
│   ├── app_state.hpp             ← shared state & screen enum
│   ├── i18n.hpp / i18n.cpp       ← i18n loader & t() helper
│   ├── plugin_check.hpp / .cpp   ← plugin validation
│   ├── backends/
│   │   ├── imgui_sdl2_impl.cpp          ← wraps imgui/backends/imgui_impl_sdl2.cpp
│   │   └── imgui_sdlrenderer2_impl.cpp  ← wraps imgui/backends/imgui_impl_sdlrenderer2.cpp
│   └── ui/
│       ├── screen_plugin_error.*  ← "plugin missing" error screen
│       ├── screen_terms.*         ← AGB / terms screen (scroll-to-end)
│       ├── screen_applet.*        ← applet-mode relaunch prompt
│       └── screen_main.*          ← main editor shell with tabs
├── icon.png                      ← 256×256 NRO icon
├── Makefile                      ← devkitPro/libnx build system
└── README.md
```

> `imgui/` is **not** tracked in git. It is fetched by `make fetch-imgui` or the CI workflow.
