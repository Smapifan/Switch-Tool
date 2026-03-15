#pragma once
#include <string>
#include <unordered_map>

/**
 * @file i18n.hpp
 * @brief Minimal i18n loader for PKMswitch.
 *
 * Loads assets/i18n.json (via RomFS) and exposes a single t() lookup.
 * All UI strings are stored in assets/i18n.json – the single source of truth.
 */

namespace I18n {

/**
 * Load translations from the given JSON file path.
 * Expected format: { "lang": { "key": "value", ... }, ... }
 * Call once after romfsInit().
 * @param path  RomFS path, e.g. "romfs:/i18n.json"
 */
void load(const char* path);

/**
 * Set the active language using a BCP-47-style two-letter code (e.g. "de", "ja").
 * Falls back to "en" if the requested language is not present.
 */
void setLanguage(const std::string& lang);

/**
 * Auto-detect the Switch system language and activate the matching locale.
 * Falls back to "en" on any error.
 * Requires setInitialize() to have been called beforehand.
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
