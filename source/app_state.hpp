#pragma once
#include <string>
#include <cstdint>
#include "games.hpp"

/**
 * @file app_state.hpp
 * @brief Shared application state passed between all UI screens.
 */

/// Which screen is currently displayed.
enum class AppScreen {
    PLUGIN_ERROR,   ///< Fatal: plugin missing or invalid
    TERMS,          ///< AGB / Terms & Conditions (scroll-to-end required)
    ASSET_INIT,     ///< Asset loader: download / update assets
    APPLET_WARN,    ///< Warning: running in applet mode (limited RAM) – no continue
    USER_SELECT,    ///< Select a Switch user account
    GAME_SELECT,    ///< Select a Pokémon game (filtered by selected user's saves)
    MAIN_MENU,      ///< Main editor shell with tabs
    EXIT,           ///< Request clean shutdown
};

/// A Switch user account (name + uid).
struct UserAccount {
    char      name[0x21] = {};   ///< UTF-8 display name (max 32 chars + NUL)
    uint8_t   uid[0x10]  = {};   ///< AccountUid (128-bit)
    bool      valid      = false;
};

struct AppState {
    AppScreen screen    = AppScreen::PLUGIN_ERROR;
    bool      shouldExit = false;

    // Plugin
    Plugin::Info plugin;

    // Mode
    bool fullRamMode = false;

    // Terms
    bool termsAccepted = false;

    // User selection
    UserAccount            selectedUser{};
    bool                   userSelected = false;

    // Game selection
    const Games::GameInfo* selectedGame = nullptr;
    bool                   gameLoaded   = false;

    // Asset loader state (used by ASSET_INIT screen)
    float  assetDownloadProgress = 0.0f;   ///< 0..1
    float  assetInitProgress     = 0.0f;   ///< 0..1
    bool   assetDownloadNeeded   = false;
    bool   assetsReady           = false;
    std::string assetStatusMessage;

    // NRO directory (resolved once in main)
    std::string nroDir;
};
