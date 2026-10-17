//
// Created by jan on 10/17/26.
//

#include <math.h>
#include "../inc/Mortgage.h"

/* Everything in this file works in minor units (cents, haléře)
   except the final conversion when results are written out. */

static long long Pow10(int iDigits) {
    long long llPower = 1;
    for (int iIndex = 0; iIndex < iDigits; iIndex++) {
        llPower *= 10;
    }
    return llPower;
}

static long long ToWhole(long long llMinor, long long llScale) {
    return (llMinor + llScale / 2) / llScale;   /* round half up, amounts are non-negative */
}

static long long MonthlyPaymentMinor(long long llLoanMinor, double dRate, int iPayments) {
    if (dRate == 0.0) {
        return llround((double)llLoanMinor / iPayments);
    }
    double dFactor = pow(1.0 + dRate, iPayments);
    return llround((double)llLoanMinor * dRate * dFactor / (dFactor - 1.0));
}

/* Runs the schedule. The last payment clears whatever balance remains.
   Returns true if the balance ends at exactly zero. */
static bool RunSchedule(long long llLoanMinor, long long llPaymentMinor, double dRate,
                        int iPayments, long long *pllTotalInterestMinor) {
    long long llBalance = llLoanMinor;
    long long llTotal = 0;

    for (int iMonth = 1; iMonth <= iPayments; iMonth++) {
        long long llInterest  = llround((double)llBalance * dRate);
        long long llPrincipal = llPaymentMinor - llInterest;

        if (iMonth == iPayments || llPrincipal > llBalance) {
            llPrincipal = llBalance;
        }

        llBalance -= llPrincipal;
        llTotal   += llInterest;
    }

    *pllTotalInterestMinor = llTotal;
    return llBalance == 0;
}

bool MortgageValidate(const Mortgage *pMortgage) {
    if (pMortgage == nullptr) return false;
    if (pMortgage->llHomeValue <= 0 || pMortgage->iTermYears <= 0) return false;
    if (pMortgage->iCurrencyDigits < 0 || pMortgage->iCurrencyDigits > 6) return false;
    if (!isfinite(pMortgage->dInterest) || pMortgage->dInterest < 0.0) return false;
    if (pMortgage->eDownPayment != DownPaymentLow &&
        pMortgage->eDownPayment != DownPaymentHigh) return false;
    return true;
}

bool MortgageCalculate(Mortgage *pMortgage) {
    if (!MortgageValidate(pMortgage)) return false;

    const long long llScale = Pow10(pMortgage->iCurrencyDigits);
    const int iPayments = pMortgage->iTermYears * 12;
    const double dRate = pMortgage->dInterest / 12.0;

    /* Down payment and loan in whole units. The loan is the remainder,
       so llDownPayment + llLoan == llHomeValue always holds. */
    const long long llPercent = (pMortgage->eDownPayment == DownPaymentLow) ? 10 : 20;
    const long long llDown = pMortgage->llHomeValue * llPercent / 100;
    const long long llLoan = pMortgage->llHomeValue - llDown;

    /* Everything from here to the conversions is in minor units. */
    const long long llLoanMinor    = llLoan * llScale;
    const long long llPaymentMinor = MonthlyPaymentMinor(llLoanMinor, dRate, iPayments);

    long long llTotalInterestMinor = 0;
    const bool bOk = RunSchedule(llLoanMinor, llPaymentMinor, dRate, iPayments,
                                 &llTotalInterestMinor);

    pMortgage->llDownPayment    = llDown;
    pMortgage->llLoan           = llLoan;
    pMortgage->llMonthlyPayment = ToWhole(llPaymentMinor, llScale);
    pMortgage->llTotalInterest  = ToWhole(llTotalInterestMinor, llScale);
    pMortgage->llTotalPaid      = llLoan + pMortgage->llTotalInterest;

    return bOk;
}