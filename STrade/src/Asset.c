#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/Asset.h"

#define LINE_BUF_SIZE 512

/* Creates a new asset. The strings are copied, so the caller keeps ownership of its own buffers. */
Asset* CreateAsset(unsigned long id, const char* name, const char* symbol, unsigned long value) {
    if (name == nullptr || symbol == nullptr) return nullptr;

    Asset* a = calloc(1, sizeof *a);
    if (a == nullptr) return nullptr;

    a->name = strdup(name);
    a->symbol = strdup(symbol);
    if (a->name == nullptr || a->symbol == nullptr) {
        free(a->name);     // free(nullptr) is safe
        free(a->symbol);
        free(a);
        return nullptr;
    }

    a->id = id;
    a->value = value;
    return a;
}

/* Frees an asset and its strings. */
bool DeleteAsset(Asset* asset) {
    if (asset == nullptr) return false;
    free(asset->symbol);
    free(asset->name);
    free(asset);
    return true;
}

/* Splits off the next ';'-separated field. Returns nullptr when there is no field left. */
static char* NextField(char** cursor) {
    char* start = *cursor;
    if (start == nullptr) return nullptr;

    char* sep = strchr(start, ';');
    if (sep != nullptr) {
        *sep = '\0';
        *cursor = sep + 1;
    } else {
        *cursor = nullptr;
    }
    return start;
}

/* Parses a non-negative base-10 integer. Rejects empty strings and trailing garbage. */
static bool ParseUnsignedLong(const char* s, unsigned long* out) {
    if (s == nullptr || *s == '\0' || *s == '-') return false;

    char* end = nullptr;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return false;

    *out = v;
    return true;
}

/* Expects a line in the form: id;name;symbol;value. Modifies the line in place. */
static Asset* ParseAssetLine(char* line) {
    char* cursor = line;
    char* id_s    = NextField(&cursor);
    char* name    = NextField(&cursor);
    char* symbol  = NextField(&cursor);
    char* value_s = NextField(&cursor);

    if (id_s == nullptr || name == nullptr || symbol == nullptr || value_s == nullptr) return nullptr;
    if (cursor != nullptr) return nullptr;   // more than four fields
    if (name[0] == '\0' || symbol[0] == '\0') return nullptr;

    unsigned long id, value;
    if (!ParseUnsignedLong(id_s, &id)) return nullptr;
    if (!ParseUnsignedLong(value_s, &value)) return nullptr;

    return CreateAsset(id, name, symbol, value);  // CreateAsset copies the strings
}

static bool HasAssetId(Asset** assets, size_t count, unsigned long id) {
    for (size_t i = 0; i < count; i++) {
        if (assets[i]->id == id) return true;
    }
    return false;
}

/**
 * Loads assets from a semicolon-separated file: id;name;symbol;value
 * The first line is a header and is skipped.
 *
 * `assets` must point to an array of MAX_ASSETS pointers, all set to nullptr
 * (for example, from calloc). On success the first *count slots are filled.
 * On failure, every asset created so far is freed and *count is 0.
 */
bool LoadAssets(const char* filename, Asset** assets, size_t* count) {
    if (filename == nullptr || assets == nullptr || count == nullptr) return false;

    *count = 0;

    FILE* fp = fopen(filename, "r");
    if (fp == nullptr) return false;

    char line[LINE_BUF_SIZE];
    bool is_header = true;
    bool ok = true;

    while (fgets(line, sizeof line, fp) != nullptr) {
        // A line that fills the buffer without a newline is too long
        size_t len = strlen(line);
        if (len == sizeof line - 1 && line[len - 1] != '\n' && !feof(fp)) {
            ok = false;
            break;
        }

        if (is_header) {
            is_header = false;
            continue;
        }

        // Strip the line ending
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;   // skip blank lines

        if (*count >= MAX_ASSETS) {
            ok = false;
            break;
        }

        Asset* a = ParseAssetLine(line);
        if (a == nullptr) {
            ok = false;
            break;
        }

        if (HasAssetId(assets, *count, a->id)) {
            DeleteAsset(a);
            ok = false;
            break;
        }

        assets[(*count)++] = a;
    }

    if (ferror(fp)) ok = false;
    fclose(fp);

    if (!ok || *count == 0) {
        UnloadAssets(assets);   // frees everything loaded so far
        *count = 0;
        return false;
    }

    return true;
}

/**
 * Frees every asset in the array and sets each slot to nullptr.
 * The caller still owns and must free the pointer array itself.
 */
bool UnloadAssets(Asset** assets) {
    if (assets == nullptr) return false;

    for (size_t i = 0; i < MAX_ASSETS; i++) {
        if (assets[i] != nullptr) {
            DeleteAsset(assets[i]);
            assets[i] = nullptr;
        }
    }
    return true;
}