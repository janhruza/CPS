//
// Created by jan on 10/17/26.
//

#include <math.h>
#include "../inc/Mortgage.h"

bool Mortgage_Validate(const Mortgage* pMortgage) {
    if (pMortgage == nullptr) return false;

    if (pMortgage->uDownPaymentType < 0) return false;
    if (pMortgage->uHomeValue <= 0) return false;
    if (pMortgage->fInterest < 0.0f) return false;
    if (pMortgage->uLoanTerm <= 0) return false;

    return true;
}

bool Mortgage_Calculate(Mortgage* pMortgage) {
    if (pMortgage == nullptr) return false;

    // Validate input values
    if (pMortgage->uLoanTerm == 0) return false;
    if (pMortgage->fInterest < 0.0f) return false;
    if (pMortgage->uHomeValue == 0) return false;

    // Calculate down payment and loan amount
    float fDown = 0.0f;
    float fLoan = 1.0f;

    if (pMortgage->uDownPaymentType == DOWN_PAYMENT_LOW) {
        fDown = 0.1f;
        fLoan = 0.9f;
    }
    else if (pMortgage->uDownPaymentType == DOWN_PAYMENT_HIGH) {
        fDown = 0.2f;
        fLoan = 0.8f;
    }
    else {
        return false;
    }

    // Calculate principal (loan amount) and down payment as floats
    float fPrincipal = fLoan * (float)pMortgage->uHomeValue;
    float fDownPayment = fDown * (float)pMortgage->uHomeValue;

    // Assign rounded values to the struct
    pMortgage->uDownPayment = (unsigned int)roundf(fDownPayment);
    pMortgage->uLoanAmount = (unsigned int)roundf(fPrincipal);

    // Calculate monthly interest and number of payments
    float fMonthlyInterest = pMortgage->fInterest / 12.0f;
    int nPayments = pMortgage->uLoanTerm * 12;

    // Calculate monthly payment, total payment, and total interest (using float for precision)
    float fMonthly = fPrincipal * ((fMonthlyInterest * powf(1.0f + fMonthlyInterest, nPayments)) / (powf(1.0f + fMonthlyInterest, nPayments) - 1.0f));
    float fTotal = fMonthly * (float)nPayments;
    float fTotalInterest = fTotal - fPrincipal;

    // Assign rounded values to the struct
    pMortgage->uMonthlyPayment = (unsigned int)roundf(fMonthly);
    pMortgage->uTotal = (unsigned int)roundf(fTotal);
    pMortgage->uTotalInterest = (unsigned int)roundf(fTotalInterest);

    return true;
}