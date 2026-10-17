#include <stdio.h>
#include <stdlib.h>

#include "inc/Engine/Mortgage.h"
#include "inc/Engine/Transaction.h"

static bool MortgageCalculator(void) {
    Mortgage mortgage = {0};

    puts("--- MORTGAGE CALCULATOR ---");
    printf("Home value: ");
    scanf("%llu", &mortgage.llHomeValue);

    int age = 0;
    printf("Age: ");
    scanf("%d", &age);
    mortgage.eDownPayment = age < 36 ? DownPaymentLow : DownPaymentHigh;

    double interest = 0;
    printf("Interest: ");
    scanf("%lf", &interest);
    mortgage.sLoan.dRate = interest;

    int term = 0;
    printf("Term (years): ");
    scanf("%d", &term);
    mortgage.sLoan.iTermMonths = term * 12;

    if (!MortgageCalculate(&mortgage)) {
        fprintf(stderr, "Unable to calculate mortgage\n");
        return false;
    }

    printf("\n--- RESULT ---\n");
    printf("Loan value:      %llu\n", mortgage.sLoan.llPrincipal);
    printf("Down payment:    %llu\n", mortgage.llDownPayment);
    printf("Total:           %llu\n", mortgage.sLoan.llTotal);
    printf("Monthly payment: %llu\n", mortgage.sLoan.llPayment);
    printf("Total interest:  %llu\n", mortgage.sLoan.llTotalInterest);

    return true;
}

int main(int argc, const char** argv, char** envp) {
    if (argc > 1) {
        fprintf(stderr, "Arguments are disabled.\n");
        return EXIT_FAILURE;
    }

    return MortgageCalculator() == true ? EXIT_SUCCESS : EXIT_FAILURE;
}
