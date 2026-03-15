#include "screen_plugin_error.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

void renderPluginError(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Full-screen dark background
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##plugin_error_bg", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Centred error dialog
    const float dlgW = 900.0f, dlgH = 460.0f;
    ImGui::SetNextWindowPos(
        {(io.DisplaySize.x - dlgW) * 0.5f, (io.DisplaySize.y - dlgH) * 0.5f},
        ImGuiCond_Always);
    ImGui::SetNextWindowSize({dlgW, dlgH});
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(0.65f, 0.10f, 0.10f, 1.0f));

    ImGui::Begin(I18n::t("plugin_missing_title"), nullptr,
                 ImGuiWindowFlags_NoResize     |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoCollapse   |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::Spacing();
    ImGui::TextWrapped("%s", I18n::t("plugin_missing_msg"));
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Show the path the app actually looked in
    ImGui::TextDisabled("Searched in: %s", state.plugin.dir.c_str());
    ImGui::Spacing();

    if (ImGui::Button(I18n::t("plugin_missing_exit"), {200.0f, 40.0f}))
        state.shouldExit = true;

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::End(); // background window
}

} // namespace UI
