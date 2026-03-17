#include <sys/select.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <map>

// --- Remote-Info ---
static constexpr const char* REMOTE_VERSION_URL = "https://raw.githubusercontent.com/Smapifan/Switch-Tool/main/assets/Version.txt";
static constexpr const char* REMOTE_ASSETS_ZIP_URL = "https://github.com/Smapifan/Switch-Tool/releases/latest/download/assets.zip";

// Beispiel-Datenstruktur für Items & IDs (Dummy)
struct ItemInfo {
    int id;
    std::string name;
    std::map<std::string, std::string> translations;
};
std::map<std::string, std::vector<ItemInfo>> gameItems = {
    {"firered",   { {1,"POKE BALL", {{"en","POKE BALL"},{"de","POKEBALL"}}}, {2,"POTION", {{"en","POTION"}}} }},
    {"leafgreen", { {1,"POKE BALL", {{"en","POKE BALL"},{"de","POKEBALL"},{"fr","BALLE"}}}, {2,"POTION", {{"en","POTION"}}} }},
    {"emerald",   { {1,"POKE BALL", {{"en","POKE BALL"}}} }}
};

static void mkdirp(const std::string& path) {
    for (size_t i = 1; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '/')
            mkdir(path.substr(0,i).c_str(), 0777);
}

static std::string readFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    std::string buf(static_cast<size_t>(sz), '\0');
    fread(&buf[0], 1, buf.size(), f); fclose(f);
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

static std::string extractPublished(const std::string& txt) {
    const char* key = "Published on:";
    size_t pos = txt.find(key);
    if (pos == std::string::npos) return {};
    pos += strlen(key);
    while (pos < txt.size() && (txt[pos] == ' ' || txt[pos] == '\t')) ++pos;
    std::string pub;
    while (pos < txt.size() && txt[pos] != '\n' && txt[pos] != '\r')
        pub += txt[pos++];
    while (!pub.empty() && (pub.back() == ' ' || pub.back() == '\r')) pub.pop_back();
    return pub;
}

static size_t curlWriteStr(char* ptr, size_t sz, size_t nm, void* ud) {
    std::string* s = reinterpret_cast<std::string*>(ud);
    s->append(ptr, sz * nm);
    return sz * nm;
}

static size_t curlWriteFile(char* ptr, size_t sz, size_t nm, void* ud) {
    FILE* fp = reinterpret_cast<FILE*>(ud);
    return fwrite(ptr, sz, nm, fp);
}

int main(int argc, char* argv[]) {
    mkdirp("assets/IDs");

    std::string assetsDir = "assets";
    std::string verPath   = assetsDir + "/Version.txt";
    std::string localTxt  = readFile(verPath);

    // --- Remote Version laden ---
    std::string remoteTxt;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, REMOTE_VERSION_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteStr);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteTxt);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl); curl_easy_cleanup(curl);
    }
    std::string remoteVer = extractVersion(remoteTxt);
    std::string remotePub = extractPublished(remoteTxt);
    std::string localVer  = extractVersion(localTxt);
    std::string localPub  = extractPublished(localTxt);

    bool update =
        localVer.empty() || localPub.empty()
        || localVer != remoteVer || localPub != remotePub;

    if (update) {
        // Download assets.zip
        std::string zipPath = assetsDir + "_update.zip";
        FILE* fp = fopen(zipPath.c_str(), "wb");
        if (!fp) { printf("Cannot write temp file.\n"); return 1; }
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, REMOTE_ASSETS_ZIP_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFile);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
            curl_easy_perform(curl); curl_easy_cleanup(curl);
        }
        fclose(fp);

        // TODO: ZIP-Entpacken
        // Hier könntest du mit miniz oder libzip usw. das Archiv entpacken

        // Version.txt schreiben mit aktuellem Timestamp
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        char ts[64];
        snprintf(ts, sizeof(ts), "%04d/%02d/%02d/%02d/%02d/%02d",
                 t->tm_year + 1900, t->tm_mon+1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec);

        FILE* vf = fopen(verPath.c_str(), "w");
        if (vf) {
            fprintf(vf, "Version: %s\nPublished on: %s\nDownloaded on: %s\n",
                    remoteVer.c_str(), remotePub.c_str(), ts);
            fclose(vf);
        }
        remove(zipPath.c_str());
    }

    // --- IDs exportieren nach assets/IDs/ ---
    for (const auto& game : gameItems) {
        std::string gameName = game.first;
        for (const auto& item : game.second) {
            // Jede Sprache separat speichern
            for (const auto& trans : item.translations) {
                std::string lang = trans.first;
                std::string fname = gameName;
                if (lang != "en") fname += "_" + lang;
                std::string idFile = assetsDir + "/IDs/" + fname + ".txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", item.id, trans.second.c_str());
                    fclose(outf);
                }
            }
            // Englisch als Fallback
            if (item.translations.count("en") == 0) {
                std::string idFile = assetsDir + "/IDs/" + gameName + ".txt";
                FILE* outf = fopen(idFile.c_str(), "a");
                if (outf) {
                    fprintf(outf, "%d\t%s\n", item.id, item.name.c_str());
                    fclose(outf);
                }
            }
        }
    }

    curl_global_cleanup();
    return 0;
}
