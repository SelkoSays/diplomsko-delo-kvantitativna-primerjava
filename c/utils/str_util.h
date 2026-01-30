#ifndef STR_UTIL_H
#define STR_UTIL_H

#include "common.h"

static inline isize str_find(const char *s, char c) {
    if (s == NULL) {
        return -1;
    }

    isize idx = 0;
    while (s[idx] != '\0') {
        if (s[idx] == c) {
            return idx;
        }
        idx++;
    }

    return -1;
}

#endif // STR_UTIL_H
