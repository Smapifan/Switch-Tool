#include "plugin_check.hpp"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <string>

namespace Plugin {

// ── Path helpers ────────────────────────────────────────────────────────────

std::string resolvePluginDir(const char* argv0) {
    std::string nroDir;

    if (argv0 && argv0[0] != '\0') {
        std::string path(argv0);
        // Normalise backslashes (shouldn't appear, but be safe)
        for (char& c : path) if (c == '\\') c = '/';

        size_t lastSlash = path.rfind('/');
        if (lastSlash != std::string::npos)
            nroDir = path.substr(0, lastSlash + 1);
    }

    if (nroDir.empty())
        nroDir = "sdmc:/switch/PKMswitch/";

    return nroDir + "PKMswitch.plugin";
}

// ── Minimal JSON value extractor ────────────────────────────────────────────
// Only needed for the few fields in manifest.json – not a full parser.

static std::string jsonStringField(const std::string& json, const char* key) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();
    // Skip whitespace and ':'
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':' ||
                                  json[pos] == '\t')) ++pos;
    if (pos >= json.size() || json[pos] != '"') return {};
    ++pos; // skip opening "
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\') {
            ++pos;
            if (pos < json.size()) result += json[pos];
        } else {
            result += json[pos];
        }
        ++pos;
    }
    return result;
}

static int jsonIntField(const std::string& json, const char* key, int defVal = 0) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return defVal;
    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':' ||
                                  json[pos] == '\t')) ++pos;
    if (pos >= json.size()) return defVal;
    int value = 0;
    bool neg = false;
    if (json[pos] == '-') { neg = true; ++pos; }
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        value = value * 10 + (json[pos] - '0');
        ++pos;
    }
    return neg ? -value : value;
}

static bool dirExists(const std::string& path) {
    struct stat st{};
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool fileExists(const std::string& path) {
    struct stat st{};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

// ── Public API ──────────────────────────────────────────────────────────────

bool validate(const char* pluginDirC, Info& info) {
    info = Info{};
    if (!pluginDirC) return false;

    std::string pluginDir(pluginDirC);
    info.dir = pluginDir;

    // 1. The plugin directory must exist
    if (!dirExists(pluginDir)) return false;

    // 2. manifest.json must exist
    std::string manifestPath = pluginDir + "/manifest.json";
    if (!fileExists(manifestPath)) return false;

    // 3. Read and parse manifest
    FILE* f = fopen(manifestPath.c_str(), "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 64 * 1024) { // Sanity-cap at 64 KiB
        fclose(f);
        return false;
    }

    std::string buf(static_cast<size_t>(size), '\0');
    fread(&buf[0], 1, static_cast<size_t>(size), f);
    fclose(f);

    // 4. Validate required fields
    info.pluginId     = jsonStringField(buf, "pluginId");
    info.author       = jsonStringField(buf, "author");
    info.formatVersion = jsonIntField(buf, "formatVersion", 0);
    info.version      = jsonIntField(buf, "version", 0);

    if (info.pluginId.empty())      return false;
    if (info.formatVersion != 1)    return false;

    info.valid = true;
    return true;
}

} // namespace Plugin
