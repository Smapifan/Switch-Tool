#include <switch.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <string>
#include <cstdio>
#include <sys/stat.h>

#include "i18n.hpp"
#include "plugin_check.hpp"
#include "app_state.hpp"
#include "ui/screen_plugin_error.hpp"
#include "ui/screen_terms.hpp"
#include "ui/screen_applet.hpp"
#include "ui/screen_main.hpp"

// ── Constants ──────────────────────────────────────────────────────────────
static constexpr int  SCREEN_W    = 1280;
static constexpr int  SCREEN_H    = 720;
static constexpr int  FONT_SIZE   = 22;
// Path within RomFS where translations live
static constexpr const char* I18N_PATH = "romfs:/i18n.json";
// Config file that remembers whether the user accepted the terms
static constexpr const char* TERMS_FILE = "sdmc:/switch/PKMswitch/.terms_accepted";

// ── Terms persistence ──────────────────────────────────────────────────────
static bool loadTermsAccepted() {
    FILE* f = fopen(TERMS_FILE, "r");
    if (!f) return false;
    fclose(f);
    return true;
}

static void saveTermsAccepted() {
    // Ensure directory exists
    mkdir("sdmc:/switch/PKMswitch", 0777);
    FILE* f = fopen(TERMS_FILE, "w");
    if (f) { fputs("1", f); fclose(f); }
}

// ── Entry point ────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {

    // ── libnx service init ─────────────────────────────────────────────
    romfsInit();
    plInitialize(PlServiceType_User);
    setInitialize();

    // ── Detect applet / full-RAM mode ──────────────────────────────────
    AppletType appletType = appletGetAppletType();
    bool fullRam = (appletType == AppletType_Application ||
                    appletType == AppletType_SystemApplet);

    // ── Load translations ──────────────────────────────────────────────
    I18n::load(I18N_PATH);
    I18n::detectSystemLanguage();

    // ── Locate and validate the plugin ────────────────────────────────
    std::string pluginDir = Plugin::resolvePluginDir(argc > 0 ? argv[0] : nullptr);
    Plugin::Info pluginInfo;
    bool pluginOk = Plugin::validate(pluginDir.c_str(), pluginInfo);
    if (!pluginOk) pluginInfo.dir = pluginDir; // Keep the path for the error screen

    // ── SDL2 init ──────────────────────────────────────────────────────
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    SDL_Window* window = SDL_CreateWindow(
        "PKMswitch",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // ── Dear ImGui init ────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename  = nullptr; // Disable imgui.ini on Switch

    // Load system font (no font file needed)
    PlFontData fontData{};
    if (R_SUCCEEDED(plGetSharedFontByType(&fontData, PlSharedFontType_Standard))
        && fontData.address && fontData.size > 0)
    {
        // ImGui takes a copy of the font data
        io.Fonts->AddFontFromMemoryTTF(
            fontData.address,
            static_cast<int>(fontData.size),
            static_cast<float>(FONT_SIZE));
    } else {
        io.Fonts->AddFontDefault();
    }

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding      = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_Header]   = ImVec4(0.20f, 0.35f, 0.55f, 1.0f);

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // ── Determine initial screen ───────────────────────────────────────
    AppState appState{};
    appState.fullRamMode  = fullRam;
    appState.plugin       = pluginInfo;
    appState.termsAccepted = loadTermsAccepted();

    if (!pluginOk) {
        appState.screen = AppScreen::PLUGIN_ERROR;
    } else if (!appState.termsAccepted) {
        appState.screen = AppScreen::TERMS;
    } else if (!fullRam) {
        appState.screen = AppScreen::APPLET_WARN;
    } else {
        appState.screen = AppScreen::MAIN_MENU;
    }

    // ── Main loop ──────────────────────────────────────────────────────
    // Track whether we've already written the terms-accepted marker file this session.
    // Initialise to true if terms were loaded from disk (already accepted previously).
    bool termsSaved = appState.termsAccepted;
    while (appletMainLoop() && !appState.shouldExit) {
        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                appState.shouldExit = true;
        }

        // Detect [+] (Plus button) → exit from anywhere
        u64 kDown = 0;
        {
            PadState pad;
            padInitializeDefault(&pad);
            padUpdate(&pad);
            kDown = padGetButtonsDown(&pad);
        }
        if (kDown & HidNpadButton_Plus)
            appState.shouldExit = true;

        // Persist terms once if just accepted this session
        if (appState.termsAccepted && !termsSaved) {
            saveTermsAccepted();
            termsSaved = true;
        }

        // New ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Dispatch to current screen
        switch (appState.screen) {
            case AppScreen::PLUGIN_ERROR: UI::renderPluginError(appState); break;
            case AppScreen::TERMS:        UI::renderTerms(appState);       break;
            case AppScreen::APPLET_WARN:  UI::renderAppletWarn(appState);  break;
            case AppScreen::MAIN_MENU:    UI::renderMainMenu(appState);    break;
            case AppScreen::EXIT:
                appState.shouldExit = true;
                break;
        }

        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // ── Cleanup ────────────────────────────────────────────────────────
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    setExit();
    plExit();
    romfsExit();

    return 0;
}
