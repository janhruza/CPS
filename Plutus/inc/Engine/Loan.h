//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_LOAN_H
#define PLUTUS_LOAN_H

#include <stdbool.h>

/* Any loan: specify the inputs, then read the outputs. Whole currency units. */
typedef struct tagLoan {
    /* Inputs */
    long long llPrincipal;     /* amount borrowed */
    int iCurrencyDigits;       /* ISO 4217 minor-unit exponent: 2 for CZK, USD, EUR; 0 for JPY */
    double dRate;              /* annual rate, 0.0525 = 5.25% */
    int iTermMonths;

    /* Outputs */
    long long llPayment;       /* regular monthly payment */
    long long llTotalInterest;
    long long llTotal;         /* llPrincipal + llTotalInterest */
} Loan;

bool LoanCalculate(Loan *pLoan);

#endif //PLUTUS_LOAN_H