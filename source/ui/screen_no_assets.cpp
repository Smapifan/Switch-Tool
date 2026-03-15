#include "screen_no_assets.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

void renderNoAssets(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Full-screen dark background
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##no_assets_bg", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Centred error dialog
    const float dlgW = 900.0f, dlgH = 420.0f;
    ImGui::SetNextWindowPos(
        {(io.DisplaySize.x - dlgW) * 0.5f, (io.DisplaySize.y - dlgH) * 0.5f},
        ImGuiCond_Always);
    ImGui::SetNextWindowSize({dlgW, dlgH});
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.70f, 0.40f, 0.05f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(0.60f, 0.30f, 0.03f, 1.0f));

    ImGui::Begin(I18n::t("no_assets_title"), nullptr,
                 ImGuiWindowFlags_NoResize     |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoCollapse   |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::Spacing();
    ImGui::TextWrapped("%s", I18n::t("no_assets_msg"));
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(I18n::t("no_assets_exit"), {200.0f, 40.0f}))
        state.shouldExit = true;

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::End(); // background window
}

} // namespace UI
