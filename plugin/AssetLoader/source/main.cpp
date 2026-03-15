//---------------------------------------------------------------------------------
// PKMswitch AssetLoader Plugin
// Standalone NRO companion for PKMswitch.nro
//
// Responsibilities:
//  - Check Wi-Fi availability (nifmInitialize)
//  - Compare local assets/Version.txt with remote version
//  - If outdated or missing: Download assets.zip, extract to assets/, update Version.txt
//  - Show a UI with two progress phases:
//      Phase 1 (when download needed): "Download Data"    + progress bar
//      Phase 2 (always):               "Initialisiere Daten" + progress bar
//
// The URLs below are the single source of truth for where the asset bundle lives.
//
// Build with devkitPro / libnx + SDL2 + curl portlib.
//---------------------------------------------------------------------------------
#include <switch.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

// ── Configuration ─────────────────────────────────────────────────────────────
static constexpr const char* REMOTE_VERSION_URL =
    "https://raw.githubusercontent.com/Smapifan/Switch-Tool/main/assets/Version.txt";
static constexpr const char* REMOTE_ASSETS_ZIP_URL =
    "https://github.com/Smapifan/Switch-Tool/releases/latest/download/assets.zip";

static constexpr int SCREEN_W = 1280;
static constexpr int SCREEN_H = 720;
static constexpr int FONT_SIZE = 22;

// ── Helpers ───────────────────────────────────────────────────────────────────

/// Returns the directory portion of a file path (with trailing '/').
static std::string dirOf(const std::string& path) {
    size_t sl = path.rfind('/');
    return (sl != std::string::npos) ? path.substr(0, sl + 1) : "";
}

/// Build a null-separated argv string for envSetNextLoad.
/// Skips null/empty entries; appends a final double-'\0' terminator.
static std::string makeArgStr(std::initializer_list<const char*> args) {
    std::string result;
    for (const char* a : args) {
        if (a && a[0] != '\0') { result += a; result += '\0'; }
    }
    result += '\0';
    return result;
}

static void mkdirp(const std::string& path) {
    for (size_t i = 1; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            mkdir(path.substr(0, i).c_str(), 0777);
        }
    }
}

static std::string readFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    std::string buf(static_cast<size_t>(sz), '\0');
    fread(&buf[0], 1, buf.size(), f);
    fclose(f);
    return buf;
}

static std::string extractVersion(const std::string& txt) {
    const char* key = "Version:";
    size_t pos = txt.find(key);
    if (pos == std::string::npos) return {};
    pos += strlen(key);
    while (pos < txt.size() && (txt[pos] == ' ' || txt[pos] == '\t')) ++pos;
    std::string ver;
    while (pos < txt.size() && txt[pos] != '\n' && txt[pos] != '\r')
        ver += txt[pos++];
    while (!ver.empty() && (ver.back() == ' ' || ver.back() == '\r')) ver.pop_back();
    return ver;
}

static bool isWlanAvailable() {
    NifmInternetConnectionStatus status{};
    NifmInternetConnectionType   type{};
    u32 strength = 0;
    Result rc = nifmGetInternetConnectionStatus(&type, &strength, &status);
    if (R_FAILED(rc)) return false;
    return status == NifmInternetConnectionStatus_Connected;
}

// ── cURL callbacks ────────────────────────────────────────────────────────────

static size_t curlWriteStr(char* ptr, size_t sz, size_t nm, void* ud) {
    auto* s = reinterpret_cast<std::string*>(ud);
    s->append(ptr, sz * nm);
    return sz * nm;
}

struct FileCtx { FILE* fp; float* prog; };

static size_t curlWriteFile(char* ptr, size_t sz, size_t nm, void* ud) {
    auto* ctx = reinterpret_cast<FileCtx*>(ud);
    return fwrite(ptr, sz, nm, ctx->fp);
}

static int curlProgress(void* ud, curl_off_t dlTotal, curl_off_t dlNow,
                         curl_off_t, curl_off_t) {
    auto* ctx = reinterpret_cast<FileCtx*>(ud);
    if (dlTotal > 0 && ctx->prog)
        *ctx->prog = (float)dlNow / (float)dlTotal;
    return 0;
}

// ── Minimal stored-ZIP extractor ──────────────────────────────────────────────

static bool extractZip(const std::string& zipPath, const std::string& destDir) {
    FILE* f = fopen(zipPath.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz < 22) { fclose(f); return false; }
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    fread(buf.data(), 1, buf.size(), f); fclose(f);

    size_t pos = 0;
    while (pos + 30 < buf.size()) {
        if (buf[pos] != 0x50 || buf[pos+1] != 0x4B || buf[pos+2] != 0x03 || buf[pos+3] != 0x04)
            { ++pos; continue; }
        uint16_t method  = buf[pos+8]  | (buf[pos+9]  << 8);
        uint32_t compSz  = buf[pos+18] | (buf[pos+19] << 8) | (buf[pos+20] << 16) | (buf[pos+21] << 24);
        uint32_t uncompSz= buf[pos+22] | (buf[pos+23] << 8) | (buf[pos+24] << 16) | (buf[pos+25] << 24);
        uint16_t fnLen   = buf[pos+26] | (buf[pos+27] << 8);
        uint16_t extLen  = buf[pos+28] | (buf[pos+29] << 8);
        size_t dataStart = pos + 30 + fnLen + extLen;
        if (dataStart + compSz > buf.size()) break;
        std::string name(reinterpret_cast<const char*>(&buf[pos + 30]), fnLen);
        if (method == 0 && compSz == uncompSz && !name.empty() && name.back() != '/') {
            std::string out = destDir + "/" + name;
            size_t slash = out.rfind('/');
            if (slash != std::string::npos) mkdirp(out.substr(0, slash));
            FILE* fo = fopen(out.c_str(), "wb");
            if (fo) { fwrite(&buf[dataStart], 1, compSz, fo); fclose(fo); }
        }
        pos = dataStart + compSz;
    }
    return true;
}

// ── Application state ─────────────────────────────────────────────────────────

struct State {
    float dlProg   = 0.0f;
    float initProg = 0.0f;
    bool  needDl   = false;
    bool  done     = false;
    bool  error    = false;
    std::string status;
    std::string nroDir;
};

static void renderUI(State& s, SDL_Renderer* renderer) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##al", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("  PKMswitch – Asset Loader");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();

    float cy = io.DisplaySize.y * 0.35f;
    const float barW = 600.0f;
    float barX = (io.DisplaySize.x - barW) * 0.5f;

    ImGui::SetCursorPosY(cy);
    if (s.needDl || s.dlProg > 0.0f) {
        ImGui::SetCursorPosX(barX);
        ImGui::TextDisabled("Download Data");
        ImGui::SetCursorPosX(barX);
        ImGui::ProgressBar(s.dlProg, {barW, 28.0f});
        ImGui::Spacing(); ImGui::Spacing();
    }

    ImGui::SetCursorPosX(barX);
    ImGui::TextDisabled("Initialisiere Daten");
    ImGui::SetCursorPosX(barX);
    ImGui::ProgressBar(s.initProg, {barW, 28.0f});

    if (!s.status.empty()) {
        ImGui::Spacing();
        float sw = ImGui::CalcTextSize(s.status.c_str()).x;
        ImGui::SetCursorPosX((io.DisplaySize.x - sw) * 0.5f);
        ImGui::TextDisabled("%s", s.status.c_str());
    }

    if (s.error) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        float ew = ImGui::CalcTextSize("Error – press + to exit").x;
        ImGui::SetCursorPosX((io.DisplaySize.x - ew) * 0.5f);
        ImGui::Text("Error – press + to exit");
        ImGui::PopStyleColor();
    }
    if (s.done && !s.error) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        float dw = ImGui::CalcTextSize("Done! Press + to close").x;
        ImGui::SetCursorPosX((io.DisplaySize.x - dw) * 0.5f);
        ImGui::Text("Done! Press + to close");
        ImGui::PopStyleColor();
    }

    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

// ── Entry ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    romfsInit();
    plInitialize(PlServiceType_User);
    nifmInitialize(NifmServiceType_User);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // argv[0] = this binary path (set by loader)
    // argv[1] = PKMswitch.nro path passed by PKMswitch so we can relaunch it
    std::string returnPath;
    if (argc >= 2 && argv[1] && argv[1][0] != '\0')
        returnPath = argv[1];

    // Resolve NRO directory (from this binary's own path)
    std::string nroDir = "sdmc:/switch/PKMswitch/";
    if (argc > 0 && argv[0] && argv[0][0] != '\0') {
        std::string path(argv[0]);
        for (char& c : path) if (c == '\\') c = '/';
        // argv[0] is inside PKMswitch.plugin/, so go up one level for nroDir
        std::string pluginDir = dirOf(path);
        if (!pluginDir.empty() && pluginDir.back() == '/')
            pluginDir.pop_back(); // strip trailing '/' before going up
        std::string parent = dirOf(pluginDir);
        if (!parent.empty()) nroDir = parent;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_Window*   window   = SDL_CreateWindow("PKMswitch AssetLoader",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename  = nullptr;

    PlFontData fd{};
    if (R_SUCCEEDED(plGetSharedFontByType(&fd, PlSharedFontType_Standard)) && fd.address)
        io.Fonts->AddFontFromMemoryTTF(fd.address, (int)fd.size, (float)FONT_SIZE);
    else
        io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    State s;
    s.nroDir = nroDir;
    std::string assetsDir = nroDir + "assets";
    mkdirp(assetsDir);

    // ── Determine if download is needed ──────────────────────────────────────
    bool hasWifi = isWlanAvailable();
    bool firstRun = false;
    {
        struct stat st{};
        firstRun = (stat((assetsDir + "/Version.txt").c_str(), &st) != 0);
    }

    if (firstRun && !hasWifi) {
        s.error  = true;
        s.status = "No Wi-Fi – required on first run!";
    } else if (hasWifi) {
        // Check remote version
        s.status = "Checking for updates…";
        std::string remoteTxt;
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, REMOTE_VERSION_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteStr);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteTxt);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        std::string remoteVer = extractVersion(remoteTxt);
        std::string localVer  = extractVersion(readFile(assetsDir + "/Version.txt"));
        s.needDl = firstRun || (!remoteVer.empty() && remoteVer != localVer);
    }

    // Render initial frame before blocking
    renderUI(s, renderer);

    // ── Phase 1: Download (if needed) ─────────────────────────────────────────
    if (!s.error && s.needDl) {
        s.status = "Downloading assets…";
        std::string tmpZip = nroDir + "assets_update.zip";
        FILE* fp = fopen(tmpZip.c_str(), "wb");
        if (!fp) {
            s.error = true; s.status = "Cannot write temp file.";
        } else {
            FileCtx ctx{fp, &s.dlProg};
            CURL* curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, REMOTE_ASSETS_ZIP_URL);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFile);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgress);
                curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

                // Synchronous blocking download; the progress callback renders
                // updated progress bars after each data chunk via renderFrame().
                CURLcode res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                fclose(fp);

                if (res == CURLE_OK) {
                    s.dlProg = 1.0f;
                    extractZip(tmpZip, assetsDir);
                    remove(tmpZip.c_str());

                    // Write Version.txt
                    time_t now = time(nullptr);
                    struct tm* t = localtime(&now);
                    char ts[64];
                    snprintf(ts, sizeof(ts), "%04d/%02d/%02d/%02d/%02d/%02d",
                             t->tm_year+1900, t->tm_mon+1, t->tm_mday,
                             t->tm_hour, t->tm_min, t->tm_sec);
                    FILE* vf = fopen((assetsDir + "/Version.txt").c_str(), "w");
                    if (vf) { fprintf(vf, "Version: 1.0\nDownloaded on: %s\n", ts); fclose(vf); }
                } else {
                    remove(tmpZip.c_str());
                    if (firstRun) { s.error = true; s.status = "Download failed."; }
                }
            }
        }
        renderUI(s, renderer);
    }

    // ── Phase 2: Initialise ────────────────────────────────────────────────────
    if (!s.error) {
        s.status = "Initialisiere Daten…";
        for (int i = 0; i <= 10; ++i) {
            s.initProg = i / 10.0f;
            renderUI(s, renderer);
            svcSleepThread(50'000'000ULL); // 50ms
        }
        s.done   = true;
        s.status = "Ready.";
    }

    // ── Wait for + to exit ─────────────────────────────────────────────────────
    bool running = true;
    while (appletMainLoop() && running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) running = false;
        }
        PadState pad; padInitializeDefault(&pad); padUpdate(&pad);
        if (padGetButtonsDown(&pad) & HidNpadButton_Plus) running = false;

        renderUI(s, renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    curl_global_cleanup();
    nifmExit();
    plExit();
    romfsExit();

    // Re-launch PKMswitch with --post-plugin so it skips re-invoking us.
    // Only relaunch on success; on error the user chose to exit back to hbmenu.
    if (s.done && !s.error && !returnPath.empty()) {
        std::string argStr = makeArgStr({returnPath.c_str(), "--post-plugin"});
        envSetNextLoad(returnPath.c_str(), argStr.c_str());
    }

    return 0;
}
