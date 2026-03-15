#include "screen_main.hpp"
#include "../i18n.hpp"
#include "../games.hpp"
#include <imgui.h>
#include <cstring>

namespace UI {

// ── Editor left panel ─────────────────────────────────────────────────────────
// Shows the currently selected Pokémon's details.

static void renderPokemonEditor() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.14f, 1.0f));
    ImGui::BeginChild("##poke_editor", {0, 0}, true);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::Text("  %s", I18n::t("editor_title"));
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Placeholder Pokémon data ───────────────────────────────────────────
    ImGui::TextDisabled("%-10s", I18n::t("editor_species"));
    ImGui::SameLine(); ImGui::Text("Pikachu  (#025)");

    ImGui::TextDisabled("%-10s", I18n::t("editor_level"));
    ImGui::SameLine(); ImGui::Text("50");

    ImGui::TextDisabled("%-10s", I18n::t("editor_nature"));
    ImGui::SameLine(); ImGui::Text("Timid");

    ImGui::TextDisabled("%-10s", I18n::t("editor_ability"));
    ImGui::SameLine(); ImGui::Text("Static");

    ImGui::TextDisabled("%-10s", I18n::t("editor_item"));
    ImGui::SameLine(); ImGui::Text("Light Ball");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // IVs / EVs
    ImGui::Text("%s", I18n::t("editor_ivs"));
    static int ivs[6] = {31, 31, 31, 31, 31, 31};
    const char* statNames[6] = {"HP","Atk","Def","SpA","SpD","Spe"};
    for (int i = 0; i < 6; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "##iv%d", i);
        ImGui::Text("%-5s", statNames[i]); ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::SliderInt(lbl, &ivs[i], 0, 31);
        if (i < 5) ImGui::SameLine();
    }

    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("editor_evs"));
    static int evs[6] = {0, 0, 0, 252, 0, 252};
    for (int i = 0; i < 6; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "##ev%d", i);
        ImGui::Text("%-5s", statNames[i]); ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::SliderInt(lbl, &evs[i], 0, 252);
        if (i < 5) ImGui::SameLine();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Moves
    ImGui::Text("%s", I18n::t("editor_moves"));
    static char moves[4][64] = {"Thunderbolt", "Volt Tackle", "Iron Tail", "Quick Attack"};
    for (int i = 0; i < 4; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "##move%d", i);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText(lbl, moves[i], sizeof(moves[i]));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // PID / TID / SID
    static char pid[16] = "1A2B3C4D";
    static char tid[8]  = "12345";
    static char sid[8]  = "54321";
    ImGui::TextDisabled("%-6s", "PID");  ImGui::SameLine();
    ImGui::SetNextItemWidth(120); ImGui::InputText("##pid", pid, sizeof(pid));
    ImGui::TextDisabled("%-6s", "TID");  ImGui::SameLine();
    ImGui::SetNextItemWidth(80);  ImGui::InputText("##tid", tid, sizeof(tid));
    ImGui::SameLine();
    ImGui::TextDisabled("%-6s", "SID");  ImGui::SameLine();
    ImGui::SetNextItemWidth(80);  ImGui::InputText("##sid", sid, sizeof(sid));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Legality indicator (placeholder)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
    ImGui::Text("  ✓  %s", I18n::t("legality_ok"));
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,       ImVec4(0.20f, 0.50f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.65f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.75f, 0.40f, 1.0f));
    ImGui::Button(I18n::t("editor_apply"), {-1.0f, 40.0f});
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ── Box view right panel ──────────────────────────────────────────────────────

static void renderBoxView(const AppState& state) {
    const Games::GameInfo* game = state.selectedGame;
    int boxCount  = game ? game->boxCount  : 8;
    int boxSlots  = game ? game->boxSlots  : 30;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
    ImGui::BeginChild("##box_view", {0, 0}, true);

    // Box selector
    static int currentBox = 0;
    ImGui::Text("%s %d / %d", I18n::t("box_label"), currentBox + 1, boxCount);
    ImGui::SameLine();
    if (ImGui::SmallButton("<")) { if (currentBox > 0) --currentBox; }
    ImGui::SameLine();
    if (ImGui::SmallButton(">")) { if (currentBox < boxCount - 1) ++currentBox; }
    ImGui::Separator();
    ImGui::Spacing();

    // Slot grid (6 columns)
    const float slotSz = 48.0f;
    for (int i = 0; i < boxSlots; ++i) {
        if (i > 0 && i % 6 != 0) ImGui::SameLine();
        char lbl[32];
        snprintf(lbl, sizeof(lbl), "??##s%d", i);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.20f, 0.28f, 1.0f));
        ImGui::Button(lbl, {slotSz, slotSz});
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

static void renderPartyView() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
    ImGui::BeginChild("##party_view", {0, 0}, true);
    ImGui::Text("%s", I18n::t("party_label"));
    ImGui::Separator();
    ImGui::Spacing();
    const float slotSz = 56.0f;
    for (int i = 0; i < 6; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "??##p%d", i);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.20f, 0.28f, 1.0f));
        ImGui::Button(lbl, {slotSz, slotSz});
        ImGui::PopStyleColor();
        if (i < 5) ImGui::SameLine();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ── Tab implementations ───────────────────────────────────────────────────────

static void renderBoxTab(AppState& state) {
    float editorW = ImGui::GetContentRegionAvail().x * 0.38f;
    float listW   = ImGui::GetContentRegionAvail().x - editorW - 8.0f;

    ImGui::BeginChild("##box_split_left", {editorW, 0}, false);
    renderPokemonEditor();
    ImGui::EndChild();

    ImGui::SameLine(0, 8);

    ImGui::BeginChild("##box_split_right", {listW, 0}, false);
    renderBoxView(state);
    ImGui::EndChild();
}

static void renderPartyTab() {
    float editorW = ImGui::GetContentRegionAvail().x * 0.38f;
    float listW   = ImGui::GetContentRegionAvail().x - editorW - 8.0f;

    ImGui::BeginChild("##party_split_left", {editorW, 0}, false);
    renderPokemonEditor();
    ImGui::EndChild();

    ImGui::SameLine(0, 8);

    ImGui::BeginChild("##party_split_right", {listW, 0}, false);
    renderPartyView();
    ImGui::EndChild();
}

static void renderItemsTab() {
    // Header row
    ImGui::TextDisabled("%-30s  %-20s  %s",
        I18n::t("items_col_name"),
        I18n::t("items_col_category"),
        I18n::t("items_col_count"));
    ImGui::Separator();

    // Placeholder item rows
    struct Item { const char* name; const char* cat; int cnt; };
    static Item items[] = {
        { "Potion",        "Medicine",   5  },
        { "Super Potion",  "Medicine",   3  },
        { "Poké Ball",     "Balls",      20 },
        { "Great Ball",    "Balls",      10 },
        { "Rare Candy",    "Items",      1  },
        { "TM01",          "TM",         0  },
    };
    static int counts[6] = {5, 3, 20, 10, 1, 0};

    ImGui::BeginChild("##item_list", {0, -80.0f}, false);
    for (int i = 0; i < 6; ++i) {
        char lbl[128];
        snprintf(lbl, sizeof(lbl), "%-30s  %-20s", items[i].name, items[i].cat);
        ImGui::Text("%s", lbl);
        ImGui::SameLine(400.0f);
        char clbl[32]; snprintf(clbl, sizeof(clbl), "##ic%d", i);
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt(clbl, &counts[i]);
        ImGui::SameLine();
        char dlbl[32]; snprintf(dlbl, sizeof(dlbl), "%s##id%d", I18n::t("items_delete"), i);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
        ImGui::SmallButton(dlbl);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Spacing();
    if (ImGui::Button(I18n::t("items_give_all"), {200.0f, 36.0f})) { /* TODO */ }
    ImGui::SameLine();
    static int giveCount = 1;
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("##give_count", &giveCount);
}

static void renderTrainerTab(const AppState& state) {
    ImGui::Spacing();
    if (!state.selectedGame) {
        ImGui::TextDisabled("%s", I18n::t("no_save"));
        return;
    }
    const auto* g = state.selectedGame;

    static char tid[8]  = "00000";
    static char sid[8]  = "00000";
    static char name[32] = "Trainer";
    static int  money   = 9999;

    ImGui::Text("%s", I18n::t("trainer_name")); ImGui::SameLine();
    ImGui::SetNextItemWidth(200); ImGui::InputText("##tr_name", name, sizeof(name));

    ImGui::Text("TID"); ImGui::SameLine();
    ImGui::SetNextItemWidth(80); ImGui::InputText("##tr_tid", tid, sizeof(tid));
    ImGui::SameLine();
    ImGui::Text("SID"); ImGui::SameLine();
    ImGui::SetNextItemWidth(80); ImGui::InputText("##tr_sid", sid, sizeof(sid));

    ImGui::Text("%s", I18n::t("trainer_money")); ImGui::SameLine();
    ImGui::SetNextItemWidth(120); ImGui::InputInt("##tr_money", &money);

    // Game-specific fields
    if (g->hasBP) {
        static int bp = 0;
        ImGui::Text("BP"); ImGui::SameLine();
        ImGui::SetNextItemWidth(100); ImGui::InputInt("##tr_bp", &bp);
    }
    if (g->hasLP) {
        static int lp = 0;
        ImGui::Text("LP"); ImGui::SameLine();
        ImGui::SetNextItemWidth(100); ImGui::InputInt("##tr_lp", &lp);
    }

    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("trainer_badges"));
    ImGui::Spacing();
    static bool badges[8] = {};
    const char* badgeNames[8] = {"Badge 1","Badge 2","Badge 3","Badge 4",
                                  "Badge 5","Badge 6","Badge 7","Badge 8"};
    for (int i = 0; i < 8; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "%s##b%d", badgeNames[i], i);
        ImGui::Checkbox(lbl, &badges[i]);
        if (i % 4 != 3) ImGui::SameLine();
    }
}

static void renderRaidsTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("raids_title"));
    ImGui::Separator();
    ImGui::Spacing();

    static int raidSpecies = 1;
    static int raidStars   = 3;
    static bool eventRaid  = false;

    ImGui::Text("%s", I18n::t("raids_species")); ImGui::SameLine();
    ImGui::SetNextItemWidth(120); ImGui::InputInt("##raid_sp", &raidSpecies);

    ImGui::Text("%s", I18n::t("raids_stars"));   ImGui::SameLine();
    ImGui::SetNextItemWidth(80);  ImGui::SliderInt("##raid_st", &raidStars, 1, 6);

    ImGui::Checkbox(I18n::t("raids_event"), &eventRaid);
    if (eventRaid) {
        ImGui::Spacing();
        ImGui::TextDisabled("%s", I18n::t("raids_event_hint"));
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,       ImVec4(0.20f, 0.50f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.65f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.75f, 0.40f, 1.0f));
    ImGui::Button(I18n::t("raids_inject"), {200.0f, 40.0f});
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("wip"));
}

static void renderDonutsTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("donuts_title"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("donuts_hint"));
    ImGui::Spacing();

    static int donutCounts[4] = {0, 0, 0, 0};
    const char* dims[4] = {"Dimension 1", "Dimension 2", "Dimension 3", "Dimension 4"};
    for (int i = 0; i < 4; ++i) {
        char lbl[32]; snprintf(lbl, sizeof(lbl), "##dn%d", i);
        ImGui::Text("%-14s", dims[i]); ImGui::SameLine();
        ImGui::SetNextItemWidth(100); ImGui::InputInt(lbl, &donutCounts[i]);
    }
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("wip"));
}

static void renderEventsTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("events_title"));
    ImGui::Separator();
    ImGui::Spacing();

    // Mystery Gift section
    ImGui::Text("%s", I18n::t("events_mystery_gift"));
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("events_mg_hint"));
    ImGui::Spacing();
    if (ImGui::Button(I18n::t("events_import_wc"), {220.0f, 36.0f})) { /* TODO */ }
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("wip"));
}

static void renderPokedexTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("tab_pokedex"));
    ImGui::Separator();
    ImGui::Spacing();

    static char search[64] = "";
    ImGui::SetNextItemWidth(300);
    ImGui::InputText(I18n::t("pokedex_search"), search, sizeof(search));
    ImGui::Spacing();

    // Placeholder table
    if (ImGui::BeginTable("##dex_table", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY, {0, -1}))
    {
        ImGui::TableSetupColumn("#",      ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn(I18n::t("pokedex_name"),   ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18n::t("pokedex_status"), ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (int i = 1; i <= 20; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
            ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("???");
            ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("-");
        }
        ImGui::EndTable();
    }
}

static void renderEncounterDbTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("tab_encounter_db"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("encounter_db_hint"));
    ImGui::Spacing();

    if (ImGui::BeginTable("##enc_table", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY, {0, -80.0f}))
    {
        ImGui::TableSetupColumn(I18n::t("encounter_col_species"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18n::t("encounter_col_level"),   ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn(I18n::t("encounter_col_location"),ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18n::t("encounter_col_rate"),    ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableHeadersRow();
        // Placeholder rows
        for (int i = 0; i < 10; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("???");
            ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("-");
            ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("-");
            ImGui::TableSetColumnIndex(3); ImGui::TextDisabled("-");
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Button(I18n::t("encounter_inject"), {220.0f, 36.0f});
}

static void renderEventDbTab() {
    ImGui::Spacing();
    ImGui::Text("%s", I18n::t("tab_event_db"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("event_db_hint"));
    ImGui::Spacing();

    // Predefined notable Pokémon
    struct EventPkmn { const char* name; const char* description; };
    static const EventPkmn events[] = {
        { "Manaphy Egg",        "Gen IV event egg"        },
        { "Shiny Koraidon",     "SV mystery gift"          },
        { "Shiny Miraidon",     "SV mystery gift"          },
        { "Mew (MELMETAL)",     "Pokémon GO transfer"      },
        { "Zarude",             "Movie distribution"       },
    };

    if (ImGui::BeginTable("##evt_table", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY, {0, -80.0f}))
    {
        ImGui::TableSetupColumn(I18n::t("event_col_name"),  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18n::t("event_col_desc"),  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(I18n::t("event_col_inject"),ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();
        for (int i = 0; i < 5; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", events[i].name);
            ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("%s", events[i].description);
            ImGui::TableSetColumnIndex(2);
            char lbl[32]; snprintf(lbl, sizeof(lbl), "%s##ev%d", I18n::t("encounter_inject"), i);
            if (ImGui::SmallButton(lbl)) { /* TODO */ }
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
    ImGui::Spacing();
}

// ── Main render ─────────────────────────────────────────────────────────────────

void renderMainMenu(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    const float statusBarH = 28.0f;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##main", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // ── Header bar ─────────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("  %s", I18n::t("app_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Game + user info (right-aligned)
    if (state.selectedGame && state.selectedUser.valid) {
        char info[128];
        snprintf(info, sizeof(info), "%s  |  %s",
                 state.selectedGame->displayName, state.selectedUser.name);
        float w = ImGui::CalcTextSize(info).x;
        ImGui::SameLine(io.DisplaySize.x - w - 20.0f);
        ImGui::TextDisabled("%s", info);
    } else if (state.plugin.valid) {
        std::string pinfo = std::string(I18n::t("plugin_info")) + state.plugin.pluginId
                          + "  " + I18n::t("plugin_version")
                          + std::to_string(state.plugin.version);
        float pw = ImGui::CalcTextSize(pinfo.c_str()).x;
        ImGui::SameLine(io.DisplaySize.x - pw - 20.0f);
        ImGui::TextDisabled("%s", pinfo.c_str());
    }

    ImGui::Separator();

    // ── Tab area ────────────────────────────────────────────────────────────
    float tabAreaH = io.DisplaySize.y - ImGui::GetCursorPosY() - statusBarH - 4.0f;
    if (tabAreaH < 100.0f) tabAreaH = 100.0f;

    ImGui::BeginChild("##tab_area", {0.0f, tabAreaH}, false);

    if (ImGui::BeginTabBar("##main_tabs")) {

        if (ImGui::BeginTabItem(I18n::t("tab_box"))) {
            renderBoxTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_party"))) {
            renderPartyTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_items"))) {
            renderItemsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_trainer"))) {
            renderTrainerTab(state);
            ImGui::EndTabItem();
        }

        // Show Raids only for SwSh / SV
        bool showRaids = state.selectedGame && state.selectedGame->hasRaids;
        if (showRaids || !state.gameLoaded) {
            if (ImGui::BeginTabItem(I18n::t("tab_raids"))) {
                renderRaidsTab();
                ImGui::EndTabItem();
            }
        }

        // Show Donuts only for ZA
        bool showDonuts = state.selectedGame && state.selectedGame->hasDonuts;
        if (showDonuts || !state.gameLoaded) {
            if (ImGui::BeginTabItem(I18n::t("tab_donuts"))) {
                renderDonutsTab();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem(I18n::t("tab_events"))) {
            renderEventsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_pokedex"))) {
            renderPokedexTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_encounter_db"))) {
            renderEncounterDbTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_event_db"))) {
            renderEventDbTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();

    // ── Status bar ──────────────────────────────────────────────────────────
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    const char* gameStr = state.selectedGame ? state.selectedGame->displayName : "-";
    ImGui::Text("  %s  |  %s  |  [+] %s",
                I18n::t("status_ready"), gameStr, I18n::t("btn_close"));
    ImGui::PopStyleColor();

    ImGui::End();
}

} // namespace UI
