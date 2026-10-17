//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_MORTGAGE_H
#define PLUTUS_MORTGAGE_H

#include <stdbool.h>
#include "Loan.h"

typedef enum {
    DownPaymentLow,   /* 10% */
    DownPaymentHigh   /* 20% */
} DownPaymentType;

/* Usage:
   1) set llHomeValue, eDownPayment, and the sLoan inputs (iCurrencyDigits, dRate, iTermMonths)
   2) call MortgageCalculate
   3) read llDownPayment and the sLoan outputs (llPayment, llTotalInterest, llTotal) */
typedef struct tagMortgage {
    /* Inputs */
    long long llHomeValue;
    DownPaymentType eDownPayment;

    /* Outputs */
    long long llDownPayment;

    /* The loan this mortgage is based on. Its principal is set from the home value. */
    Loan sLoan;
} Mortgage;

bool MortgageCalculate(Mortgage *pMortgage);

#endif //PLUTUS_MORTGAGE_H