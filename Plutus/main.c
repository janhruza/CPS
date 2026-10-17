#include <stdio.h>
#include <stdlib.h>

#include "inc/Mortgage.h"

int main(int argc, const char** argv, char** envp) {

    printf("Mortgage calculator\n");

    Mortgage mortgage = {0};

    mortgage.uHomeValue = 4000000;
    mortgage.uLoanTerm = 30;
    mortgage.uDownPaymentType = DOWN_PAYMENT_LOW;
    mortgage.fInterest = 0.05f;

    if (Mortgage_Validate(&mortgage) == false) {
        fprintf(stderr, "Mortgage validation failed\n");
        return EXIT_FAILURE;
    }

    if (Mortgage_Calculate(&mortgage) == false) {
        fprintf(stderr, "Mortgage calculation failed\n");
        return EXIT_FAILURE;
    }

    printf("HV: %u\n", mortgage.uHomeValue);
    printf("Interest: %.2f\n", mortgage.fInterest);
    printf("DPT: %s, term: %d years\n\n", mortgage.uDownPaymentType == DOWN_PAYMENT_LOW ? "LOW" : "HIGH", mortgage.uLoanTerm);

    printf("Loan value: %u\n", mortgage.uLoanAmount);
    printf("Down payment: %u\n", mortgage.uDownPayment);
    printf("Total payment: %u\n", mortgage.uTotal);
    return EXIT_SUCCESS;
}
