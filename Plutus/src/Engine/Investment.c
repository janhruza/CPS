//
// Created by jan on 10/23/26.
//

#include <math.h>
#include "../../inc/Engine/Investment.h"

bool InvestmentCalculate(Investment *pInvestment) {
    if (pInvestment == nullptr) return false;
    if (pInvestment->llInitial < 0 || pInvestment->llContribution < 0 || pInvestment->iTermMonths <= 0) return false;
    if (!isfinite(pInvestment->dRate) || pInvestment->dRate < 0.0) return false;

    const double monthlyReturn = pInvestment->dRate / 12.0;
    const double growth = pow(1.0 + monthlyReturn, pInvestment->iTermMonths);

    double contributionFactor;
    if (monthlyReturn == 0.0) {
        contributionFactor = pInvestment->iTermMonths;
    } else {
        contributionFactor = (growth - 1.0) / monthlyReturn;
    }

    const double fv = pInvestment->llInitial * growth
                    + pInvestment->llContribution * contributionFactor;

    pInvestment->llFutureValue = llround(fv);
    pInvestment->llTotalContributed = pInvestment->llInitial
                                + pInvestment->llContribution * pInvestment->iTermMonths;
    pInvestment->llGain = pInvestment->llFutureValue - pInvestment->llTotalContributed;
    return true;
}