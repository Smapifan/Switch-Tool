#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the "no Wi-Fi and assets not downloaded" first-run error screen.
/// Sets state.shouldExit = true when the user chooses to exit.
void renderNoAssets(AppState& state);
} // namespace UI
