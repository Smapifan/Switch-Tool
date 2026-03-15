#pragma once
#include <string>
#include "plugin_check.hpp"

/**
 * @file app_state.hpp
 * @brief Shared application state passed between all UI screens.
 */

/// Which screen is currently displayed.
enum class AppScreen {
    PLUGIN_ERROR,  ///< Fatal: plugin missing or invalid
    TERMS,         ///< AGB / Terms & Conditions (scroll-to-end required)
    APPLET_WARN,   ///< Warning: running in applet mode (limited RAM)
    MAIN_MENU,     ///< Main editor shell with tabs
    EXIT,          ///< Request clean shutdown
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
};
