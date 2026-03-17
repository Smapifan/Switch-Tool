#include <switch.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>
#include <cstdio>

// --- Dummy-Datenstruktur für Items ---
struct ItemInfo {
    int id;
    std::string name;
    std::map<std::string, std::string> translations;
};
// --- Dummy-Datenstruktur für Pokémon ---
struct PokemonInfo {
    int id;
    std::string name;
    std::map<std::string, std::string> translations;
};

// Beispiel-Daten (beliebig ausbaubar)
std::map<std::string, std::vector<ItemInfo>> gameItems = {
    {"firered",   { {1,"POKE BALL", {{"en","POKE BALL"},{"de","POKEBALL"}}}, {2,"POTION", {{"en","POTION"}}} }},
    {"leafgreen", { {1,"POKE BALL", {{"en","POKE BALL"},{"de","POKEBALL"},{"fr","BALLE"}}}, {2,"POTION", {{"en","POTION"}}} }},
    {"emerald",   { {1,"POKE BALL", {{"en","POKE BALL"}}} }}
};
std::map<std::string, std::vector<PokemonInfo>> gamePokemon = {
    {"firered", {
        {1,"Bulbasaur", {{"de","Bisasam"},{"fr","Bulbizarre"}}},
        {2,"Ivysaur",   {{"de","Bisaknosp"}}}
    }},
    {"emerald", { {3,"Treecko", {{"de","Geckarbor"}}} }}
};

// mkdirp für sdmc:/...
static void mkdirp(const std::string& path) {
    for (size_t i = 1; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '/')
            mkdir(path.substr(0,i).c_str(), 0777);
}

int main(int argc, char* argv[]) {
    romfsInit();
    mkdirp("sdmc:/switch/PKMswitch/assets/IDs");

    // --- Items exportieren ---
    for (const auto& game : gameItems) {
        std::string gameName = game.first;
        for (const auto& item : game.second) {
            // Export für alle übersetzten Namen (Sprachen)
            for (const auto& trans : item.translations) {
                std::string lang = trans.first;
                std::string fname = gameName;
                if (lang != "en") fname += "_" + lang;
                std::string idFile = "sdmc:/switch/PKMswitch/assets/IDs/" + fname + "_items.txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", item.id, trans.second.c_str());
                    fclose(outf);
                }
            }
            // Englisch-Fallback falls nicht im translations
            if (item.translations.count("en") == 0) {
                std::string idFile = "sdmc:/switch/PKMswitch/assets/IDs/" + gameName + "_items.txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", item.id, item.name.c_str());
                    fclose(outf);
                }
            }
        }
    }

    // --- Pokémon exportieren ---
    for (const auto& game : gamePokemon) {
        std::string gameName = game.first;
        for (const auto& poke : game.second) {
            // Export für alle übersetzten Namen (Sprachen)
            for (const auto& trans : poke.translations) {
                std::string lang = trans.first;
                std::string fname = gameName;
                if (lang != "en") fname += "_" + lang;
                std::string idFile = "sdmc:/switch/PKMswitch/assets/IDs/" + fname + "_pokemon.txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", poke.id, trans.second.c_str());
                    fclose(outf);
                }
            }
            // Englisch-Fallback falls nicht im translations
            if (poke.translations.count("en") == 0) {
                std::string idFile = "sdmc:/switch/PKMswitch/assets/IDs/" + gameName + "_pokemon.txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", poke.id, poke.name.c_str());
                    fclose(outf);
                }
            }
        }
    }

    romfsExit();
    return 0;
}
