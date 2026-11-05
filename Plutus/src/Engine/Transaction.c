//
// Created on 10/17/26.
//

#include "../../inc/Engine/Transaction.h"

#include <stdio.h>
#include <inttypes.h>

bool TransactionValidate(const Transaction *pTransaction) {
    if (pTransaction == nullptr) return false;
    if (pTransaction->uId == 0) return false;
    if (pTransaction->uType == TRANSACTION_INVALID) return false;
    if (pTransaction->dValue <= 0.005) return false;
    return true;
}

bool TransactionToCSV(const Transaction* pTransaction, char separator, char *buf, size_t size) {
    if (pTransaction == nullptr) return false;
    if (buf == nullptr || size == 0) return false;

    // prints the CSV representation of the transaction
    const int result = snprintf(buf, size,
        "%" PRIu64 "%c%lf%c%" PRIu8 "%c%" PRIu16 "%c%" PRIu32 "%c%" PRIu8,
        pTransaction->uId,
        separator,
        pTransaction->dValue,
        separator,
        pTransaction->eCategory,
        separator,
        pTransaction->eSubCategory,
        separator,
        pTransaction->uDate,
        separator,
        pTransaction->uType);
    return (result >= 0 && (size_t)result < size);
}