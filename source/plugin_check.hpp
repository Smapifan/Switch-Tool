#pragma once
#include <string>

/**
 * @file plugin_check.hpp
 * @brief PKMswitch data-plugin validation.
 *
 * Plugin layout (sibling of the .nro file):
 *
 *   <nro_dir>/PKMswitch.plugin/
 *       manifest.json       ← required
 *       strings.json        ← optional game-string data
 *       icons/              ← optional image assets
 *           pokemon/        ← <species>_<form>.png
 *           items/          ← <id>.png
 *           moves/          ← <id>.png
 *           types/          ← <id>.png
 *
 * manifest.json minimal schema:
 * {
 *   "pluginId":      "my_plugin",          // string, required
 *   "formatVersion": 1,                    // integer, must be 1
 *   "version":       1,                    // integer, plugin data version
 *   "author":        "Author Name",        // string, optional
 *   "games":         ["010003F003A34000"], // array of title-IDs, optional
 *   "content":       ["strings","icons"]  // array of provided content types
 * }
 */

namespace Plugin {

struct Info {
    std::string dir;          ///< Absolute path of PKMswitch.plugin/
    std::string pluginId;
    std::string author;
    int         formatVersion = 0;
    int         version       = 0;
    bool        valid         = false;
};

/**
 * Derive the plugin directory path from the running NRO path.
 * Returns "<nro_dir>/PKMswitch.plugin" or a safe default.
 * @param argv0  argv[0] from main(), may be nullptr.
 */
std::string resolvePluginDir(const char* argv0);

/**
 * Check that the plugin directory and its manifest.json exist and are valid.
 * Populates @p info on success.
 * @return true if the plugin is present and valid.
 */
bool validate(const char* pluginDir, Info& info);

} // namespace Plugin
