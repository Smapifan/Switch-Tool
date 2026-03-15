#pragma once
#include <cstdint>
#include <vector>
#include <string>

/**
 * @file games.hpp
 * @brief Definitions for all supported Pokémon Switch titles.
 *
 * Title IDs are from the Nintendo game catalogue.
 * Game icon paths point to files in assets/icons/games/ (downloaded by AssetLoader).
 */

namespace Games {

/// Identifies a Pokémon game generation / platform.
enum class Gen : uint8_t {
    LGPE = 0,  ///< Let's Go Pikachu / Eevee
    SwSh,      ///< Sword / Shield
    BDSP,      ///< Brilliant Diamond / Shining Pearl
    LA,        ///< Legends: Arceus
    SV,        ///< Scarlet / Violet
    ZA,        ///< Legends: Z-A (placeholder)
    Unknown,
};

struct GameInfo {
    uint64_t    titleId;      ///< Primary title ID (used for save mounting)
    const char* shortName;    ///< Short identifier key (used in file/folder names)
    const char* displayName;  ///< Human-readable display name
    const char* iconFile;     ///< Filename in assets/icons/games/ (PNG)
    Gen         gen;          ///< Generation
    int         boxCount;     ///< Number of PC boxes
    int         boxSlots;     ///< Slots per box
    bool        hasRaids;     ///< Raids tab visible
    bool        hasDonuts;    ///< Donuts tab visible
    bool        hasBP;        ///< Has Battle Points (SwSh/SV)
    bool        hasLP;        ///< Has League Points (LA)
};

// ── Supported games ──────────────────────────────────────────────────────────

static constexpr GameInfo GAMES[] = {
    // Let's Go
    { 0x010003F003A34000ULL, "lgpe", "Pokémon: Let's Go, Pikachu!",
      "lgpp.png", Gen::LGPE, 8, 30, false, false, false, false },
    { 0x0100187003A34000ULL, "lgee", "Pokémon: Let's Go, Eevee!",
      "lgpe.png", Gen::LGPE, 8, 30, false, false, false, false },

    // Sword / Shield
    { 0x0100ABF008968000ULL, "sw", "Pokémon Sword",
      "sword.png", Gen::SwSh, 32, 30, true, false, true, false },
    { 0x01008DB008C2C000ULL, "sh", "Pokémon Shield",
      "shield.png", Gen::SwSh, 32, 30, true, false, true, false },

    // Brilliant Diamond / Shining Pearl
    { 0x0100000011D90000ULL, "bd", "Pokémon Brilliant Diamond",
      "bd.png", Gen::BDSP, 24, 30, false, false, false, false },
    { 0x010018E011D92000ULL, "sp", "Pokémon Shining Pearl",
      "sp.png", Gen::BDSP, 24, 30, false, false, false, false },

    // Legends: Arceus
    { 0x01001F5010DFA000ULL, "la", "Pokémon Legends: Arceus",
      "la.png", Gen::LA, 32, 30, false, false, false, true },

    // Scarlet / Violet
    { 0x0100A3D008C5C000ULL, "sc", "Pokémon Scarlet",
      "scarlet.png", Gen::SV, 32, 30, true, false, true, false },
    { 0x01008F6008C5E000ULL, "vi", "Pokémon Violet",
      "violet.png", Gen::SV, 32, 30, true, false, true, false },

    // Legends: Z-A (TID TBD – placeholder)
    { 0x0100B3F000BE2000ULL, "za", "Pokémon Legends: Z-A",
      "za.png", Gen::ZA, 32, 30, false, true, false, false },
};

static constexpr size_t GAME_COUNT = sizeof(GAMES) / sizeof(GAMES[0]);

/// Find a GameInfo entry by title ID (nullptr if not found).
inline const GameInfo* findByTitleId(uint64_t id) {
    for (size_t i = 0; i < GAME_COUNT; ++i)
        if (GAMES[i].titleId == id) return &GAMES[i];
    return nullptr;
}

} // namespace Games
