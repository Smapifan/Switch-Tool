#include "screen_user.hpp"
#include "../i18n.hpp"
#include <imgui.h>
#include <switch.h>
#include <cstring>
#include <vector>

namespace UI {

// ── User list cache (refreshed once per visit to this screen) ────────────────

struct CachedUser {
    UserAccount info;
};

static std::vector<CachedUser> s_users;
static bool s_usersLoaded = false;

/// Enumerate all user accounts on this console.
static void loadUsers() {
    s_users.clear();
    s_usersLoaded = true;

    AccountUid uids[ACC_USER_LIST_SIZE]{};
    s32 count = 0;

    if (R_FAILED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &count)))
        return;

    for (s32 i = 0; i < count; ++i) {
        AccountProfile profile{};
        if (R_FAILED(accountGetProfile(&profile, uids[i])))
            continue;

        AccountProfileBase base{};
        AccountUserData    userData{};
        if (R_FAILED(accountProfileGet(&profile, &userData, &base))) {
            accountProfileClose(&profile);
            continue;
        }
        accountProfileClose(&profile);

        CachedUser cu{};
        // Copy UID bytes
        static_assert(sizeof(uids[i].uid) == sizeof(cu.info.uid), "UID size mismatch");
        memcpy(cu.info.uid, uids[i].uid, sizeof(cu.info.uid));
        // Copy username (base.nickname is a null-terminated UTF-8 string, max 32 chars)
        strncpy(cu.info.name, base.nickname, sizeof(cu.info.name) - 1);
        cu.info.name[sizeof(cu.info.name) - 1] = '\0';
        cu.info.valid = true;
        s_users.push_back(cu);
    }
}

// ── Render ───────────────────────────────────────────────────────────────────

void renderUserSelect(AppState& state) {
    // Reload users each time we enter this screen
    if (!s_usersLoaded)
        loadUsers();

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##user_select", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("  %s", I18n::t("user_select_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("%s", I18n::t("user_select_hint"));
    ImGui::Spacing();

    if (s_users.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", I18n::t("user_none_found"));
        ImGui::PopStyleColor();
    } else {
        // User grid – 4 per row
        const float btnSize = 220.0f;
        const float btnH    = 60.0f;
        int col = 0;
        for (auto& cu : s_users) {
            if (col > 0) ImGui::SameLine();

            // Highlight button
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(0.20f, 0.40f, 0.65f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(0.30f, 0.55f, 0.80f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                ImVec4(0.40f, 0.65f, 0.90f, 1.0f));

            // Use name + first two UID bytes as unique button id
            char btnLabel[64];
            snprintf(btnLabel, sizeof(btnLabel), "%s##user_%02x%02x",
                     cu.info.name, cu.info.uid[0], cu.info.uid[1]);

            if (ImGui::Button(btnLabel, {btnSize, btnH})) {
                state.selectedUser = cu.info;
                state.userSelected = true;
                s_usersLoaded = false; // reset for next visit
                state.screen = AppScreen::GAME_SELECT;
            }
            ImGui::PopStyleColor(3);

            ++col;
            if (col >= 4) {
                col = 0; // start a new row
                ImGui::Spacing();
            }
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Refresh / Exit row
    if (ImGui::Button(I18n::t("btn_refresh"), {180.0f, 40.0f})) {
        s_usersLoaded = false;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button(I18n::t("applet_exit"), {180.0f, 40.0f}))
        state.shouldExit = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
}

} // namespace UI
