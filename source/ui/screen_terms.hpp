#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the AGB / Terms & Conditions screen.
/// User must scroll to the end before the Accept button is enabled.
/// Transitions to APPLET_WARN or MAIN_MENU on acceptance,
/// or sets shouldExit on decline.
void renderTerms(AppState& state);
} // namespace UI
