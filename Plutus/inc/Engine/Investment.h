//
// Created by jan on 10/23/26.
//

#ifndef PLUTUS_INVESTMENT_H
#define PLUTUS_INVESTMENT_H

typedef struct tagInvestment {
    /* Inputs, set by the caller before InvestmentCalculate */

    /* Amount invested at the start, in whole currency units. Zero is allowed (no starting capital). */
    long long llInitial;

    /* Amount added at the end of each month, in whole currency units. Zero is allowed (lump sum only). */
    long long llContribution;

    /* Expected annual return as a fraction: 0.0525 means 5.25%. Compounded monthly as dRate / 12. */
    double dRate;

    /* Number of months the money is invested. Must be > 0. */
    int iTermMonths;

    /* Balance at the end of the term, including all gains. */
    long long llFutureValue;

    /* llInitial + llContribution * iTermMonths: the money the investor put in, excluding any return. */
    long long llTotalContributed;

    /* llFutureValue - llTotalContributed: the return earned. */
    long long llGain;
} Investment;

bool InvestmentCalculate(Investment *pInvestment);

#endif //PLUTUS_INVESTMENT_H
