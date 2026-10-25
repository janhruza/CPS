//
// Created by jan on 10/17/26.
//

#include <stddef.h>
#include "../../inc/Engine/Mortgage.h"

bool MortgageCalculate(Mortgage *pMortgage) {
    if (pMortgage == NULL) return false;
    if (pMortgage->llHomeValue <= 0) return false;
    if (pMortgage->eDownPayment != DOWN_PAY_LOW &&
        pMortgage->eDownPayment != DOWN_PAY_HIGH) return false;

    /* The loan is the home value minus the down payment. */
    const long long llPercent = (pMortgage->eDownPayment == DOWN_PAY_LOW) ? 10 : 20;
    pMortgage->llDownPayment = pMortgage->llHomeValue * llPercent / 100;
    pMortgage->sLoan.llPrincipal = pMortgage->llHomeValue - pMortgage->llDownPayment;

    return LoanCalculate(&pMortgage->sLoan);
}