//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_TRANSACTION_H
#define PLUTUS_TRANSACTION_H

#include <stdint.h>

enum {
    TRANSACTION_INVALID,
    TRANSACTION_INCOME,
    TRANSACTION_EXPENSE
};

typedef struct tagTransaction {

    uint64_t uId;
    uint64_t uCreationTimeMs;
    double dValue;
    uint32_t uDate;
    uint8_t eType;

} Transaction;

/**
 * Validates the gven transaction.
 * @param pTransaction Pointer to a transaction object.
 * @return true if the transaction is valid, otherwise false.
 */
bool TransactionValidate(const Transaction *pTransaction);

#endif //PLUTUS_TRANSACTION_H
