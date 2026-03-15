#pragma once
#include <string>
#include <vector>

/**
 * @file ids_loader.hpp
 * @brief Recursive IDs.json discovery and IDs.txt generation.
 *
 * Searches for IDs.json files at the following depths under a root directory:
 *   <root>/<a>/IDs.json
 *   <root>/<a>/<b>/IDs.json
 *   <root>/<a>/<b>/<c>/IDs.json
 *   <root>/<a>/<b>/<c>/<d>/IDs.json
 *   <root>/<a>/<b>/<c>/<d>/<e>/IDs.json   (up to 5 levels)
 *
 * For each IDs.json found, a human-readable IDs.txt is generated in the same
 * directory.  The txt format is one line per entry:
 *   <name> hat id<ID> Kategorie <category>
 *
 * The ID-to-save-data mapping is handled separately in the game-specific
 * save-editor modules; IDs.txt is provided only for human readability.
 */

namespace IdsLoader {

/// A single entry parsed from an IDs.json file.
struct Entry {
    std::string id;       ///< e.g. "0001"
    std::string texture;  ///< e.g. "abc.png" (file in same dir as IDs.json)
    std::string name;     ///< human-readable name (from "Name" field, optional)
    std::string category; ///< from "Category" field, optional
};

/// A loaded IDs.json file together with its directory.
struct IdsFile {
    std::string            dir;      ///< Directory containing IDs.json
    std::vector<Entry>     entries;
    bool                   txtWritten = false;
};

/**
 * Scan a root directory for all IDs.json files up to 5 levels deep.
 * Parses each file and writes the corresponding IDs.txt alongside it.
 *
 * @param rootDir  Absolute path of the directory to start scanning.
 *                 Typically "<nroDir>/assets/".
 * @return Vector of loaded IDs files (may be empty).
 */
std::vector<IdsFile> discoverAndLoad(const std::string& rootDir);

} // namespace IdsLoader
