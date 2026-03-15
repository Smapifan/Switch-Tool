#include "ids_loader.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

namespace IdsLoader {

// ── Minimal JSON helpers ──────────────────────────────────────────────────────

static std::string readFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 4 * 1024 * 1024) { fclose(f); return {}; }
    std::string buf(static_cast<size_t>(sz), '\0');
    fread(&buf[0], 1, static_cast<size_t>(sz), f);
    fclose(f);
    return buf;
}

static void skipWs(const std::string& s, size_t& p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\r' || s[p] == '\n'))
        ++p;
}

static std::string parseStr(const std::string& s, size_t& p) {
    // advance to opening quote
    while (p < s.size() && s[p] != '"') ++p;
    if (p >= s.size()) return {};
    ++p; // skip "
    std::string r;
    while (p < s.size() && s[p] != '"') {
        if (s[p] == '\\') { ++p; if (p < s.size()) r += s[p]; }
        else r += s[p];
        ++p;
    }
    if (p < s.size()) ++p; // skip closing "
    return r;
}

/// Parse the non-standard IDs.json format:
///   { [ "ID": "0001", "Texture": "abc.png", ... ] }
/// (It uses an array syntax inside an object literal.)
static std::vector<Entry> parseIdsJson(const std::string& src) {
    std::vector<Entry> entries;
    size_t p = 0;
    // Skip the outer '{' and find '['
    while (p < src.size() && src[p] != '[') ++p;
    if (p >= src.size()) return entries;
    ++p; // skip '['

    // Parse comma-separated key:value pairs until ']'
    Entry current;
    while (p < src.size() && src[p] != ']') {
        skipWs(src, p);
        if (p >= src.size() || src[p] == ']') break;

        if (src[p] == '"') {
            std::string key = parseStr(src, p);
            skipWs(src, p);
            if (p < src.size() && src[p] == ':') { ++p; }
            skipWs(src, p);
            std::string val = parseStr(src, p);

            if (key == "ID")       current.id       = val;
            else if (key == "Texture")  current.texture  = val;
            else if (key == "Name")     current.name     = val;
            else if (key == "Category") current.category = val;

            skipWs(src, p);
            if (p < src.size() && src[p] == ',') { ++p; continue; }
            // End of object-in-array?  If both ID and Texture are set, save entry.
            if (!current.id.empty() && !current.texture.empty()) {
                entries.push_back(current);
                current = Entry{};
            }
        } else {
            ++p;
        }
    }
    // Flush last entry if any
    if (!current.id.empty() && !current.texture.empty())
        entries.push_back(current);

    return entries;
}

// ── IDs.txt writer ────────────────────────────────────────────────────────────

static void writeTxt(const std::string& dir, const std::vector<Entry>& entries) {
    std::string path = dir + "/IDs.txt";
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    for (const auto& e : entries) {
        // Format: "<name> hat id<ID> Kategorie <category>"
        std::string name     = e.name.empty()     ? e.texture : e.name;
        std::string category = e.category.empty() ? "unknown" : e.category;
        fprintf(f, "%s hat id%s Kategorie %s\n", name.c_str(), e.id.c_str(), category.c_str());
    }
    fclose(f);
}

// ── Recursive directory scanner ───────────────────────────────────────────────

/// Check if a file named "IDs.json" exists in `dir`.
static bool hasIdsJson(const std::string& dir) {
    std::string path = dir + "/IDs.json";
    struct stat st{};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

/// List all subdirectory names under `dir`.
static std::vector<std::string> listDirs(const std::string& dir) {
    std::vector<std::string> out;
    DIR* d = opendir(dir.c_str());
    if (!d) return out;
    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        std::string full = dir + "/" + ent->d_name;
        struct stat st{};
        if (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            out.push_back(full);
    }
    closedir(d);
    return out;
}

/// Recursively scan up to `maxDepth` levels, collecting IdsFile entries.
static void scanDir(const std::string& dir, int depth, int maxDepth,
                    std::vector<IdsFile>& results) {
    if (depth > maxDepth) return;

    if (hasIdsJson(dir)) {
        std::string src = readFile(dir + "/IDs.json");
        if (!src.empty()) {
            IdsFile f;
            f.dir     = dir;
            f.entries = parseIdsJson(src);
            writeTxt(dir, f.entries);
            f.txtWritten = true;
            results.push_back(std::move(f));
        }
    }

    if (depth < maxDepth) {
        for (const auto& sub : listDirs(dir))
            scanDir(sub, depth + 1, maxDepth, results);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

std::vector<IdsFile> discoverAndLoad(const std::string& rootDir) {
    std::vector<IdsFile> results;
    // Search depths 1..5 relative to rootDir
    // (assets/*/IDs.json  through  assets/*/*/*/*/*/IDs.json)
    scanDir(rootDir, 0, 5, results);
    return results;
}

} // namespace IdsLoader
