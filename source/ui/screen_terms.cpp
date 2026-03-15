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

    // Reserve space for: hint text row (~22 px) + two Spacing() calls (~8 px) + button row (45 px)
    // plus the window's bottom padding so buttons are never clipped.
    const float buttonRowH = 90.0f;
    float scrollH = io.DisplaySize.y - ImGui::GetCursorPosY()
                    - buttonRowH - ImGui::GetStyle().WindowPadding.y;
    if (scrollH < 80.0f) scrollH = 80.0f;

    // Auto-focus the scroll child on first entry so controller scrolling
    // works immediately without the user having to click/select it first.
    static bool s_focusChild = true;
    if (s_focusChild) {
        ImGui::SetNextWindowFocus();
        s_focusChild = false;
    }

    ImGui::BeginChild("##terms_scroll", {0.0f, scrollH}, true);
    ImGui::TextWrapped("%s", I18n::t("terms_text"));
    ImGui::Spacing();
    ImGui::Spacing();

    const float scrollY    = ImGui::GetScrollY();
    const float scrollMaxY = ImGui::GetScrollMaxY();
    // Considered "at bottom" when there is no overflow, or when within
    // SCROLL_BOTTOM_EPSILON pixels of the maximum scroll position.
    static constexpr float SCROLL_BOTTOM_EPSILON = 4.0f;
    const bool  atBottom   = (scrollMaxY <= 1.0f ||
                               scrollY >= scrollMaxY - SCROLL_BOTTOM_EPSILON);

    // Explicit manual scrolling with DPad Up/Down and left stick.
    // Accumulate a single delta so simultaneous up+down presses cancel out.
    // IsKeyDown gives per-frame movement; 8 px/frame ≈ 480 px/s at 60 fps.
    static constexpr float SCROLL_STEP = 8.0f;
    if (ImGui::IsWindowFocused()) {
        float scrollDelta = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown) ||
            ImGui::IsKeyDown(ImGuiKey_GamepadLStickDown))
            scrollDelta += SCROLL_STEP;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadDpadUp) ||
            ImGui::IsKeyDown(ImGuiKey_GamepadLStickUp))
            scrollDelta -= SCROLL_STEP;
        if (scrollDelta != 0.0f)
            ImGui::SetScrollY(scrollY + scrollDelta);
    }

    ImGui::EndChild();

    ImGui::Spacing();

    // Hint shown until the user reaches the bottom
    if (!atBottom) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.2f, 1.0f));
        ImGui::Text("  ▼  %s  ▼", I18n::t("terms_scroll_hint"));
        ImGui::PopStyleColor();
    } else {
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Accept button – only active after reaching the bottom.
    if (!atBottom) ImGui::BeginDisabled();
    const bool btnAccept = ImGui::Button(I18n::t("terms_accept"), {220.0f, 45.0f});
    if (!atBottom) ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    const bool btnDecline = ImGui::Button(I18n::t("terms_decline"), {220.0f, 45.0f});
    ImGui::PopStyleColor(3);

    // Gamepad shortcuts grouped here with button logic:
    // A (GamepadFaceDown) accepts once the user has scrolled to the bottom,
    // avoiding the need to navigate focus from the scroll area to the button.
    // B (GamepadFaceRight) declines/backs out from anywhere on this screen.
    const bool keyAccept  = atBottom &&
                            ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false);
    const bool keyDecline = ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false);

    if (btnAccept || keyAccept) {
        state.termsAccepted = true;
        if (!state.fullRamMode) {
            state.screen = AppScreen::APPLET_WARN;
        } else {
            state.screen = AppScreen::USER_SELECT;
        }
    }
    if (btnDecline || keyDecline)
        state.shouldExit = true;

    ImGui::End();
}

} // namespace UI
