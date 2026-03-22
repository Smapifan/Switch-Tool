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

// ── Minimal JSON parser (handles only the i18n structure) ──────────────────

static void skipWhitespace(const char* s, size_t len, size_t& pos) {
    while (pos < len && (s[pos] == ' ' || s[pos] == '\t' ||
                         s[pos] == '\r' || s[pos] == '\n'))
        ++pos;
}

static std::string parseString(const char* s, size_t len, size_t& pos) {
    while (pos < len && s[pos] != '"') ++pos;
    if (pos >= len) return {};
    ++pos;

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
    if (pos < len) ++pos;
    return result;
}

/// Parse flat { "key": "value" } into a StringMap.
static void parseFlatJson(const char* src, size_t len, StringMap& map) {
    size_t pos = 0;
    while (pos < len && src[pos] != '{') ++pos;
    if (pos >= len) return;
    ++pos;

    while (pos < len) {
        skipWhitespace(src, len, pos);
        if (pos >= len || src[pos] == '}') break;
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
}

/// Parse nested { "lang": { "key": "value" } } into s_langs.
static void parseI18nJson(const char* src, size_t len) {
    size_t pos = 0;
    while (pos < len && src[pos] != '{') ++pos;
    if (pos >= len) return;
    ++pos;

    while (pos < len) {
        skipWhitespace(src, len, pos);
        if (pos >= len || src[pos] == '}') break;
        if (src[pos] != '"') { ++pos; continue; }

        std::string langKey = parseString(src, len, pos);
        skipWhitespace(src, len, pos);
        while (pos < len && src[pos] != ':') ++pos;
        if (pos >= len) break;
        ++pos;
        skipWhitespace(src, len, pos);
        while (pos < len && src[pos] != '{') ++pos;
        if (pos >= len) break;
        ++pos;

        StringMap& map = s_langs[langKey];

        while (pos < len) {
            skipWhitespace(src, len, pos);
            if (pos >= len || src[pos] == '}') { ++pos; break; }
            if (src[pos] != '"') { ++pos; continue; }

            std::string key   = parseString(src, len, pos);
            skipWhitespace(src, len,

