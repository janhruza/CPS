//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_LOAN_H
#define PLUTUS_LOAN_H

typedef struct tagLoan {
    long long llPrincipal;     /* whole units */
    int iCurrencyDigits;
    double dRate;              /* annual, 0.0525 = 5.25% */
    int iTermMonths;

    /* outputs */
    long long llPayment;
    long long llTotalInterest;
} Loan;

bool LoanCalculate(Loan *pLoan);

#endif //PLUTUS_LOAN_H
