//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_TRANSACTION_H
#define PLUTUS_TRANSACTION_H

#include <stdint.h>
#include <stddef.h>

enum {
    TRANSACTION_INVALID,    /* Invalid type. */
    TRANSACTION_INCOME,     /* Income type. */
    TRANSACTION_EXPENSE,    /* Expense type. */
    TRANSACTION_CUSTOM      /* Custom, user defined, transaction types starts at this value. */
};

typedef struct tagTransaction {
    /* Representing the transaction identifier. */
    uint64_t uId;

    /* Representing the value of the transaction. */
    double dValue;

    /* Representing the date of the transaction. */
    uint32_t uDate;

    /* Representing the transaction type - income or expense. */
    uint8_t uType;

    /* Representing the main category of the transaction. */
    uint8_t eCategory;

    /* Representing the subcategory of the transaction. */
    uint16_t eSubCategory;

} Transaction;

// assertions
_Static_assert(sizeof(Transaction) == 24, "Transaction layout changed");
_Static_assert(offsetof(Transaction, uDate) == 16, "uDate offset changed");
_Static_assert(offsetof(Transaction, uType) == 20, "uType offset changed");
_Static_assert(offsetof(Transaction, eCategory) == 21, "eCategory offset changed");
_Static_assert(offsetof(Transaction, eSubCategory) == 22, "eSubCategory offset changed");

/**
 * Validates the given transaction.
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
