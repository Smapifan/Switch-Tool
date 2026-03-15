#include "screen_terms.hpp"
#include "../i18n.hpp"
#include <imgui.h>

namespace UI {

void renderTerms(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Full-screen window
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##terms", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Title bar row
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("  %s", I18n::t("terms_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // Scrollable text region — leave room for the button row at the bottom
    const float buttonRowH = 70.0f;
    float scrollH = io.DisplaySize.y - ImGui::GetCursorPosY() - buttonRowH;
    if (scrollH < 80.0f) scrollH = 80.0f;

    ImGui::BeginChild("##terms_scroll", {0.0f, scrollH}, true,
                      ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextWrapped("%s", I18n::t("terms_text"));
    ImGui::Spacing();
    ImGui::Spacing();

    float scrollY    = ImGui::GetScrollY();
    float scrollMaxY = ImGui::GetScrollMaxY();
    // Considered "at bottom" when there is no overflow, or when within SCROLL_BOTTOM_EPSILON
    // pixels of the maximum scroll position.
    static constexpr float SCROLL_BOTTOM_EPSILON = 4.0f;
    bool  atBottom   = (scrollMaxY <= 1.0f || scrollY >= scrollMaxY - SCROLL_BOTTOM_EPSILON);

    ImGui::EndChild();

    ImGui::Spacing();

    // Hint when not yet at the bottom
    if (!atBottom) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.2f, 1.0f));
        ImGui::Text("  ▼  %s  ▼", I18n::t("terms_scroll_hint"));
        ImGui::PopStyleColor();
    } else {
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Accept button – only active after reaching the bottom
    if (!atBottom) ImGui::BeginDisabled();
    if (ImGui::Button(I18n::t("terms_accept"), {220.0f, 45.0f})) {
        state.termsAccepted = true;
        state.screen = state.fullRamMode ? AppScreen::MAIN_MENU
                                         : AppScreen::APPLET_WARN;
    }
    if (!atBottom) ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button(I18n::t("terms_decline"), {220.0f, 45.0f}))
        state.shouldExit = true;
    ImGui::PopStyleColor(3);

    ImGui::End();
}

} // namespace UI
