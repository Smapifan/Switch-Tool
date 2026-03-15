#include "i18n.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <switch.h>

namespace I18n {

// ── Internal state ─────────────────────────────────────────────────────────
using StringMap = std::unordered_map<std::string, std::string>;
using LangMap   = std::unordered_map<std::string, StringMap>;

static LangMap   s_langs;
static StringMap s_empty;
static std::string s_lang = "en";

// ── Minimal JSON parser (handles only the i18n.json structure) ─────────────
// Parses: { "lang": { "key": "value", ... }, ... }

static void skipWhitespace(const char* s, size_t len, size_t& pos) {
    while (pos < len && (s[pos] == ' ' || s[pos] == '\t' ||
                         s[pos] == '\r' || s[pos] == '\n'))
        ++pos;
}

static std::string parseString(const char* s, size_t len, size_t& pos) {
    // Skip to opening quote
    while (pos < len && s[pos] != '"') ++pos;
    if (pos >= len) return {};
    ++pos; // consume "

    std::string result;
    result.reserve(64);
    while (pos < len && s[pos] != '"') {
        if (s[pos] == '\\') {
            ++pos;
            if (pos < len) {
                switch (s[pos]) {
                    case '"':  result += '"';  break;
                    case '\\': result += '\\'; break;
                    case '/':  result += '/';  break;
                    case 'n':  result += '\n'; break;
                    case 't':  result += '\t'; break;
                    case 'r':  result += '\r'; break;
                    default:   result += s[pos]; break;
                }
            }
        } else {
            result += s[pos];
        }
        ++pos;
    }
    if (pos < len) ++pos; // consume closing "
    return result;
}

static void parseI18nJson(const char* src, size_t len) {
    size_t pos = 0;
    // Skip outer '{'
    while (pos < len && src[pos] != '{') ++pos;
    if (pos >= len) return;
    ++pos;

    while (pos < len) {
        skipWhitespace(src, len, pos);
        if (pos >= len || src[pos] == '}') break;
        if (src[pos] != '"') { ++pos; continue; }

        // Read language key
        std::string langKey = parseString(src, len, pos);
        skipWhitespace(src, len, pos);
        // Expect ':'
        while (pos < len && src[pos] != ':') ++pos;
        if (pos >= len) break;
        ++pos;
        skipWhitespace(src, len, pos);
        // Expect inner '{'
        while (pos < len && src[pos] != '{') ++pos;
        if (pos >= len) break;
        ++pos;

        StringMap& map = s_langs[langKey];

        // Parse inner key/value pairs
        while (pos < len) {
            skipWhitespace(src, len, pos);
            if (pos >= len || src[pos] == '}') { ++pos; break; }
            if (src[pos] != '"') { ++pos; continue; }

            std::string key   = parseString(src, len, pos);
            skipWhitespace(src, len, pos);
            while (pos < len && src[pos] != ':') ++pos;
            if (pos >= len) break;
            ++pos;
            std::string value = parseString(src, len, pos);
            map[key] = std::move(value);

            skipWhitespace(src, len, pos);
            if (pos < len && src[pos] == ',') ++pos;
        }

        skipWhitespace(src, len, pos);
        if (pos < len && src[pos] == ',') ++pos;
    }
}

// ── Public API ──────────────────────────────────────────────────────────────

void load(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return; }

    std::string buf(static_cast<size_t>(size), '\0');
    fread(&buf[0], 1, static_cast<size_t>(size), f);
    fclose(f);

    parseI18nJson(buf.data(), buf.size());
}

void setLanguage(const std::string& lang) {
    if (s_langs.count(lang))
        s_lang = lang;
    else if (s_langs.count("en"))
        s_lang = "en";
}

void detectSystemLanguage() {
    // Requires setInitialize() to have been called
    u64 langCode = 0;
    if (R_FAILED(setGetSystemLanguage(&langCode))) {
        setLanguage("en");
        return;
    }
    SetLanguage lang = SetLanguage_ENUS;
    setMakeLanguage(langCode, &lang);

    switch (lang) {
        case SetLanguage_JA:    setLanguage("ja"); break;
        case SetLanguage_FR:
        case SetLanguage_FRCA:  setLanguage("fr"); break;
        case SetLanguage_DE:    setLanguage("de"); break;
        case SetLanguage_IT:    setLanguage("it"); break;
        case SetLanguage_ES:
        case SetLanguage_ES419: setLanguage("es"); break;
        default:                setLanguage("en"); break;
    }
}

const char* t(const char* key) {
    // Try active language
    auto langIt = s_langs.find(s_lang);
    if (langIt != s_langs.end()) {
        auto it = langIt->second.find(key);
        if (it != langIt->second.end())
            return it->second.c_str();
    }
    // Fallback to English
    if (s_lang != "en") {
        auto enIt = s_langs.find("en");
        if (enIt != s_langs.end()) {
            auto it = enIt->second.find(key);
            if (it != enIt->second.end())
                return it->second.c_str();
        }
    }
    return key; // Return the key itself as last resort
}

const std::string& currentLanguage() {
    return s_lang;
}

} // namespace I18n
