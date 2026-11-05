//
// Created on 10/17/26.
//

#include <stdlib.h>
#include <stdio.h>

#include "../inc/Config.h"

bool ConfigRead(Config *pConfig) {
    if (pConfig == nullptr) return false;
    FILE* f = fopen(CONFIG_FILE_NAME, "rb");
    if (f == nullptr) return false;

    const size_t read = fread(pConfig, sizeof(Config), 1, f);
    fclose(f);

    return read == 1;
}

bool ConfigWrite(const Config *pConfig) {
    if (pConfig == nullptr) return false;

    FILE* f = fopen(CONFIG_FILE_NAME, "wb");
    if (f == nullptr) return false;

    const size_t written = fwrite(pConfig, sizeof(Config), 1, f);
    fclose(f);

    return written == 1;
}