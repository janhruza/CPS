#include <stdio.h>
#include <stdlib.h>

#include "inc/Mortgage.h"

static int TestMortgageMain(void) {
    Mortgage sMortgage = {
        .llHomeValue  = 5000000,
        .eDownPayment = DownPaymentLow,
        .sLoan = {
            .iCurrencyDigits = 2,
            .dRate           = 0.0525,
            .iTermMonths     = 360
        }
    };

    if (!MortgageCalculate(&sMortgage)) {
        printf("FAIL: MortgageCalculate returned false\n");
        return 1;
    }

    printf("Home value:     %lld\n", sMortgage.llHomeValue);
    printf("Down payment:   %lld\n", sMortgage.llDownPayment);
    printf("Loan:           %lld\n", sMortgage.sLoan.llPrincipal);
    printf("Monthly:        %lld\n", sMortgage.sLoan.llPayment);
    printf("Total interest: %lld\n", sMortgage.sLoan.llTotalInterest);
    printf("Total paid:     %lld\n", sMortgage.sLoan.llTotal);

    if (sMortgage.llDownPayment + sMortgage.sLoan.llPrincipal != sMortgage.llHomeValue) {
        printf("FAIL: down payment + loan != home value\n");
        return 1;
    }
    if (sMortgage.sLoan.llTotal != sMortgage.sLoan.llPrincipal + sMortgage.sLoan.llTotalInterest) {
        printf("FAIL: total != principal + interest\n");
        return 1;
    }

    printf("PASS\n");
    return 0;
}

int main(int argc, const char** argv, char** envp) {
    return TestMortgageMain();
}
