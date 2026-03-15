#include "screen_main.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

// ── Tab implementations (placeholders, to be filled later) ─────────────────

static void renderBoxTab() {
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("no_save"));
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    if (ImGui::Button(I18n::t("save_load"), {200.0f, 40.0f})) {
        // TODO: open file browser / save loader
    }
    ImGui::SameLine();
    if (ImGui::Button(I18n::t("save_export"), {200.0f, 40.0f})) {
        // TODO: export current save
    }
    ImGui::Spacing();
    ImGui::TextDisabled("%s", I18n::t("wip"));
}

static void renderWipTab(const char* tabKey) {
    ImGui::Spacing();
    ImGui::TextDisabled("[%s]  %s", tabKey, I18n::t("wip"));
}

// ── Main render ─────────────────────────────────────────────────────────────

void renderMainMenu(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // ── Menu bar height reserve ──────────────────────────────────────────
    const float statusBarH = 28.0f;

    // ── Full-screen window ───────────────────────────────────────────────
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##main", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // ── Title / header bar ───────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("  %s", I18n::t("app_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Plugin info on the same line (right-aligned)
    if (state.plugin.valid) {
        std::string info = std::string(I18n::t("plugin_info"))
                         + state.plugin.pluginId
                         + "  "
                         + I18n::t("plugin_version")
                         + std::to_string(state.plugin.version);
        float textW = ImGui::CalcTextSize(info.c_str()).x;
        ImGui::SameLine(io.DisplaySize.x - textW - 20.0f);
        ImGui::TextDisabled("%s", info.c_str());
    }

    ImGui::Separator();

    // ── Tab bar ──────────────────────────────────────────────────────────
    float tabAreaH = io.DisplaySize.y - ImGui::GetCursorPosY() - statusBarH - 4.0f;
    if (tabAreaH < 100.0f) tabAreaH = 100.0f;

    ImGui::BeginChild("##tab_area", {0.0f, tabAreaH}, false);

    if (ImGui::BeginTabBar("##main_tabs", ImGuiTabBarFlags_None)) {

        if (ImGui::BeginTabItem(I18n::t("tab_box"))) {
            renderBoxTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_party"))) {
            renderWipTab("tab_party");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_items"))) {
            renderWipTab("tab_items");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_trainer"))) {
            renderWipTab("tab_trainer");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_pokedex"))) {
            renderWipTab("tab_pokedex");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(I18n::t("tab_misc"))) {
            renderWipTab("tab_misc");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();

    // ── Status bar ───────────────────────────────────────────────────────
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    if (state.fullRamMode)
        ImGui::Text("  %s  |  [+] Exit", I18n::t("status_ready"));
    else
        ImGui::Text("  %s (applet mode)  |  [+] Exit", I18n::t("status_ready"));
    ImGui::PopStyleColor();

    ImGui::End();
}

} // namespace UI
