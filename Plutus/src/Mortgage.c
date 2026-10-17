//
// Created by jan on 10/17/26.
//

#include <math.h>
#include <stddef.h>
#include "../inc/Mortgage.h"
#include "../inc/Loan.h"

bool MortgageValidate(const Mortgage *pMortgage) {
    if (pMortgage == NULL) return false;
    if (pMortgage->llHomeValue <= 0 || pMortgage->iTermYears <= 0) return false;
    if (pMortgage->eDownPayment != DownPaymentLow &&
        pMortgage->eDownPayment != DownPaymentHigh) return false;
    return true;
}

bool MortgageCalculate(Mortgage *pMortgage) {
    if (!MortgageValidate(pMortgage)) return false;

    /* Down payment in whole units. The loan is the remainder,
       so llDownPayment + llLoan == llHomeValue always holds. */
    const long long llPercent = (pMortgage->eDownPayment == DownPaymentLow) ? 10 : 20;
    const long long llDown = pMortgage->llHomeValue * llPercent / 100;
    const long long llLoan = pMortgage->llHomeValue - llDown;

    /* The generic schedule does the rest. */
    Loan oLoan = {
        .llPrincipal     = llLoan,
        .iCurrencyDigits = pMortgage->iCurrencyDigits,
        .dRate           = pMortgage->dInterest,
        .iTermMonths     = pMortgage->iTermYears * 12
    };

    if (!LoanCalculate(&oLoan)) return false;

    pMortgage->llDownPayment    = llDown;
    pMortgage->llLoan           = llLoan;
    pMortgage->llMonthlyPayment = oLoan.llPayment;
    pMortgage->llTotalInterest  = oLoan.llTotalInterest;
    pMortgage->llTotalPaid      = llLoan + oLoan.llTotalInterest;

    return true;
}