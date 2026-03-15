#include "screen_applet.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

void renderAppletWarn(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Dim background
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##applet_bg", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoInputs     |
                 ImGuiWindowFlags_NoSavedSettings);
    ImGui::End();

    // Dialog
    const float dlgW = 820.0f, dlgH = 380.0f;
    ImGui::SetNextWindowPos(
        {(io.DisplaySize.x - dlgW) * 0.5f, (io.DisplaySize.y - dlgH) * 0.5f},
        ImGuiCond_Always);
    ImGui::SetNextWindowSize({dlgW, dlgH});
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.80f, 0.55f, 0.05f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(0.70f, 0.45f, 0.05f, 1.0f));

    ImGui::Begin(I18n::t("applet_title"), nullptr,
                 ImGuiWindowFlags_NoResize   |
                 ImGuiWindowFlags_NoMove     |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::Spacing();
    ImGui::TextWrapped("%s", I18n::t("applet_msg"));
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,       ImVec4(0.20f, 0.55f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.70f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.80f, 0.40f, 1.0f));
    if (ImGui::Button(I18n::t("applet_continue"), {320.0f, 45.0f}))
        state.screen = AppScreen::MAIN_MENU;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,       ImVec4(0.50f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
    if (ImGui::Button(I18n::t("applet_exit"), {200.0f, 45.0f}))
        state.shouldExit = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleColor(2);
}

} // namespace UI
