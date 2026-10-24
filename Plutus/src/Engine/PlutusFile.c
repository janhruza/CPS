//
// Created by jan on 10/24/26.
//

#include "../../inc/Engine/PlutusFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool PlutusInit(PlutusFile *file) {
    if (file == nullptr) return false;
    memset(file, 0, sizeof(PlutusFile));

    static constexpr char buf[8] = {'P', 'L', 'T', 'S', 'F', 'I', 'L', 'E'};
    memcpy(file->header, buf, sizeof buf);
    memcpy(file->footer, buf, sizeof buf);

    return true;
}

bool PlutusSave(const char* filename, const PlutusFile *file) {
    if (file == nullptr) return false;

    FILE* fp = fopen(filename, "wb");
    if (fp == nullptr) return false;

    const size_t result = fwrite(file, sizeof(PlutusFile), 1, fp);
    fclose(fp);

    return result == 1;
}

bool PlutusLoad(const char* filename, PlutusFile *file) {
    if (file == nullptr) return false;

    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr) return false;

    const size_t result = fread(file, sizeof(PlutusFile), 1, fp);
    fclose(fp);

    return result == 1;
}

bool PlutusPutTransactions(const Transaction *transactions, const uint64_t count, PlutusFile *file) {
    if (file == nullptr) return false;
    if (count > PLUTUS_MAX_TRANSACTIONS) return false;
    if (count > 0 && transactions == nullptr) return false;

    file->uNextPos = count;
    if (count > 0) {
        memcpy(file->transactions, transactions, count * sizeof(Transaction));
    }

    return true;
}

PlutusFile* PlutusCreate() {
    return calloc(1, sizeof(PlutusFile));
}

void PlutusDestroy(PlutusFile *file) {
    free(file);
    return;
}

void PlutusDestroy(PlutusFile *file);