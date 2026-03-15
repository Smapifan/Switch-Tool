#pragma once
#include "app_state.hpp"

/**
 * @file save_backup.hpp
 * @brief Creates a ZIP backup of a game's save data.
 *
 * The backup is placed in:
 *   <nroDir>/backup/<username> <YYYY>_<MM>_<DD>_<HH>_<mm>_<ss>.zip
 *
 * The save files are read from the mounted save filesystem via libnx.
 * On success returns true and populates outPath.
 */

namespace Backup {

/**
 * Create a ZIP backup of the currently selected game's save data.
 *
 * @param state   Application state (nroDir, selectedUser, selectedGame).
 * @param outPath Populated with the full path of the created ZIP on success.
 * @return true on success, false on any error.
 */
bool createSaveBackup(const AppState& state, std::string& outPath);

} // namespace Backup
