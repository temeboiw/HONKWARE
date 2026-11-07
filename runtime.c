#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (size_t i = 0; i < n; i++)
        d[i] = s[i];

    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else if (d > s) {
        while (n > 0) {
            n--;
            d[n] = s[n];
        }
    }

    return dest;
}

void *memset(void *dest, int c, size_t n) {
    unsigned char *d = dest;

    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)c;

    return dest;
}
