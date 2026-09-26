//
// Created by jan on 26/09/2026.
//

#include <stdlib.h>
#include <stdio.h>
#include "../inc/Version.h"

int SetVersion(Version *version) {
    if (!version) return -1;

    FILE *f = fopen("version", "w");
    if (!f) return -1;

    fwrite(version, sizeof(Version), 1, f);
    fclose(f);
    return 0;
}

int GetVersion(Version *version) {
    if (!version) return -1;

    FILE *f = fopen("version", "r");
    if (!f) return -1;

    fread(version, sizeof(Version), 1, f);
    fclose(f);
    return 0;
}