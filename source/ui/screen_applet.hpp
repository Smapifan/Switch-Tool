#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the applet-mode warning / relaunch prompt.
/// "Continue" → MAIN_MENU, "Exit" → shouldExit.
void renderAppletWarn(AppState& state);
} // namespace UI
