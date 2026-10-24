//
// Created by jan on 10/24/26.
//

#include "../../inc/Engine/PlutusFile.h"

#include <stdio.h>
#include <string.h>

bool PlutusInit(PlutusFile *file) {
    if (file == nullptr) return false;
    memset(file, 0, sizeof(PlutusFile));
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