#pragma once
#include <string>
#include <unordered_map>

/**
 * @file i18n.hpp
 * @brief Minimal i18n loader for PKMswitch.
 *
 * Loads per-language JSON files from the `i18n/` directory in RomFS.
 * Each file is named after its locale code:
 *   `default.json` → "en"
 *   `de.json`, `fr.json`, `es.json`, `pt.json`, `it.json`, `ja.json`
 *
 * Format of each per-language file:
 *   { "key": "value", ... }
 */

namespace I18n {

/**
 * Load all translations from a directory.
 * Reads `<dir>/default.json` (→ "en"), `<dir>/de.json`, `<dir>/fr.json`,
 * `<dir>/es.json`, `<dir>/pt.json`, `<dir>/it.json`, `<dir>/ja.json`.
 * Call once after romfsInit().
 * @param dir  Path to the directory, e.g. "romfs:/i18n"
 */
void loadDirectory(const char* dir);

/**
 * Load translations from a single legacy JSON file (kept for tooling/testing).
 * Expected format: { "lang": { "key": "value", ... }, ... }
 * @param path  Path to the JSON file
 */
void load(const char* path);

/**
 * Return true if at least one language was successfully loaded.
 */
bool hasAnyLanguage();

/**
 * Set the active language using a BCP-47-style two-letter code (e.g. "de", "ja").
 * Falls back to "en" if the requested language is not present.
 */
void setLanguage(const std::string& lang);

/**
 * Auto-detect the Switch system language and activate the matching locale.
 * Falls back to "en" on any error.
 */
void detectSystemLanguage();

/**
 * Look up a translation string.
 * Returns the English fallback if the key is missing in the active locale.
 * Returns the key itself if it is not present in any locale.
 */
const char* t(const char* key);

/** Return the currently active language code (e.g. "de"). */
const std::string& currentLanguage();

} // namespace I18n
