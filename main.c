#include <stdio.h>
#include "libovf2.h"

int main(int argc, char **argv) {
    for(int i=1; i<argc; i++) {
        ovf2_data d = ovf2_readfile(argv[i]);
        ovf2_write(stdout, d);
		//ovf2_free(&d);
    }

    return 0;
}
