/*
 * strndup16to8() was dropped from libutils in R. Nougat nvram_agent_binder uses it
 * to turn binder String16 arguments into file names. Same contract as the old
 * libcutils/libutils version: a malloc()ed, NUL-terminated UTF-8 copy of the first
 * n UTF-16 units, or NULL.
 */
#include <stdlib.h>
#include <sys/types.h>

#include <utils/Unicode.h>

extern "C" char* strndup16to8(const char16_t* s, size_t n) {
    if (s == nullptr) {
        return nullptr;
    }
    ssize_t len = utf16_to_utf8_length(s, n);
    if (len < 0) {
        return nullptr;
    }
    char* out = static_cast<char*>(malloc(len + 1));
    if (out == nullptr) {
        return nullptr;
    }
    utf16_to_utf8(s, n, out, len + 1);
    return out;
}
