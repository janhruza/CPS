#include <stdlib.h>
#include <stdio.h>

#include "inc/macros.h"

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "%s: No data file provided.\nUsage: %s <data_file> [arg1] [arg2] ...\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    // banner
    printf(FMT_BANNER, PRODUCT, VERSION, AUTHOR);

    return EXIT_SUCCESS;
}
