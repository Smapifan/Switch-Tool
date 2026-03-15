#include "save_backup.hpp"
#include "app_state.hpp"

#include <switch.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

namespace Backup {

// ── Minimal ZIP writer (stored, no compression) ──────────────────────────────
// Implements just enough of the ZIP spec (PKWARE AppNote) to produce a valid
// .zip containing one or more binary files.  No compression is used (method 0)
// to keep the implementation self-contained without needing zlib.

static uint32_t crc32_table[256];
static bool     crc32_init = false;

static void buildCrc32Table() {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int j = 0; j < 8; ++j)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc32_table[i] = c;
    }
    crc32_init = true;
}

static uint32_t crc32(const uint8_t* buf, size_t len) {
    if (!crc32_init) buildCrc32Table();
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i)
        c = crc32_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

struct ZipEntry {
    std::string name;
    std::vector<uint8_t> data;
};

/// Write a little-endian 16-bit value.
static void writeU16(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(static_cast<uint8_t>(v & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}
/// Write a little-endian 32-bit value.
static void writeU32(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) { out.push_back(static_cast<uint8_t>(v & 0xFF)); v >>= 8; }
}

static std::vector<uint8_t> buildZip(const std::vector<ZipEntry>& entries) {
    std::vector<uint8_t> zip;

    // DOS timestamp (fixed: 1980-01-01 00:00:00)
    constexpr uint16_t DOS_DATE = (0 << 9) | (1 << 5) | 1;  // year=0 (1980), month=1, day=1
    constexpr uint16_t DOS_TIME = 0;

    struct LocalHdrOffset { uint32_t offset; uint16_t fnLen; uint32_t crc32v; uint32_t size; };
    std::vector<LocalHdrOffset> offsets;

    for (const auto& e : entries) {
        uint32_t offset = static_cast<uint32_t>(zip.size());
        uint16_t fnLen  = static_cast<uint16_t>(e.name.size());
        uint32_t crcVal = crc32(e.data.data(), e.data.size());
        uint32_t sz     = static_cast<uint32_t>(e.data.size());

        // Local file header  (signature 0x04034b50)
        writeU32(zip, 0x04034b50u);
        writeU16(zip, 20);          // version needed
        writeU16(zip, 0);           // flags
        writeU16(zip, 0);           // compression: stored
        writeU16(zip, DOS_TIME);
        writeU16(zip, DOS_DATE);
        writeU32(zip, crcVal);
        writeU32(zip, sz);          // compressed size = uncompressed size (stored)
        writeU32(zip, sz);
        writeU16(zip, fnLen);
        writeU16(zip, 0);           // extra field length
        for (char c : e.name) zip.push_back(static_cast<uint8_t>(c));

        // File data
        zip.insert(zip.end(), e.data.begin(), e.data.end());

        offsets.push_back({offset, fnLen, crcVal, sz});
    }

    // Central directory
    uint32_t cdOffset = static_cast<uint32_t>(zip.size());
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e   = entries[i];
        const auto& off = offsets[i];
        uint16_t fnLen  = off.fnLen;

        writeU32(zip, 0x02014b50u);  // central dir signature
        writeU16(zip, 20);            // version made by
        writeU16(zip, 20);            // version needed
        writeU16(zip, 0);             // flags
        writeU16(zip, 0);             // compression: stored
        writeU16(zip, DOS_TIME);
        writeU16(zip, DOS_DATE);
        writeU32(zip, off.crc32v);
        writeU32(zip, off.size);
        writeU32(zip, off.size);
        writeU16(zip, fnLen);
        writeU16(zip, 0);             // extra field len
        writeU16(zip, 0);             // comment len
        writeU16(zip, 0);             // disk start
        writeU16(zip, 0);             // internal attrs
        writeU32(zip, 0);             // external attrs
        writeU32(zip, off.offset);
        for (char c : e.name) zip.push_back(static_cast<uint8_t>(c));
    }

    uint32_t cdSize = static_cast<uint32_t>(zip.size()) - cdOffset;
    uint16_t count  = static_cast<uint16_t>(entries.size());

    // End of central directory
    writeU32(zip, 0x06054b50u);
    writeU16(zip, 0);       // disk number
    writeU16(zip, 0);       // disk with CD start
    writeU16(zip, count);   // entries on this disk
    writeU16(zip, count);   // total entries
    writeU32(zip, cdSize);
    writeU32(zip, cdOffset);
    writeU16(zip, 0);       // comment length

    return zip;
}

// ── Filesystem helpers ────────────────────────────────────────────────────────

static void ensureDir(const std::string& path) {
    mkdir(path.c_str(), 0777);
}

/// Recursively collect all files under a directory into ZipEntry list.
static void collectFiles(const std::string& rootDir,
                         const std::string& relDir,
                         std::vector<ZipEntry>& out) {
    std::string fullDir = rootDir + relDir;
    DIR* d = opendir(fullDir.c_str());
    if (!d) return;

    struct dirent* ent;
    while ((ent = readdir(d)) != nullptr) {
        if (ent->d_name[0] == '.') continue; // skip . and ..

        std::string relPath  = relDir + ent->d_name;
        std::string fullPath = rootDir + relPath;

        struct stat st{};
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            collectFiles(rootDir, relPath + "/", out);
        } else if (S_ISREG(st.st_mode)) {
            FILE* f = fopen(fullPath.c_str(), "rb");
            if (!f) continue;
            ZipEntry ze;
            ze.name = relPath;
            ze.data.resize(static_cast<size_t>(st.st_size));
            if (ze.data.empty() || fread(ze.data.data(), 1, ze.data.size(), f) == ze.data.size())
                out.push_back(std::move(ze));
            fclose(f);
        }
    }
    closedir(d);
}

// ── Public API ────────────────────────────────────────────────────────────────

bool createSaveBackup(const AppState& state, std::string& outPath) {
    if (!state.selectedGame || state.nroDir.empty()) return false;

    // ── 1. Mount save data ────────────────────────────────────────────────────
    AccountUid uid{};
    static_assert(sizeof(uid.uid) == sizeof(state.selectedUser.uid), "UID size mismatch");
    memcpy(uid.uid, state.selectedUser.uid, sizeof(uid.uid));

    // Mount the save as read-only
    Result rc = fsdevMountSaveData("save", state.selectedGame->titleId, uid);
    if (R_FAILED(rc)) return false;

    // ── 2. Collect save files ─────────────────────────────────────────────────
    std::vector<ZipEntry> entries;
    collectFiles("save:/", "", entries);
    fsdevUnmountDevice("save");

    if (entries.empty()) return false; // nothing to back up

    // ── 3. Build ZIP bytes ────────────────────────────────────────────────────
    std::vector<uint8_t> zipData = buildZip(entries);

    // ── 4. Build backup path ──────────────────────────────────────────────────
    // Format: <nroDir>/backup/<username> <YYYY>_<MM>_<DD>_<HH>_<mm>_<ss>.zip
    std::string backupDir = state.nroDir + "backup/";
    ensureDir(backupDir);

    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char ts[64];
    snprintf(ts, sizeof(ts), "%04d_%02d_%02d_%02d_%02d_%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    outPath = backupDir + state.selectedUser.name + " " + ts + ".zip";

    // ── 5. Write ZIP file ─────────────────────────────────────────────────────
    FILE* f = fopen(outPath.c_str(), "wb");
    if (!f) return false;

    bool ok = (fwrite(zipData.data(), 1, zipData.size(), f) == zipData.size());
    fclose(f);

    if (!ok) {
        remove(outPath.c_str());
        outPath.clear();
    }
    return ok;
}

} // namespace Backup
