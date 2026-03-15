#include "asset_loader.hpp"
#include "ids_loader.hpp"
#include "i18n.hpp"

#include <switch.h>
#include <imgui.h>
#include <curl/curl.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

// ── Simple ZIP extractor (stored-only, no compression) ───────────────────────
// Handles method-0 (stored) entries in the downloaded assets ZIP.
// A full deflate decompressor can be added by linking zlib and handling method 8.

namespace AssetLoader {

// ── Helper: create directory recursively ─────────────────────────────────────
static void mkdirp(const std::string& path) {
    for (size_t i = 1; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            std::string sub = path.substr(0, i);
            mkdir(sub.c_str(), 0777);
        }
    }
}

// ── Helper: read a local file into a string ───────────────────────────────────
static std::string readLocalFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    std::string buf(static_cast<size_t>(sz), '\0');
    fread(&buf[0], 1, static_cast<size_t>(sz), f);
    fclose(f);
    return buf;
}

// ── Version.txt parser ────────────────────────────────────────────────────────
static std::string extractVersion(const std::string& txt) {
    // "Version: 1.0"  → "1.0"
    const char* key = "Version:";
    size_t pos = txt.find(key);
    if (pos == std::string::npos) return {};
    pos += strlen(key);
    while (pos < txt.size() && (txt[pos] == ' ' || txt[pos] == '\t')) ++pos;
    std::string ver;
    while (pos < txt.size() && txt[pos] != '\n' && txt[pos] != '\r')
        ver += txt[pos++];
    // trim trailing whitespace
    while (!ver.empty() && (ver.back() == ' ' || ver.back() == '\r')) ver.pop_back();
    return ver;
}

// ── cURL write callback ───────────────────────────────────────────────────────
static size_t curlWriteString(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = reinterpret_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

struct CurlFileCtx {
    FILE*  fp    = nullptr;
    double total = 0.0;
    float* progress = nullptr; // pointer into AppState
};

static size_t curlWriteFile(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = reinterpret_cast<CurlFileCtx*>(userdata);
    size_t written = fwrite(ptr, size, nmemb, ctx->fp);
    return written;
}

static int curlProgressCb(void* userdata, curl_off_t dlTotal, curl_off_t dlNow,
                           curl_off_t /*ulTotal*/, curl_off_t /*ulNow*/) {
    auto* ctx = reinterpret_cast<CurlFileCtx*>(userdata);
    if (dlTotal > 0 && ctx->progress)
        *ctx->progress = static_cast<float>(dlNow) / static_cast<float>(dlTotal);
    return 0;
}

// ── Minimal stored-ZIP extractor ──────────────────────────────────────────────
// Reads a file written with the "stored" (method 0) format.

static bool extractZip(const std::string& zipPath, const std::string& destDir) {
    FILE* f = fopen(zipPath.c_str(), "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 22) { fclose(f); return false; }

    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    if (fread(buf.data(), 1, buf.size(), f) != buf.size()) { fclose(f); return false; }
    fclose(f);

    size_t pos = 0;
    while (pos + 30 < buf.size()) {
        // Local file header signature
        if (buf[pos]   != 0x50 || buf[pos+1] != 0x4B ||
            buf[pos+2] != 0x03 || buf[pos+3] != 0x04) {
            // Not a local file header – skip to next possible signature
            ++pos;
            continue;
        }
        uint16_t method    = static_cast<uint16_t>(buf[pos+8]  | (buf[pos+9]  << 8));
        uint32_t compSz    = static_cast<uint32_t>(buf[pos+18] | (buf[pos+19] << 8) |
                                                   (buf[pos+20] << 16) | (buf[pos+21] << 24));
        uint32_t uncompSz  = static_cast<uint32_t>(buf[pos+22] | (buf[pos+23] << 8) |
                                                   (buf[pos+24] << 16) | (buf[pos+25] << 24));
        uint16_t fnLen     = static_cast<uint16_t>(buf[pos+26] | (buf[pos+27] << 8));
        uint16_t extraLen  = static_cast<uint16_t>(buf[pos+28] | (buf[pos+29] << 8));

        size_t dataStart = pos + 30 + fnLen + extraLen;
        if (dataStart + compSz > buf.size()) break;

        // Extract filename
        std::string name(reinterpret_cast<const char*>(&buf[pos + 30]), fnLen);

        // Only handle stored files (method 0)
        if (method == 0 && compSz == uncompSz && !name.empty() && name.back() != '/') {
            std::string outPath = destDir + "/" + name;
            // Ensure parent directories
            size_t slash = outPath.rfind('/');
            if (slash != std::string::npos)
                mkdirp(outPath.substr(0, slash));

            FILE* out = fopen(outPath.c_str(), "wb");
            if (out) {
                fwrite(&buf[dataStart], 1, compSz, out);
                fclose(out);
            }
        }
        pos = dataStart + compSz;
    }
    return true;
}

// ── Write Version.txt ─────────────────────────────────────────────────────────

static void writeVersionTxt(const std::string& assetsDir,
                             const std::string& version,
                             const std::string& publishedOn) {
    std::string path = assetsDir + "/Version.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;

    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char ts[64];
    snprintf(ts, sizeof(ts), "%04d/%02d/%02d/%02d/%02d/%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    fprintf(f, "Version: %s\n", version.c_str());
    if (!publishedOn.empty())
        fprintf(f, "Published on: %s\n", publishedOn.c_str());
    fprintf(f, "Downloaded on: %s\n", ts);
    fclose(f);
}

// ── Public API ────────────────────────────────────────────────────────────────

bool isWlanAvailable() {
    NifmInternetConnectionStatus status{};
    NifmInternetConnectionType   type{};
    u32                          strength = 0;
    Result rc = nifmGetInternetConnectionStatus(&type, &strength, &status);
    if (R_FAILED(rc)) return false;
    return status == NifmInternetConnectionStatus_Connected;
}

bool isFirstRun(const std::string& assetsDir) {
    std::string path = assetsDir + "/Version.txt";
    struct stat st{};
    return stat(path.c_str(), &st) != 0;
}

bool run(AppState& state) {
    std::string assetsDir = state.nroDir + "assets";
    mkdirp(assetsDir);

    bool firstRun   = isFirstRun(assetsDir);
    bool hasNetwork = isWlanAvailable();

    // ── Phase 1: Download (if needed) ────────────────────────────────────────
    bool updateNeeded = firstRun;
    state.assetDownloadNeeded = false;

    if (!firstRun && hasNetwork) {
        // Fetch remote Version.txt and compare
        state.assetStatusMessage = I18n::t("asset_checking_updates");
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string remoteTxt;
            curl_easy_setopt(curl, CURLOPT_URL, REMOTE_VERSION_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteString);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteTxt);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            std::string remoteVer = extractVersion(remoteTxt);
            std::string localTxt  = readLocalFile(assetsDir + "/Version.txt");
            std::string localVer  = extractVersion(localTxt);

            updateNeeded = (!remoteVer.empty() && remoteVer != localVer);
        }
    }

    state.assetDownloadNeeded = updateNeeded;

    if (updateNeeded) {
        if (!hasNetwork) {
            // First run requires network; subsequent updates are skipped silently
            if (firstRun) {
                state.assetStatusMessage = I18n::t("asset_no_wifi_first");
                state.assetsReady = false;
                return false;
            }
            // Non-first-run: skip update, proceed with local assets
            updateNeeded = false;
        } else {
            // ── Download assets.zip ───────────────────────────────────────────
            state.assetStatusMessage = I18n::t("asset_downloading");
            state.assetDownloadProgress = 0.0f;

            std::string tmpZip = state.nroDir + "assets_update.zip";
            FILE* fp = fopen(tmpZip.c_str(), "wb");
            if (!fp) { state.assetsReady = false; return false; }

            CurlFileCtx ctx{fp, 0.0, &state.assetDownloadProgress};
            CURL* curl = curl_easy_init();
            if (!curl) { fclose(fp); state.assetsReady = false; return false; }

            curl_easy_setopt(curl, CURLOPT_URL, REMOTE_ASSETS_ZIP_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFile);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curlProgressCb);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);

            if (res != CURLE_OK) {
                remove(tmpZip.c_str());
                if (firstRun) { state.assetsReady = false; return false; }
                // Fall through with local assets on non-fatal update failure
            } else {
                state.assetDownloadProgress = 1.0f;
                // Extract
                extractZip(tmpZip, assetsDir);
                remove(tmpZip.c_str());
                // Write Version.txt with download timestamp
                writeVersionTxt(assetsDir, "latest", "");
            }
        }
    }

    // ── Phase 2: Initialise data ──────────────────────────────────────────────
    state.assetStatusMessage = I18n::t("asset_initialising");
    state.assetInitProgress  = 0.0f;

    // Discover and generate IDs.txt files
    auto idsFiles = IdsLoader::discoverAndLoad(assetsDir);
    state.assetInitProgress = 0.5f;

    // Simulate short init work (tick-based in real implementation)
    state.assetInitProgress = 1.0f;

    (void)idsFiles; // IDs data will be used by game modules

    state.assetsReady        = true;
    state.assetStatusMessage = I18n::t("asset_ready");
    return true;
}

// ── Init screen render ────────────────────────────────────────────────────────

void renderInitScreen(AppState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("##asset_init", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove       |
                 ImGuiWindowFlags_NoSavedSettings);

    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.25f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("  %s", I18n::t("app_title"));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();

    float centerY = io.DisplaySize.y * 0.35f;
    ImGui::SetCursorPosY(centerY);

    const float barW = 600.0f;
    float       barX = (io.DisplaySize.x - barW) * 0.5f;

    // Phase 1: Download (only shown when needed)
    if (state.assetDownloadNeeded || state.assetDownloadProgress > 0.0f) {
        ImGui::SetCursorPosX(barX);
        ImGui::TextDisabled("%s", I18n::t("asset_dl_label"));
        ImGui::SetCursorPosX(barX);
        ImGui::ProgressBar(state.assetDownloadProgress, {barW, 28.0f});
        ImGui::Spacing();
        ImGui::Spacing();
    }

    // Phase 2: Initialise
    ImGui::SetCursorPosX(barX);
    ImGui::TextDisabled("%s", I18n::t("asset_init_label"));
    ImGui::SetCursorPosX(barX);
    ImGui::ProgressBar(state.assetInitProgress, {barW, 28.0f});

    ImGui::Spacing();
    ImGui::Spacing();
    if (!state.assetStatusMessage.empty()) {
        float msgW = ImGui::CalcTextSize(state.assetStatusMessage.c_str()).x;
        ImGui::SetCursorPosX((io.DisplaySize.x - msgW) * 0.5f);
        ImGui::TextDisabled("%s", state.assetStatusMessage.c_str());
    }

    ImGui::End();
}

} // namespace AssetLoader
