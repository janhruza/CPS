//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_MORTGAGE_H
#define PLUTUS_MORTGAGE_H

typedef enum {
    DownPaymentLow,   /* 10% */
    DownPaymentHigh   /* 20% */
} DownPaymentType;

/* All money fields are in whole currency units (CZK, USD, EUR, ...). */
typedef struct {
    /* Inputs */
    long long llHomeValue;
    int iCurrencyDigits;          /* ISO 4217 minor-unit exponent: 2 for CZK, USD, EUR; 0 for JPY */
    DownPaymentType eDownPayment;
    double dInterest;             /* annual rate, 0.0525 = 5.25% */
    int iTermYears;

    /* Outputs, whole units */
    long long llDownPayment;
    long long llLoan;
    long long llMonthlyPayment;
    long long llTotalInterest;
    long long llTotalPaid;        /* llLoan + llTotalInterest */
} Mortgage;

bool MortgageValidate(const Mortgage *pMortgage);
bool MortgageCalculate(Mortgage *pMortgage);

#endif