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
#include <inttypes.h>
#include "../inc/Core.h"
#include "../inc/Menu.h"
#include "../inc/Engine/Investment.h"
#include "../inc/Engine/PlutusFile.h"

bool MAMortgageCalculator(void) {
    Mortgage mortgage = {0};

    puts("--- MORTGAGE CALCULATOR ---");
    printf("Home value: ");
    mortgage.llHomeValue = ReadLongLong();

    int age = 0;
    printf("Age: ");
    age = ReadInt();
    mortgage.eDownPayment = age < 36 ? DOWN_PAY_LOW : DOWN_PAY_HIGH;

    double interest = 0;
    printf("Interest (e.g. 0.05 for 5%%): ");
    interest = ReadDouble();
    mortgage.sLoan.dRate = interest;

    int term = 0;
    printf("Term (years): ");
    term = ReadInt();
    mortgage.sLoan.iTermMonths = term * 12;
    mortgage.sLoan.iCurrencyDigits = 2;

    if (!MortgageCalculate(&mortgage)) {
        fprintf(stderr, "Unable to calculate mortgage\n");
        Pause("Press enter to continue... ");
        return false;
    }

    printf("\n--- RESULT ---\n");
    printf("Loan value:      %lld\n", mortgage.sLoan.llPrincipal);
    printf("Down payment:    %lld\n", mortgage.llDownPayment);
    printf("Total:           %lld\n", mortgage.sLoan.llTotal);
    printf("Monthly payment: %lld\n", mortgage.sLoan.llPayment);
    printf("Total interest:  %lld\n", mortgage.sLoan.llTotalInterest);
    puts("--- END OF MORTGAGE CALCULATOR ---\n");

    Pause("Press enter to continue... ");
    return true;
}

static bool CreateTransaction(ExpenseTracker* pTracker, Transaction* pTr, const uint8_t ttype) {
    memset((Transaction*)pTr, 0, sizeof(Transaction));

    // Generate unique ID based on existing tracker items and timestamp
    uint64_t maxId = 0;
    for (size_t i = 0; i < pTracker->uCount; i++) {
        if (pTracker->pItems[i].uId > maxId) {
            maxId = pTracker->pItems[i].uId;
        }
    }
    const time_t tNow = time(nullptr);
    uint64_t nextId = (uint64_t)tNow;
    if (nextId <= maxId) {
        nextId = maxId + 1;
    }

    pTr->uId = nextId;
    pTr->uType = ttype;

    // Standardize date as YYYYMMDD
    struct tm tmNow;
#ifdef _WIN32
    localtime_s(&tmNow, &tNow);
#else
    localtime_r(&tNow, &tmNow);
#endif
    pTr->uDate = (uint32_t)((tmNow.tm_year + 1900) * 10000 + (tmNow.tm_mon + 1) * 100 + tmNow.tm_mday);

    // Prompt with correct transaction type label
    const char *typeStr = (ttype == TRANSACTION_EXPENSE) ? "expense" : "income";
    printf("Enter value for %s: ", typeStr);
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

    printf("Added %s successfully (ID: %" PRIu64 ").\n", typeStr, pTr->uId);
    Pause("Press enter to continue... ");
    return true;
}

bool MAExpenseTracker(void) {
    // simple, in-memory, expense tracker

    enum {
        ID_DASH,
        ID_EXPENSE,
        ID_INCOME,
        ID_HISTORY,
        ID_DELETE,
        ID_IMPORT,
        ID_EXPORT,
        ID_EXIT
    };

#define SEPARATOR (MenuItem) {.id = 0, .label = "", .selectable = 0, .type = MENU_SEPARATOR}

    // file data
    PlutusFile* pFile = PlutusCreate();
    if (pFile == nullptr) {
        fputs("Failed to allocate file structure\n", stderr);
        return false;
    }
    PlutusInit(pFile);

    // initialize the tracker directly with pFile's storage
    ExpenseTracker tracker = {0};
    ExpenseTrackerInit(&tracker, pFile->transactions, PLUTUS_MAX_TRANSACTIONS);

    bool bExit = false;
    while (bExit == false) {
        const char *filename = "ExpenseTracker.plutus";
        const MenuItem menus[] = {
                (MenuItem){.id = ID_DASH, .label = "Dashboard", .selectable = tracker.uCount > 0 ? 1 : 0, .type = MENU_ITEM},
                (MenuItem){.id = ID_HISTORY, .label = "History", .selectable = tracker.uCount > 0 ? 1 : 0, .type = MENU_ITEM},
                SEPARATOR,
                (MenuItem){.id = ID_INCOME, .label = "Add income", .selectable = 1, .type = MENU_ITEM},
                (MenuItem){.id = ID_EXPENSE, .label = "Add expense", .selectable = 1, .type = MENU_ITEM},
                (MenuItem){.id = ID_DELETE, .label = "Delete transaction", .selectable = tracker.uCount > 0 ? 1 : 0, .type = MENU_ITEM},
                SEPARATOR,
                (MenuItem){.id = ID_IMPORT, .label = "Import", .selectable = 1, .type = MENU_ITEM},
                (MenuItem){.id = ID_EXPORT, .label = "Export", .selectable = 1, .type = MENU_ITEM},
                SEPARATOR,
                (MenuItem){.id = ID_EXIT, .label = "Exit", .selectable = 1, .type = MENU_ITEM},
            };

        const int opt = MenuSelect("Expense Tracker", menus, sizeof(menus) / sizeof(MenuItem));
        if (opt == ID_EXIT || opt == -1) bExit = true;

        switch (opt) {
            case ID_DASH: {
                const double expenses = ExpenseTrackerSum(&tracker, TRANSACTION_EXPENSE, 0, UINT32_MAX);
                const double incomes = ExpenseTrackerSum(&tracker, TRANSACTION_INCOME, 0, UINT32_MAX);
                const double total = ExpenseTrackerBalance(&tracker);
                printf("--- DASHBOARD ---\nIncomes (total):  %.2lf\nExpenses (total): %.2lf\nBalance:          %.2lf\n\n",
                       incomes, expenses, total);

                Pause("Press enter to continue... ");
                break;
            }

            case ID_HISTORY: {
                printf("--- TRANSACTIONS (%zu) ---\n", tracker.uCount);
                for (size_t i = 0; i < tracker.uCount; i++) {
                    Transaction tr = {0};
                    ExpenseTrackerGet(&tracker, i, &tr);
                    printf("[%zu] ID: %" PRIu64 " | Date: %08" PRIu32 " | %-7s | Value: %.2lf\n",
                           i + 1, tr.uId, tr.uDate, tr.uType == TRANSACTION_EXPENSE ? "Expense" : "Income", tr.dValue);
                }

                Pause("Press enter to continue... ");
                break;
            }

            case ID_INCOME: {
                Transaction income = {0};
                CreateTransaction(&tracker, &income, TRANSACTION_INCOME);
                break;
            }

            case ID_EXPENSE: {
                Transaction expense = {0};
                CreateTransaction(&tracker, &expense, TRANSACTION_EXPENSE);
                break;
            }

            case ID_DELETE: {
                printf("Enter Transaction ID to delete: ");
                const uint64_t delId = ReadUInt64();
                if (ExpenseTrackerRemove(&tracker, delId)) {
                    printf("Transaction %" PRIu64 " removed successfully.\n", delId);
                } else {
                    printf("Transaction %" PRIu64 " not found.\n", delId);
                }
                Pause("Press enter to continue... ");
                break;
            }

            case ID_IMPORT:
                if (PlutusLoad(filename, pFile) == false) {
                    fputs("Failed to load plutus file\n", stderr);
                    Pause("Press enter to continue... ");
                    break;
                }

                tracker.uCount = pFile->uNextPos;
                printf("Successfully imported %zu transaction(s) from %s.\n", tracker.uCount, filename);
                Pause("Press enter to continue... ");
                break;

            case ID_EXPORT:
                pFile->uNextPos = tracker.uCount;
                if (!PlutusSave(filename, pFile)) {
                    fputs("Failed to save plutus file\n", stderr);
                    Pause("Press enter to continue... ");
                    break;
                }
                printf("Successfully exported %zu transaction(s) to %s.\n", tracker.uCount, filename);
                Pause("Press enter to continue... ");
                break;

            default: break;
        }
    }

#undef SEPARATOR

    PlutusDestroy(pFile);
    return true;
}

bool MAInvestmentCalculator(void) {
    Investment inv = {0};
    bool bReturn = true;

    puts("--- INVESTMENT CALCULATOR ---");

    // initial value
    printf("Initial value: ");
    inv.llInitial = ReadLongLong();

    printf("Term (years): ");
    inv.iTermMonths = ReadInt() * 12;

    printf("Monthly contribution: ");
    inv.llContribution = ReadLongLong();

    printf("Annual rate (e. g. 0.025 = 2.5 %%): ");
    inv.dRate = ReadDouble();

    if (InvestmentCalculate(&inv) == false) {
        fputs("Unable to calculate the investment.", stderr);
        bReturn = false;
    }

    else {
        puts("\nRESULTS");
        printf("Future value: %lld\nTotal contribution: %lld\nGain: %lld\n", inv.llFutureValue, inv.llTotalContributed, inv.llGain);
    }

    puts("--- END OF INVESTMENT CALCULATOR ---");

    Pause("\nPress enter to continue... ");

    return bReturn;
}