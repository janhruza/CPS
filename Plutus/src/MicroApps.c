//
// Created by jan on 10/18/26.
//

#include <stdio.h>
#include "../inc/Engine/Mortgage.h"
#include "../inc/Engine/ExpenseTracker.h"
#include "../inc/MicroApps.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../inc/Core.h"
#include "../inc/Menu.h"

bool MAMortgageCalculator(void) {
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

    Pause("Press enter to continue... ");
    return true;
}

static bool CreateTransaction(ExpenseTracker* pTracker, Transaction* pTr, const uint8_t ttype) {
    memset((Transaction*)pTr, 0, sizeof(Transaction));
    const uint64_t timeNow = time(nullptr);

    pTr->uId = timeNow;
    pTr->eType = ttype;
    pTr->uCreationTimeMs = timeNow;
    pTr->uDate = timeNow;

    // get value
    printf("Enter value for income: ");
    const double value = ReadDouble();

    pTr->dValue = value;

    if (!TransactionValidate(pTr)) {
        fprintf(stderr, "Transaction validation failed\n");
        Pause("Press enter to continue... ");
        return false;
    }

    if (ExpenseTrackerAdd(pTracker, pTr) == false) {
        fputs("Failed to add transaction to the tracker\n", stderr);
        Pause("Press enter to continue... ");
        return false;
    }

    return true;
}

bool MAExpenseTracker(void) {
    // simple, in-memory, expense tracker

    enum {
        ID_DASH,
        ID_EXPENSE,
        ID_INCOME,
        ID_HISTORY,
        ID_EXIT
    };

#define SEPARATOR (MenuItem) {.id = 0, .label = "", .selectable = 0, .type = MENU_SEPARATOR}

    // initialize the tracker
    ExpenseTracker tracker = {0};
    Transaction transactions[64] = {0};
    ExpenseTrackerInit(&tracker, transactions, sizeof(transactions) / sizeof(Transaction));

    bool bExit = false;
    while (bExit == false) {
        MenuItem menus[] = {
            (MenuItem){.id = ID_DASH, .label = "Dashboard", .selectable = tracker.uCount > 0 ? 1 : 0, .type = MENU_ITEM},
            (MenuItem){.id = ID_HISTORY, .label = "History", .selectable = tracker.uCount > 0 ? 1 : 0, .type = MENU_ITEM},
            SEPARATOR,
            (MenuItem){.id = ID_INCOME, .label = "Add income", .selectable = 1, .type = MENU_ITEM},
            (MenuItem){.id = ID_EXPENSE, .label = "Add expense", .selectable = 1, .type = MENU_ITEM},
            SEPARATOR,
            (MenuItem){.id = ID_EXIT, .label = "Exit", .selectable = 1, .type = MENU_ITEM},
        };

        const int opt = MenuSelect("Expense Tracker", menus, sizeof(menus) / sizeof(MenuItem));
        if (opt == ID_EXIT) bExit = true;

        switch (opt) {
            case ID_DASH:
                // display dashboard
                const double expenses = ExpenseTrackerSum(&tracker, TRANSACTION_EXPENSE, 0, UINT32_MAX);
                const double incomes = ExpenseTrackerSum(&tracker, TRANSACTION_INCOME, 0, UINT32_MAX);
                const double total = incomes - expenses;
                printf("--- DASHBOARD ---\nIncomes (total): %.2lf\nExpenses (total): %.2lf\nBalance: %.2lf\n\n", incomes, expenses, total);

                Pause("Press enter to continue... ");
                break;

            case ID_HISTORY:
                printf("--- TRANSACTIONS ---\n");
                for (int i = 0; i < tracker.uCount; i++) {
                    Transaction tr = {0};
                    ExpenseTrackerGet(&tracker, i, &tr);
                    printf("%s, value: %.2lf\n", tr.eType == TRANSACTION_EXPENSE ? "Expense" : "Income", tr.dValue);
                }

                Pause("Press enter to continue... ");
                break;

            case ID_INCOME:
                Transaction income = {0};
                CreateTransaction(&tracker, &income, TRANSACTION_INCOME);
                break;

            case ID_EXPENSE:
                Transaction expense = {0};
                CreateTransaction(&tracker, &expense,TRANSACTION_EXPENSE);
                break;

            default: break;
        }
    }

#undef SEPARATOR

    return true;
}