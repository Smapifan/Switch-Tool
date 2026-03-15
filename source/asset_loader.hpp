#pragma once
#include "app_state.hpp"
#include <string>

/**
 * @file asset_loader.hpp
 * @brief Asset download and update logic for PKMswitch.
 *
 * On first run (no local assets/Version.txt), downloads the full asset bundle
 * from a remote URL.  On subsequent runs, fetches the remote Version.txt and
 * compares versions.  If an update is available the bundle is re-downloaded.
 *
 * Progress is reported via AppState::assetDownloadProgress /
 * AppState::assetInitProgress so the UI can render progress bars.
 *
 * Network (nifm) must be initialised before calling any function here.
 */

namespace AssetLoader {

// ── Configuration (edit to point at your actual release URLs) ─────────────────
// These are the only two places where remote URLs appear in the codebase.
static constexpr const char* REMOTE_VERSION_URL =
    "https://raw.githubusercontent.com/Smapifan/Switch-Tool/main/assets/Version.txt";
static constexpr const char* REMOTE_ASSETS_ZIP_URL =
    "https://github.com/Smapifan/Switch-Tool/releases/latest/download/assets.zip";

/**
 * Run the asset initialisation flow (blocking, call from a background thread
 * or accept the frame-stall on first run).
 *
 * Phases:
 *   1. If update/first-run: "Download Data" phase  (assetDownloadProgress)
 *   2. Always:              "Initialisiere Daten"  (assetInitProgress)
 *
 * @param state   AppState with nroDir set.
 * @return true on success.
 */
bool run(AppState& state);

/**
 * Check whether WLAN is currently connected.
 * Requires nifmInitialize() to have been called.
 */
bool isWlanAvailable();

/**
 * Check whether this is the first run (assets/Version.txt is absent).
 * @param assetsDir  Local assets directory path.
 */
bool isFirstRun(const std::string& assetsDir);

/**
 * Render the asset-init progress screen (called every frame while assetsReady==false).
 */
void renderInitScreen(AppState& state);

} // namespace AssetLoader
