//
// Created by jan on 10/17/26.
//

#include "../../inc/Engine/Transaction.h"

#include <stdio.h>

bool TransactionValidate(const Transaction *pTransaction) {
    if (pTransaction == nullptr) return false;
    if (pTransaction->uId == 0) return false;
    if (pTransaction->eType == TRANSACTION_INVALID) return false;
    if (pTransaction->dValue <= 0.005) return false;
    return true;
}

bool TransactionToCSV(const Transaction* pTransaction, char separator, char *buf, size_t size) {
    if (pTransaction == nullptr) return false;
    if (buf == nullptr) return false;

    // prints the CSV representation of the transaction
    const int result = snprintf(buf, size,
        "%zu%c%zu%c%lf%c%u%c%d",
        pTransaction->uId,
        separator,
        pTransaction->uCreationTimeMs,
        separator,
        pTransaction->dValue,
        separator,
        pTransaction->uDate,
        separator,
        pTransaction->eType);
    return result <= 0 ? false : true;
}