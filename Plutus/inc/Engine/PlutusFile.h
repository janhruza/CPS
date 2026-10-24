//
// Created by jan on 10/24/26.
//

#ifndef PLUTUS_PLUTUSFILE_H
#define PLUTUS_PLUTUSFILE_H

#include "ExpenseTracker.h"
#include "Transaction.h"

#define PLUTUS_MAX_TRANSACTIONS 131072
#define PLUTUS_SIGNATURE        "PLTSFILE"

/* Representing the plutus file structure. */
typedef struct tagPlutusFile {
    /* First 8 bytes of the file. Must match with footer. */
    char header[8];

    /* Position of the next transaction within the 'transactions' array. */
    uint64_t uNextPos;

    /* All stored transactions. */
    Transaction transactions[PLUTUS_MAX_TRANSACTIONS];

    /* Last 8 bytes of the file. Must match with header. */
    char footer[8];
} PlutusFile;

PlutusFile* PlutusCreate();
void PlutusDestroy(PlutusFile *file);

bool PlutusInit(PlutusFile *file);
bool PlutusSave(const char* filename, const PlutusFile *file);
bool PlutusLoad(const char* filename, PlutusFile *file);
bool PlutusPutTransactions(const Transaction *transactions, uint64_t count, PlutusFile *file);

#endif //PLUTUS_PLUTUSFILE_H
