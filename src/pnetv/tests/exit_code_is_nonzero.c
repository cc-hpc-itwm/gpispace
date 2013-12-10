#include <stdlib.h>

int main(int argc, const char *argv[]) {
    char command[16380];

    char *dst = &command[0];
    char *end = &command[sizeof(command) - 1];

    int i;
    for (i = 1; i < argc; ++i) {
        const char *src;
        if (dst == end) {
            return 2;
        }
        if (i != 1) {
            *dst++ = ' ';
        }
        for (src = argv[i]; dst != end && *src; ++src, ++dst) {
            *dst = *src;
        }
    }
    *dst = 0;

    return !system(command);
}

/* vim:set et sts=4 sw=4: */
