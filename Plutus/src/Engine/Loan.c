//
// Created by jan on 10/17/26.
//

#include <math.h>
#include <stddef.h>
#include "../../inc/Engine/Loan.h"

/* Everything here works in minor units (cents, haléře) except the final conversion. */

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

static long long MonthlyPaymentMinor(long long llPrincipalMinor, double dRate, int iPayments) {
    if (dRate == 0.0) {
        return llround((double)llPrincipalMinor / iPayments);
    }
    double dFactor = pow(1.0 + dRate, iPayments);
    return llround((double)llPrincipalMinor * dRate * dFactor / (dFactor - 1.0));
}

/* Runs the schedule. The last payment clears whatever balance remains.
   Returns true if the balance ends at exactly zero. */
static bool RunSchedule(long long llPrincipalMinor, long long llPaymentMinor, double dRate,
                        int iPayments, long long *pllTotalInterestMinor) {
    long long llBalance = llPrincipalMinor;
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

bool LoanCalculate(Loan *pLoan) {
    if (pLoan == NULL) return false;
    if (pLoan->llPrincipal <= 0 || pLoan->iTermMonths <= 0) return false;
    if (pLoan->iCurrencyDigits < 0 || pLoan->iCurrencyDigits > 6) return false;
    if (!isfinite(pLoan->dRate) || pLoan->dRate < 0.0) return false;

    const long long llScale = Pow10(pLoan->iCurrencyDigits);
    const double dMonthlyRate = pLoan->dRate / 12.0;
    const long long llPrincipalMinor = pLoan->llPrincipal * llScale;
    const long long llPaymentMinor =
        MonthlyPaymentMinor(llPrincipalMinor, dMonthlyRate, pLoan->iTermMonths);

    long long llTotalInterestMinor = 0;
    const bool bOk = RunSchedule(llPrincipalMinor, llPaymentMinor, dMonthlyRate,
                                 pLoan->iTermMonths, &llTotalInterestMinor);

    pLoan->llPayment       = ToWhole(llPaymentMinor, llScale);
    pLoan->llTotalInterest = ToWhole(llTotalInterestMinor, llScale);
    pLoan->llTotal         = pLoan->llPrincipal + pLoan->llTotalInterest;

    return bOk;
}