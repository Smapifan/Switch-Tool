#include "screen_game.hpp"
#include "../i18n.hpp"
#include "../games.hpp"
#include "../save_backup.hpp"

#include <imgui.h>
#include <switch.h>
#include <cstring>
#include <string>
#include <vector>

namespace UI {

// ── Cache of games the current user has saves for ────────────────────────────

struct AvailableGame {
    const Games::GameInfo* info;
};

static std::vector<AvailableGame> s_available;
static bool     s_gamesLoaded   = false;
static uint8_t  s_lastUid[0x10] = {};

/// Check whether save data exists for titleId / uid combo.
static bool saveExists(uint64_t titleId, const uint8_t* uidBytes) {
    AccountUid uid{};
    memcpy(uid.uid, uidBytes, sizeof(uid.uid));

    // Try to mount; if it succeeds, the save exists.
    Result rc = fsdevMountSaveData("_chk_save", titleId, uid);
    if (R_SUCCEEDED(rc)) {
        fsdevUnmountDevice("_chk_save");
        return true;
    }
    return false;
}

static void loadAvailableGames(const AppState& state) {
    s_available.clear();
    s_gamesLoaded = true;
    memcpy(s_lastUid, state.selectedUser.uid, sizeof(s_lastUid));

    for (size_t i = 0; i < Games::GAME_COUNT; ++i) {
        if (saveExists(Games::GAMES[i].titleId, state.selectedUser.uid)) {
            s_available.push_back({&Games::GAMES[i]});
        }
    }
}

// ── Pending backup state ─────────────────────────────────────────────────────
static bool        s_backupInProgress = false;
static std::string s_backupResult;
static bool        s_backupOk        = false;

// ── Render ────────────────────────────────────────────────────────────────────

void renderGameSelect(AppState& state) {
    // Reload if user changed or not yet loaded
    if (!s_gamesLoaded ||
        memcmp(s_lastUid, state.selectedUser.uid, sizeof(s_lastUid)) != 0)
    {
        loadAvailableGames(state);
    }

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##game_select", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("  %s", I18n::t("game_select_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Active user info
    ImGui::SameLine(io.DisplaySize.x - 320.0f);
    ImGui::TextDisabled("%s: %s", I18n::t("user_label"), state.selectedUser.name);
    ImGui::Separator();
    ImGui::Spacing();

    // Backup result notice (from previous selection)
    if (!s_backupResult.empty()) {
        if (s_backupOk) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s %s", I18n::t("backup_ok"), s_backupResult.c_str());
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", I18n::t("backup_fail"));
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    if (s_available.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", I18n::t("game_none_found"));
        ImGui::PopStyleColor();
    } else {
        ImGui::TextDisabled("%s", I18n::t("game_select_hint"));
        ImGui::Spacing();

        // Game list
        for (auto& ag : s_available) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.15f, 0.30f, 0.50f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(0.25f, 0.45f, 0.70f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                ImVec4(0.35f, 0.55f, 0.80f, 1.0f));

            // Show game icon placeholder + name
            char label[128];
            // Icon placeholder: use a coloured square [■] + game name
            snprintf(label, sizeof(label), "  ■  %s##game_%s",
                     ag.info->displayName, ag.info->shortName);

            if (ImGui::Button(label, {700.0f, 60.0f})) {
                state.selectedGame = ag.info;

                // Create backup FIRST
                std::string backupPath;
                s_backupOk     = Backup::createSaveBackup(state, backupPath);
                s_backupResult = backupPath;

                // Advance to main editor
                state.gameLoaded = true;
                s_gamesLoaded    = false; // reset for next visit
                state.screen     = AppScreen::MAIN_MENU;
            }
            ImGui::PopStyleColor(3);
            ImGui::Spacing();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Back / refresh / exit
    if (ImGui::Button(I18n::t("btn_back"), {160.0f, 40.0f})) {
        state.userSelected = false;
        s_gamesLoaded      = false;
        s_backupResult.clear();
        state.screen = AppScreen::USER_SELECT;
    }
    ImGui::SameLine();
    if (ImGui::Button(I18n::t("btn_refresh"), {160.0f, 40.0f})) {
        s_gamesLoaded = false;
        s_backupResult.clear();
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button(I18n::t("applet_exit"), {160.0f, 40.0f}))
        state.shouldExit = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
}

} // namespace UI
