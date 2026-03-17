#include <sys/stat.h>
#include <curl/curl.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

// Remote URLs
static constexpr const char* REMOTE_VERSION_URL =
    "https://raw.githubusercontent.com/Smapifan/Switch-Tool/main/assets/Version.txt";
static constexpr const char* REMOTE_ASSETS_ZIP_URL =
    "https://github.com/Smapifan/Switch-Tool/releases/latest/download/assets.zip";

// Rekursive Erstellung von Verzeichnissen
static void mkdirp(const std::string& path) {
    for (size_t i = 1; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '/')
            mkdir(path.substr(0,i).c_str(), 0777);
}

// Einfaches Datei-Lesen, liefert gesamte Datei als String
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

// Extrahiert "Version:" aus Text
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

// Extrahiert "Published on:" aus Text
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

// Für cURL String-In-Memory
static size_t curlWriteStr(char* ptr, size_t sz, size_t nm, void* ud) {
    std::string* s = reinterpret_cast<std::string*>(ud);
    s->append(ptr, sz * nm);
    return sz * nm;
}

// Für cURL Datei-Download
static size_t curlWriteFile(char* ptr, size_t sz, size_t nm, void* ud) {
    FILE* fp = reinterpret_cast<FILE*>(ud);
    return fwrite(ptr, sz, nm, fp);
}

int main() {
    mkdirp("assets");
    std::string assetsDir = "assets";
    std::string verPath   = assetsDir + "/Version.txt";
    std::string localTxt  = readFile(verPath);

    // --- Remote-Version holen ---
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
        // assets.zip herunterladen
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

        // TODO: Entpacken assets.zip (optional: miniz, libzip ...)

        // Version.txt aktualisieren mit Timestamp
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        char ts[64];
        snprintf(ts, sizeof(ts), "%04d/%02d/%02d/%02d/%02d/%02d",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

        FILE* vf = fopen(verPath.c_str(), "w");
        if (vf) {
            fprintf(vf, "Version: %s\nPublished on: %s\nDownloaded on: %s\n",
                remoteVer.c_str(), remotePub.c_str(), ts);
            fclose(vf);
        }
        remove(zipPath.c_str());
    }
    curl_global_cleanup();
    return 0;
}
