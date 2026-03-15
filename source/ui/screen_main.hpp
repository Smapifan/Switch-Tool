#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the main editor shell with tab placeholders.
/// Sets state.shouldExit when the user presses + (Plus) to quit.
void renderMainMenu(AppState& state);
} // namespace UI
