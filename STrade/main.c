#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inc/macros.h"
#include "inc/Asset.h"

#define DEBUG_ARGS  "--debug"

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "%s: No data file provided.\nUsage: %s <data_file> [arg1] [arg2] ...\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    // banner
    printf(FMT_BANNER, PRODUCT, VERSION, AUTHOR);

    const char *data_file = argv[1];

    // Allocate array of Asset pointers
    Asset **assets = calloc(MAX_ASSETS, sizeof(Asset *));
    if (assets == NULL) {
        fprintf(stderr, "Failed to allocate memory for assets.\n");
        return EXIT_FAILURE;
    }

    size_t count = 0;
    if (!LoadAssets(data_file, assets, &count)) {
        fprintf(stderr, "Error: Failed to load assets from %s\n", data_file);
        free(assets);
        return EXIT_FAILURE;
    }

    printf("Successfully loaded %zu assets from %s:\n", count, data_file);
    for (size_t i = 0; i < count; i++) {
        printf("  [%zu] ID: %lu, Symbol: %s, Name: %s, Value: %lu cents ($%.2f)\n",
               i + 1,
               assets[i]->id,
               assets[i]->symbol,
               assets[i]->name,
               assets[i]->value,
               (double)assets[i]->value / 100.0);
    }

    // Clean up
    UnloadAssets(assets);
    free(assets);

    return EXIT_SUCCESS;
}
