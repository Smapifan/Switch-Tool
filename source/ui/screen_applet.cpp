#include "screen_applet.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

void renderAppletWarn(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Full-screen background
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##applet_bg", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Centred warning icon area
    ImGui::SetWindowFontScale(2.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.10f, 1.0f));
    const char* icon = "  ⚠";
    float iconW = ImGui::CalcTextSize(icon).x;
    ImGui::SetCursorPosX((io.DisplaySize.x - iconW) * 0.5f);
    ImGui::SetCursorPosY(io.DisplaySize.y * 0.18f);
    ImGui::Text("%s", icon);
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    // Title
    ImGui::SetWindowFontScale(1.4f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    const char* title = I18n::t("applet_title");
    float titleW = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((io.DisplaySize.x - titleW) * 0.5f);
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Message (centred, wrapped)
    const float msgW = 820.0f;
    ImGui::SetCursorPosX((io.DisplaySize.x - msgW) * 0.5f);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + msgW);
    ImGui::TextWrapped("%s", I18n::t("applet_msg"));
    ImGui::PopTextWrapPos();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Only Exit – usage is blocked in applet mode
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.50f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.70f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
    float btnW = 260.0f;
    ImGui::SetCursorPosX((io.DisplaySize.x - btnW) * 0.5f);
    if (ImGui::Button(I18n::t("applet_exit"), {btnW, 50.0f}))
        state.shouldExit = true;
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));
    const char* hint = I18n::t("applet_hint");
    float hintW = ImGui::CalcTextSize(hint).x;
    ImGui::SetCursorPosX((io.DisplaySize.x - hintW) * 0.5f);
    ImGui::Text("%s", hint);
    ImGui::PopStyleColor();

    ImGui::End();
}

} // namespace UI
