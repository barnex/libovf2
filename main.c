/*
 * Example usage: read ovf files specified on command-line,
 * print their content to stdout.
 *
 * libovf2.c is intended to poach into your source.
 */

#include "libovf2.h"
#include <stdio.h>

int main(int argc, char **argv) {
    for(int i=1; i<argc; i++) {
        ovf2_data d = ovf2_readfile(argv[i]);
        ovf2_write(stdout, d);
		ovf2_free(&d);
    }

    return 0;
}
