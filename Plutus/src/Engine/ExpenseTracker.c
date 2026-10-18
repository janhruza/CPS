//
// Created by jan on 10/18/26.
//

#include "../../inc/Engine/ExpenseTracker.h"

#include <string.h>

/* Finds the index of uId. Returns false if it is not stored. */
static bool FindIndex(const ExpenseTracker *pTracker, uint64_t uId, size_t *puIndex) {
    for (size_t uIndex = 0; uIndex < pTracker->uCount; uIndex++) {
        if (pTracker->pItems[uIndex].uId == uId) {
            *puIndex = uIndex;
            return true;
        }
    }
    return false;
}

bool ExpenseTrackerInit(ExpenseTracker *pTracker, Transaction *pItems, size_t uCapacity) {
    if (pTracker == NULL || pItems == NULL || uCapacity == 0) return false;

    pTracker->pItems = pItems;
    pTracker->uCapacity = uCapacity;
    pTracker->uCount = 0;
    return true;
}

bool ExpenseTrackerAdd(ExpenseTracker *pTracker, const Transaction *pTransaction) {
    if (pTracker == NULL || pTransaction == NULL) return false;
    if (pTracker->uCount >= pTracker->uCapacity) return false;
    if (!TransactionValidate(pTransaction)) return false;

    size_t uIndex;
    if (FindIndex(pTracker, pTransaction->uId, &uIndex)) return false;   /* duplicate id */

    pTracker->pItems[pTracker->uCount] = *pTransaction;
    pTracker->uCount++;
    return true;
}

bool ExpenseTrackerUpdate(ExpenseTracker *pTracker, const Transaction *pTransaction) {
    if (pTracker == NULL || pTransaction == NULL) return false;
    if (!TransactionValidate(pTransaction)) return false;

    size_t uIndex;
    if (!FindIndex(pTracker, pTransaction->uId, &uIndex)) return false;

    pTracker->pItems[uIndex] = *pTransaction;   /* same slot, so insertion order is kept */
    return true;
}

bool ExpenseTrackerRemove(ExpenseTracker *pTracker, uint64_t uId) {
    if (pTracker == NULL) return false;

    size_t uIndex;
    if (!FindIndex(pTracker, uId, &uIndex)) return false;

    /* Shift the tail left so insertion order is preserved. */
    size_t uTail = pTracker->uCount - uIndex - 1;
    memmove(&pTracker->pItems[uIndex], &pTracker->pItems[uIndex + 1],
            uTail * sizeof(Transaction));
    pTracker->uCount--;
    return true;
}

bool ExpenseTrackerFind(const ExpenseTracker *pTracker, uint64_t uId,
                        Transaction *pTransaction) {
    if (pTracker == NULL || pTransaction == NULL) return false;

    size_t uIndex;
    if (!FindIndex(pTracker, uId, &uIndex)) return false;

    *pTransaction = pTracker->pItems[uIndex];
    return true;
}

bool ExpenseTrackerGet(const ExpenseTracker *pTracker, size_t uIndex,
                       Transaction *pTransaction) {
    if (pTracker == NULL || pTransaction == NULL) return false;
    if (uIndex >= pTracker->uCount) return false;

    *pTransaction = pTracker->pItems[uIndex];
    return true;
}

double ExpenseTrackerSum(const ExpenseTracker *pTracker, uint8_t eType,
                         uint32_t uFromDate, uint32_t uToDate) {
    if (pTracker == NULL) return 0.0;

    double dSum = 0.0;
    for (size_t uIndex = 0; uIndex < pTracker->uCount; uIndex++) {
        const Transaction *pItem = &pTracker->pItems[uIndex];
        if (pItem->eType == eType &&
            pItem->uDate >= uFromDate && pItem->uDate <= uToDate) {
            dSum += pItem->dValue;
        }
    }
    return dSum;
}

double ExpenseTrackerBalance(const ExpenseTracker *pTracker) {
    if (pTracker == NULL) return 0.0;

    double dBalance = 0.0;
    for (size_t uIndex = 0; uIndex < pTracker->uCount; uIndex++) {
        const Transaction *pItem = &pTracker->pItems[uIndex];
        if (pItem->eType == TRANSACTION_INCOME) {
            dBalance += pItem->dValue;
        } else if (pItem->eType == TRANSACTION_EXPENSE) {
            dBalance -= pItem->dValue;
        }
    }
    return dBalance;
}