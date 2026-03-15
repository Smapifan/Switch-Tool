#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the game-selection screen.
/// Lists all supported Pokémon games that the selected user has save data for.
/// On selection: creates a save backup, then advances to MAIN_MENU.
void renderGameSelect(AppState& state);
} // namespace UI
