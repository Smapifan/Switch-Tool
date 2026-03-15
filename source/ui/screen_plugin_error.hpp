#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the "plugin missing / invalid" error screen.
/// Sets state.shouldExit = true when the user chooses to exit.
void renderPluginError(AppState& state);
} // namespace UI
