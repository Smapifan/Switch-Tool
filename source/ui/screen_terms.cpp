#include "screen_terms.hpp"
#include "../i18n.hpp"
#include <imgui.h>
#include <switch.h>

namespace UI {

static float clampf(float v, float a, float b) {
    return (v < a) ? a : (v > b) ? b : v;
}

void renderTerms(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    // Controller buttons
    u64 kDown = 0;
    {
        PadState pad;
        padInitializeDefault(&pad);
        padUpdate(&pad);
        kDown = padGetButtonsDown(&pad);
    }

    // Full-screen window
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 10.0f));
    ImGui::Begin("##terms", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings);

    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.25f);
    ImGui::Text("  %s", I18n::t("terms_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // Sticky footer space
    const float footerH = 110.0f;

    float scrollH = io.DisplaySize.y - ImGui::GetCursorPosY() - footerH;
    if (scrollH < 120.0f) scrollH = 120.0f;

    ImGui::BeginChild("##terms_scroll", {0.0f, scrollH}, true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    // Auto-focus scroll area so dpad/stick scrolling works immediately
    static bool focusedOnce = false;
    if (!focusedOnce) {
        ImGui::SetKeyboardFocusHere();
        focusedOnce = true;
    }

    ImGui::TextWrapped("%s", I18n::t("terms_text"));
    ImGui::Spacing();
    ImGui::Spacing();

    // Manual scrolling
    const float lineStep = 36.0f;
    const float pageStep = scrollH * 0.80f;

    float stick = io.NavInputs[ImGuiNavInput_LStickDown] - io.NavInputs[ImGuiNavInput_LStickUp];
    if (stick != 0.0f) {
        ImGui::SetScrollY(ImGui::GetScrollY() + stick * 18.0f);
    }
    if (kDown & HidNpadButton_Down) ImGui::SetScrollY(ImGui::GetScrollY() + lineStep);
    if (kDown & HidNpadButton_Up)   ImGui::SetScrollY(ImGui::GetScrollY() - lineStep);
    if (kDown & HidNpadButton_L)    ImGui::SetScrollY(ImGui::GetScrollY() - pageStep);
    if (kDown & HidNpadButton_R)    ImGui::SetScrollY(ImGui::GetScrollY() + pageStep);

    float scrollMaxY = ImGui::GetScrollMaxY();
    ImGui::SetScrollY(clampf(ImGui::GetScrollY(), 0.0f, scrollMaxY));

    float scrollY = ImGui::GetScrollY();
    static constexpr float SCROLL_BOTTOM_EPSILON = 6.0f;
    bool atBottom = (scrollMaxY <= 1.0f || scrollY >= scrollMaxY - SCROLL_BOTTOM_EPSILON);

    ImGui::EndChild();

    ImGui::Spacing();

    if (!atBottom) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.2f, 1.0f));
        ImGui::Text("  ▼  %s  ▼", I18n::t("terms_scroll_hint"));
        ImGui::PopStyleColor();
    } else {
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Accept only at bottom
    if (!atBottom) ImGui::BeginDisabled();

    bool accept = ImGui::Button(I18n::t("terms_accept"), {260.0f, 48.0f});
    if (atBottom && (kDown & HidNpadButton_A)) accept = true;

    if (accept) {
        state.termsAccepted = true;
        state.screen = AppScreen::ASSET_INIT; // Always asset phase
        focusedOnce = false;
    }

    if (!atBottom) ImGui::EndDisabled();

    ImGui::SameLine();

    // Decline (B)
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.50f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.08f, 0.08f, 1.0f));

    bool decline = ImGui::Button(I18n::t("terms_decline"), {260.0f, 48.0f});
    if (kDown & HidNpadButton_B) decline = true;

    if (decline) {
        state.shouldExit = true;
        focusedOnce = false;
    }

    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace UI
