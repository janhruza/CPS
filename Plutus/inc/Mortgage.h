//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_MORTGAGE_H
#define PLUTUS_MORTGAGE_H

enum {

    /**
     * Representing the low down payment of 10%.
     */
    DOWN_PAYMENT_LOW,

    /**
     * Representing the high down payment of 20%.
     */
    DOWN_PAYMENT_HIGH

};

/**
 * Representing a simple mortgage calculator.
 */
typedef struct tagMortgage {
    /**
     * Total value of the new home.
     */
    unsigned int uHomeValue;

    /**
     * Down payment. Low 10% for the young and high 20% for those older than 36 years.
     * This field is calculated automatically.
     */
    unsigned int uDownPayment;

    /**
     * Average interest rate.
     */
    float fInterest;

    /**
     * The amount of money a bank will provide.
     * This field is calculated automatically.
     */
    unsigned int uLoanAmount;

    /**
     * Down payment type.
     * Possible values: DOWN_PAYMENT_LOW or DOWN_PAYMENT_HIGH.
     */
    unsigned int uDownPaymentType;

    /**
     * Representing the mortgage total value a client will pay over time.
     * This value is internally calculated as float/double but
     * rounded up to the nearest integer for convenience.
     * This field is calculated automatically.
     */
    unsigned int uTotal;

    /**
     * Representing the mortgage term (in years).
     */
    unsigned int uLoanTerm;

    /**
     * Representing the total interest paid.
     * This field is calculated automatically.
     */
    unsigned int uTotalInterest;

    /**
     * Representing the monthly payment.
     * This value is internally calculated as float but rounded up to the nearest integer for convenience.
     * This field is calculated automatically.
     */
    unsigned int uMonthlyPayment;

} Mortgage;

/**
 * Validates the mortgage parameters.
 * @param pMortgage Pointer to the target mortgage struct.
 * @return true if the mortgage can be calculated, otherwise false.
 */
bool Mortgage_Validate(const Mortgage *pMortgage);

/**
 * Calculates the mortgage.
 * @param pMortgage Pointer to the target mortgage struct.
 * @return true if the mortgage was calculated, otherwise false.
 */
bool Mortgage_Calculate(Mortgage *pMortgage);

#endif //PLUTUS_MORTGAGE_H
