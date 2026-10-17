//
// Created by jan on 10/17/26.
//

#include "../inc/Transaction.h"

bool TransactionValidate(const Transaction *pTransaction) {
    if (pTransaction == nullptr) return false;
    if (pTransaction->uId == 0) return false;
    if (pTransaction->eType == TRANSACTION_INVALID) return false;
    if (pTransaction->dValue <= 0.005) return false;
    return true;
}