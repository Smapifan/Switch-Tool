#include <switch.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "i18n.hpp"
#include "plugin_check.hpp"
#include "app_state.hpp"
#include "asset_loader.hpp"
#include "ui/screen_plugin_error.hpp"
#include "ui/screen_terms.hpp"
#include "ui/screen_applet.hpp"
#include "ui/screen_user.hpp"
#include "ui/screen_game.hpp"
#include "ui/screen_main.hpp"

// ── Constants ──────────────────────────────────────────────────────────────
static constexpr int  SCREEN_W    = 1280;
static constexpr int  SCREEN_H    = 720;
static constexpr int  FONT_SIZE   = 22;
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
    accountInitialize(AccountServiceType_Application);
    nifmInitialize(NifmServiceType_User);

    // ── Detect applet / full-RAM mode ──────────────────────────────────
    AppletType appletType = appletGetAppletType();
    bool fullRam = (appletType == AppletType_Application ||
                    appletType == AppletType_SystemApplet);

    // ── Resolve NRO directory ──────────────────────────────────────────
    std::string nroDir;
    if (argc > 0 && argv[0] && argv[0][0] != '\0') {
        std::string path(argv[0]);
        for (char& c : path) if (c == '\\') c = '/';
        size_t slash = path.rfind('/');
        if (slash != std::string::npos)
            nroDir = path.substr(0, slash + 1);
    }
    if (nroDir.empty()) nroDir = "sdmc:/switch/PKMswitch/";

    // ── Load translations ──────────────────────────────────────────────
    // Load per-language JSON files from the romfs:/ i18n/ directory.
    I18n::loadDirectory("romfs:/i18n");
    I18n::detectSystemLanguage();

    // ── Locate and validate the plugin ────────────────────────────────
    std::string pluginDir = Plugin::resolvePluginDir(argc > 0 ? argv[0] : nullptr);
    Plugin::Info pluginInfo;
    bool pluginOk = Plugin::validate(pluginDir.c_str(), pluginInfo);
    if (!pluginOk) pluginInfo.dir = pluginDir;

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
    io.IniFilename  = nullptr;

    // Load system font
    PlFontData fontData{};
    if (R_SUCCEEDED(plGetSharedFontByType(&fontData, PlSharedFontType_Standard))
        && fontData.address && fontData.size > 0)
    {
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

    // ── Build initial AppState ─────────────────────────────────────────
    AppState appState{};
    appState.fullRamMode   = fullRam;
    appState.plugin        = pluginInfo;
    appState.termsAccepted = loadTermsAccepted();
    appState.nroDir        = nroDir;

    // ── Determine starting screen ──────────────────────────────────────
    if (!pluginOk) {
        appState.screen = AppScreen::PLUGIN_ERROR;
    } else if (!appState.termsAccepted) {
        appState.screen = AppScreen::TERMS;
    } else if (!fullRam) {
        appState.screen = AppScreen::APPLET_WARN;
    } else {
        // Always run the asset-init phase to check for updates and scan IDs.json files.
        appState.screen = AppScreen::ASSET_INIT;
    }

    // ── Main loop ──────────────────────────────────────────────────────
    bool termsSaved = appState.termsAccepted;
    bool assetRunStarted = false;

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

        // Run asset init (blocking) when entering ASSET_INIT screen
        if (appState.screen == AppScreen::ASSET_INIT && !assetRunStarted) {
            assetRunStarted = true;
            // Render one frame first so the progress screen is visible
            // then run (will block this frame with progress updates)
        }

        // New ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Dispatch to current screen
        switch (appState.screen) {
            case AppScreen::PLUGIN_ERROR:
                UI::renderPluginError(appState);
                break;
            case AppScreen::TERMS:
                UI::renderTerms(appState);
                break;
            case AppScreen::ASSET_INIT:
                AssetLoader::renderInitScreen(appState);
                // After first render, run the blocking init
                if (assetRunStarted && !appState.assetsReady) {
                    // Render frame then run init
                    ImGui::Render();
                    SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
                    SDL_RenderClear(renderer);
                    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
                    SDL_RenderPresent(renderer);
                    // Run blocking asset init
                    AssetLoader::run(appState);
                    if (appState.assetsReady)
                        appState.screen = AppScreen::USER_SELECT;
                    // Continue loop (will re-render)
                    continue;
                }
                break;
            case AppScreen::APPLET_WARN:
                UI::renderAppletWarn(appState);
                break;
            case AppScreen::USER_SELECT:
                UI::renderUserSelect(appState);
                break;
            case AppScreen::GAME_SELECT:
                UI::renderGameSelect(appState);
                break;
            case AppScreen::MAIN_MENU:
                UI::renderMainMenu(appState);
                break;
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

    nifmExit();
    accountExit();
    setExit();
    plExit();
    romfsExit();

    return 0;
}
