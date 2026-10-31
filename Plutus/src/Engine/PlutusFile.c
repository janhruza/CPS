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
    if (file == nullptr || filename == nullptr) return false;
    if (file->uNextPos > PLUTUS_MAX_TRANSACTIONS) return false;

    FILE* fp = fopen(filename, "wb");
    if (fp == nullptr) return false;

    bool ok = (fwrite(file->header, PLUTUS_SIGNATURE_SIZE, 1, fp) == 1) &&
              (fwrite(&file->uNextPos, sizeof(file->uNextPos), 1, fp) == 1);

    if (ok && file->uNextPos > 0) {
        ok = (fwrite(file->transactions, sizeof(Transaction), file->uNextPos, fp) == file->uNextPos);
    }

    if (ok) {
        ok = (fwrite(file->footer, PLUTUS_SIGNATURE_SIZE, 1, fp) == 1);
    }

    fclose(fp);
    return ok;
}

bool PlutusLoad(const char* filename, PlutusFile *file) {
    if (file == nullptr || filename == nullptr) return false;

    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr) return false;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return false;
    }
    long fsize = ftell(fp);
    rewind(fp);

    static constexpr char sig[PLUTUS_SIGNATURE_SIZE] = {'P', 'L', 'T', 'S', 'F', 'I', 'L', 'E'};

    if (fsize == (long)sizeof(PlutusFile)) {
        // legacy fixed-size format
        const size_t result = fread(file, sizeof(PlutusFile), 1, fp);
        fclose(fp);
        if (result != 1) return false;
        return memcmp(file->header, sig, sizeof sig) == 0 &&
               memcmp(file->footer, sig, sizeof sig) == 0 &&
               file->uNextPos <= PLUTUS_MAX_TRANSACTIONS;
    }

    if (fsize < (long)(2 * PLUTUS_SIGNATURE_SIZE + sizeof(uint64_t))) {
        fclose(fp);
        return false;
    }

    PlutusInit(file);

    if (fread(file->header, PLUTUS_SIGNATURE_SIZE, 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    if (fread(&file->uNextPos, sizeof(file->uNextPos), 1, fp) != 1 ||
        file->uNextPos > PLUTUS_MAX_TRANSACTIONS) {
        fclose(fp);
        return false;
    }

    long expectedSize = (long)(2 * PLUTUS_SIGNATURE_SIZE + sizeof(uint64_t) + file->uNextPos * sizeof(Transaction));
    if (fsize != expectedSize) {
        fclose(fp);
        return false;
    }

    if (file->uNextPos > 0) {
        if (fread(file->transactions, sizeof(Transaction), file->uNextPos, fp) != file->uNextPos) {
            fclose(fp);
            return false;
        }
    }

    if (fread(file->footer, PLUTUS_SIGNATURE_SIZE, 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    fclose(fp);

    return memcmp(file->header, sig, sizeof sig) == 0 &&
           memcmp(file->footer, sig, sizeof sig) == 0;
}

bool PlutusPutTransactions(const Transaction *transactions, const uint64_t count, PlutusFile *file) {
    if (file == nullptr) return false;
    if (count > PLUTUS_MAX_TRANSACTIONS) return false;
    if (count > 0 && transactions == nullptr) return false;

    file->uNextPos = count;
    if (count > 0 && file->transactions != transactions) {
        memcpy(file->transactions, transactions, count * sizeof(Transaction));
    }

    return true;
}

PlutusFile* PlutusCreate() {
    return calloc(1, sizeof(PlutusFile));
}

void PlutusDestroy(PlutusFile *file) {
    free(file);
}