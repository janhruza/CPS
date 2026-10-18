//
// Created by jan on 10/18/26.
//

#ifndef PLUTUS_EXPENSETRACKER_H
#define PLUTUS_EXPENSETRACKER_H

#include <stddef.h>
#include <stdint.h>
#include "Transaction.h"

/* A list of transactions backed by an array the caller owns.
   Usage:
   1) allocate storage (static array or malloc) with the capacity you need
   2) call ExpenseTrackerInit
   3) add, update, remove, and query through the functions below */
typedef struct tagExpenseTracker {
    Transaction *pItems;   /* caller-owned storage */
    size_t uCapacity;      /* number of slots in pItems */
    size_t uCount;         /* number of stored transactions */
} ExpenseTracker;

/* Sets up an empty tracker over the given storage. Returns false if pTracker
   or pItems is NULL, or uCapacity is 0. */
bool ExpenseTrackerInit(ExpenseTracker *pTracker, Transaction *pItems, size_t uCapacity);

/* Validates and appends a transaction. Returns false if the transaction is
   invalid, its uId already exists, or the tracker is full. */
bool ExpenseTrackerAdd(ExpenseTracker *pTracker, const Transaction *pTransaction);

/* Replaces the stored transaction with the same uId. Returns false if the new
   values are invalid or no transaction has that uId. */
bool ExpenseTrackerUpdate(ExpenseTracker *pTracker, const Transaction *pTransaction);

/* Removes the transaction with the given uId. Returns false if it does not exist. */
bool ExpenseTrackerRemove(ExpenseTracker *pTracker, uint64_t uId);

/* Copies the transaction with the given uId into pTransaction. Returns false if
   it does not exist. */
bool ExpenseTrackerFind(const ExpenseTracker *pTracker, uint64_t uId,
                        Transaction *pTransaction);

/* Copies the transaction at uIndex (0 to uCount - 1) into pTransaction.
   Returns false if the index is out of range. Use this to list transactions. */
bool ExpenseTrackerGet(const ExpenseTracker *pTracker, size_t uIndex,
                       Transaction *pTransaction);

/* Sums the values of transactions of the given type whose dates fall within
   [uFromDate, uToDate], inclusive. Dates are YYYYMMDD. The result is a plain
   double, and the caller rounds it for display. Returns 0.0 for a NULL tracker. */
double ExpenseTrackerSum(const ExpenseTracker *pTracker, uint8_t eType,
                         uint32_t uFromDate, uint32_t uToDate);

/* Income minus expense over all stored transactions. Returns 0.0 for a NULL tracker. */
double ExpenseTrackerBalance(const ExpenseTracker *pTracker);

#endif //PLUTUS_EXPENSETRACKER_H
