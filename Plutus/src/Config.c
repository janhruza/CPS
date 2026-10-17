//
// Created by jan on 10/17/26.
//

#include <stdlib.h>
#include <stdio.h>

#include "../inc/Config.h"

bool ConfigRead(const Config *pConfig) {
    if (pConfig == nullptr) return false;
    const FILE* f = fopen(CONFIG_FILE_NAME, "rb");
    if (f == nullptr) return false;

    bool ret = true;
    const size_t read = fread((void*)pConfig, sizeof(Config), 1, (FILE*)f);
    if (read != sizeof(Config)) {
        ret = false;
    }

    fclose((FILE*)f);
    return ret;
}

bool ConfigWrite(const Config *pConfig) {
    if (pConfig == nullptr) return false;

    bool ret = true;

    const FILE* f = fopen(CONFIG_FILE_NAME, "wb");
    if (f == nullptr) return false;

    const size_t written = fwrite(pConfig, sizeof(Config), 1, (FILE*)f);
    if (written != sizeof(Config)) {
        ret = false;
    }

    fclose((FILE*)f);
    return ret;
}