#pragma once
#include "../app_state.hpp"

namespace UI {
/// Render the user-account selection screen.
/// Populates state.selectedUser and advances to GAME_SELECT on selection.
void renderUserSelect(AppState& state);
} // namespace UI
