#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/MicroApps.h"
#include "inc/Menu.h"

enum {
    APP_EXIT,
    APP_MORTGAGE,
    APP_INVESTMENT,
    APP_TRACKER,
    APP_INVESTMENT_CHART,
    APP_MORTGAGE_CHART,
    APP_LOAN_COMPARISON,
    APP_EXPENSE_REPORT,
    APP_NET_WORTH,
    APP_SAVINGS_GOAL,
    APP_BUDGET_WHEEL,
    APP_LOAN_RACE,
    APP_INFLATION
};

int main(int argc, const char** argv, char** envp) {
    (void)envp;

    if (argc > 1) {
        if (strcmp(argv[1], "--chart") == 0 || strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "chart") == 0) {
            MAInvestmentChart();
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Unknown argument '%s'. Use --chart to launch investment chart test directly.\n", argv[1]);
        return EXIT_FAILURE;
    }

    const MenuItem menu[] = {
        (MenuItem) {.id = APP_MORTGAGE,        .label = "Mortgage Calculator",        .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT,      .label = "Investment Calculator",      .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_TRACKER,         .label = "Expense Tracker",            .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) {.id = -1, .label = "SDL Charts", .selectable = 0, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT_CHART,.label = "Investment Growth Chart",     .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_MORTGAGE_CHART,  .label = "Mortgage Amortisation Chart", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_LOAN_COMPARISON, .label = "Loan Comparison Chart",       .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_EXPENSE_REPORT,  .label = "Expense Report Chart",        .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_NET_WORTH,       .label = "Net Worth Timeline",          .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_SAVINGS_GOAL,    .label = "Savings Goal Accumulator",    .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_BUDGET_WHEEL,    .label = "Monthly Budget Wheel",        .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_LOAN_RACE,       .label = "Loan Accelerated Payoff Race",.selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INFLATION,       .label = "Inflation Erosion Chart",     .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) {.id = APP_EXIT,            .label = "Exit",                        .selectable = 1, .type = MENU_ITEM}
    };

    bool bExit = false;
    while (!bExit) {
        const int opt = MenuSelect("Main Menu", menu, sizeof(menu) / sizeof(MenuItem));
        switch (opt) {
            case APP_MORTGAGE:
                MAMortgageCalculator();
                break;

            case APP_INVESTMENT:
                MAInvestmentCalculator();
                break;

            case APP_INVESTMENT_CHART:
                MAInvestmentChart();
                break;

            case APP_MORTGAGE_CHART:
                MAMortgageChart();
                break;

            case APP_LOAN_COMPARISON:
                MALoanComparisonChart();
                break;

            case APP_EXPENSE_REPORT:
                MAExpenseReport();
                break;

            case APP_NET_WORTH:
                MANetWorthTimeline();
                break;

            case APP_SAVINGS_GOAL:
                MASavingsGoalChart();
                break;

            case APP_BUDGET_WHEEL:
                MABudgetWheel();
                break;

            case APP_LOAN_RACE:
                MALoanPayoffRace();
                break;

            case APP_INFLATION:
                MAInflationErosionChart();
                break;

            case APP_TRACKER:
                MAExpenseTracker();
                break;

            case APP_EXIT:
            case -1:
                bExit = true;
                break;

            default:
                break;
        }
    }

    return EXIT_SUCCESS;
}
