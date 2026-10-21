//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_TRANSACTION_H
#define PLUTUS_TRANSACTION_H

#include <stdint.h>
#include <stddef.h>

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

/**
 *
 * @param pTransaction Pointer to the transaction object.
 * @param separator Value delimiter (separator).
 * @param buf Pointer to a character buffer.
 * @param size Size of the output buffer.
 * @return true if the entire transaction was written to the buffer, otherwise false.
 */
bool TransactionToCSV(const Transaction *pTransaction, char separator, char* buf, size_t size);

#endif //PLUTUS_TRANSACTION_H
