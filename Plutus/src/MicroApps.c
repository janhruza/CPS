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
#include <math.h>
#include <SDL.h>
#include "../inc/Core.h"
#include "../inc/Menu.h"
#include "../inc/Engine/Investment.h"
#include "../inc/Engine/PlutusFile.h"
#include "../inc/Font.h"

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

static void FormatCurrency(double val, char *buf, size_t size) {
    if (val >= 1e9) {
        snprintf(buf, size, "$%.2lfB", val / 1e9);
    } else if (val >= 1e6) {
        snprintf(buf, size, "$%.2lfM", val / 1e6);
    } else if (val >= 1e4) {
        snprintf(buf, size, "$%.1lfk", val / 1e3);
    } else {
        snprintf(buf, size, "$%.0lf", val);
    }
}

bool MAInvestmentChart(void) {
    puts("========================================");
    puts("   INVESTMENT CHART TEST (SDL)");
    puts("========================================");

    printf("Initial investment amount ($): ");
    const long long llInitial = ReadLongLong();

    printf("Monthly contribution ($): ");
    const long long llContribution = ReadLongLong();

    printf("Annual return rate (e.g. 0.08 for 8%%): ");
    const double dRate = ReadDouble();

    printf("Investment term in years: ");
    const int iYears = ReadInt();

    if (llInitial < 0 || llContribution < 0 || dRate < 0.0 || iYears <= 0) {
        fprintf(stderr, "Invalid investment parameters.\n");
        Pause("Press enter to continue... ");
        return false;
    }

    const int termMonths = iYears * 12;
    Investment inv = {
        .llInitial = llInitial,
        .llContribution = llContribution,
        .dRate = dRate,
        .iTermMonths = termMonths
    };

    if (!InvestmentCalculate(&inv)) {
        fprintf(stderr, "Calculation failed.\n");
        Pause("Press enter to continue... ");
        return false;
    }

    // Allocate progression data
    double *pContributed = calloc((size_t)termMonths + 1, sizeof(double));
    double *pBalance = calloc((size_t)termMonths + 1, sizeof(double));
    if (!pContributed || !pBalance) {
        fprintf(stderr, "Memory allocation error.\n");
        free(pContributed);
        free(pBalance);
        return false;
    }

    const double monthlyRate = dRate / 12.0;
    for (int m = 0; m <= termMonths; m++) {
        pContributed[m] = (double)llInitial + (double)llContribution * m;
        if (monthlyRate == 0.0) {
            pBalance[m] = pContributed[m];
        } else {
            const double growth = pow(1.0 + monthlyRate, m);
            const double factor = (growth - 1.0) / monthlyRate;
            pBalance[m] = (double)llInitial * growth + (double)llContribution * factor;
        }
    }

    printf("\n--- SUMMARY ---\n");
    printf("Initial:            $%lld\n", llInitial);
    printf("Monthly:            $%lld\n", llContribution);
    printf("Annual Rate:        %.2lf%%\n", dRate * 100.0);
    printf("Term:               %d years (%d months)\n", iYears, termMonths);
    printf("Total Contributed:  $%lld\n", inv.llTotalContributed);
    printf("Future Value:       $%lld\n", inv.llFutureValue);
    printf("Interest Gain:      $%lld (+%.1lf%%)\n\n",
           inv.llGain,
           inv.llTotalContributed > 0 ? ((double)inv.llGain / (double)inv.llTotalContributed * 100.0) : 0.0);

    puts("Opening SDL Chart window...");
    puts("(Controls: Hover with mouse to inspect, press ESC or Q in the window to close)");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        Pause("Press enter to return to menu... ");
        free(pContributed);
        free(pBalance);
        return false;
    }

    const int defaultW = 960;
    const int defaultH = 620;
    SDL_Window *window = SDL_CreateWindow(
        "Plutus - Investment Growth Chart",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        defaultW, defaultH,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        free(pContributed);
        free(pBalance);
        return false;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        free(pContributed);
        free(pBalance);
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Color definitions
    const SDL_Color cBg       = { 15,  23,  42, 255 }; // Slate 900
    const SDL_Color cPlotBg   = { 30,  41,  59, 255 }; // Slate 800
    const SDL_Color cBorder   = { 51,  65,  85, 255 }; // Slate 700
    const SDL_Color cGrid     = { 71,  85, 105, 100 }; // Slate 600 subtle
    const SDL_Color cText     = { 241, 245, 249, 255 }; // Slate 100
    const SDL_Color cSubText  = { 148, 163, 184, 255 }; // Slate 400
    const SDL_Color cBlue     = { 59, 130, 246, 255 }; // Principal/Contributions
    const SDL_Color cBlueFill = { 59, 130, 246,  30 };
    const SDL_Color cGreen    = { 16, 185, 129, 255 }; // Total Balance
    const SDL_Color cGreenFill= { 16, 185, 129,  45 };

    const double maxVal = pBalance[termMonths] > 0 ? pBalance[termMonths] * 1.12 : 1000.0;

    int mouseX = -1, mouseY = -1;
    bool bRunning = true;
    SDL_Event e;

    while (bRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                bRunning = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q) {
                    bRunning = false;
                }
            } else if (e.type == SDL_MOUSEMOTION) {
                mouseX = e.motion.x;
                mouseY = e.motion.y;
            }
        }

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        // Layout margins
        const int padL = 95;
        const int padR = 40;
        const int padT = 90;
        const int padB = 60;
        const int plotW = winW - padL - padR;
        const int plotH = winH - padT - padB;

        // Clear window
        SDL_SetRenderDrawColor(renderer, cBg.r, cBg.g, cBg.b, cBg.a);
        SDL_RenderClear(renderer);

        // Header Title & Legend
        DrawText(renderer, padL, 20, "INVESTMENT GROWTH CHART", cText, 2);

        char statsBuf[256];
        snprintf(statsBuf, sizeof(statsBuf),
                 "Initial: $%lld  Monthly: $%lld  Rate: %.1lf%%  Term: %dy  |  Future Value: $%lld  Gain: +$%lld",
                 llInitial, llContribution, dRate * 100.0, iYears, inv.llFutureValue, inv.llGain);
        DrawText(renderer, padL, 52, statsBuf, cSubText, 1);

        // Legend
        const int legendX = winW - padR - 380;
        const int legendY = 22;
        SDL_Rect rL1 = { legendX, legendY + 2, 14, 10 };
        SDL_SetRenderDrawColor(renderer, cBlue.r, cBlue.g, cBlue.b, 255);
        SDL_RenderFillRect(renderer, &rL1);
        DrawText(renderer, legendX + 20, legendY, "Total Contributed", cSubText, 1);

        SDL_Rect rL2 = { legendX + 175, legendY + 2, 14, 10 };
        SDL_SetRenderDrawColor(renderer, cGreen.r, cGreen.g, cGreen.b, 255);
        SDL_RenderFillRect(renderer, &rL2);
        DrawText(renderer, legendX + 195, legendY, "Total Balance (Compound)", cSubText, 1);

        if (plotW > 50 && plotH > 50) {
            // Plot background
            SDL_Rect rPlot = { padL, padT, plotW, plotH };
            SDL_SetRenderDrawColor(renderer, cPlotBg.r, cPlotBg.g, cPlotBg.b, 255);
            SDL_RenderFillRect(renderer, &rPlot);

            // Horizontal grid lines & Y labels
            const int numYDivs = 5;
            for (int i = 0; i <= numYDivs; i++) {
                const double v = maxVal * (double)i / (double)numYDivs;
                const int yPos = padT + plotH - (int)llround((double)plotH * (double)i / (double)numYDivs);

                SDL_SetRenderDrawColor(renderer, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(renderer, padL, yPos, padL + plotW, yPos);

                char yStr[32];
                FormatCurrency(v, yStr, sizeof(yStr));
                const int labelW = (int)strlen(yStr) * FONT_CHAR_W;
                DrawText(renderer, padL - labelW - 10, yPos - FONT_CHAR_H / 2, yStr, cSubText, 1);
            }

            // Vertical grid lines & X labels (years)
            int yearStep = 1;
            if (iYears > 40) yearStep = 10;
            else if (iYears > 20) yearStep = 5;
            else if (iYears > 10) yearStep = 2;

            for (int y = 0; y <= iYears; y += yearStep) {
                const int xPos = padL + (int)llround((double)plotW * (double)y / (double)iYears);

                SDL_SetRenderDrawColor(renderer, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(renderer, xPos, padT, xPos, padT + plotH);

                char xStr[32];
                snprintf(xStr, sizeof(xStr), "Yr %d", y);
                const int labelW = (int)strlen(xStr) * FONT_CHAR_W;
                DrawText(renderer, xPos - labelW / 2, padT + plotH + 8, xStr, cSubText, 1);
            }

            // Shaded Areas and Curves
            int prevX = padL;
            int prevYCont = padT + plotH - (int)llround(pContributed[0] / maxVal * plotH);
            int prevYBal  = padT + plotH - (int)llround(pBalance[0] / maxVal * plotH);

            for (int m = 1; m <= termMonths; m++) {
                const int curX = padL + (int)llround((double)m / (double)termMonths * plotW);
                const int curYCont = padT + plotH - (int)llround(pContributed[m] / maxVal * plotH);
                const int curYBal  = padT + plotH - (int)llround(pBalance[m] / maxVal * plotH);

                // Shaded compound growth area (between contributed and balance)
                SDL_SetRenderDrawColor(renderer, cGreenFill.r, cGreenFill.g, cGreenFill.b, cGreenFill.a);
                SDL_RenderDrawLine(renderer, curX, curYBal, curX, curYCont);

                // Shaded principal area (below contributed)
                SDL_SetRenderDrawColor(renderer, cBlueFill.r, cBlueFill.g, cBlueFill.b, cBlueFill.a);
                SDL_RenderDrawLine(renderer, curX, curYCont, curX, padT + plotH);

                // Contributed curve (blue)
                SDL_SetRenderDrawColor(renderer, cBlue.r, cBlue.g, cBlue.b, 255);
                SDL_RenderDrawLine(renderer, prevX, prevYCont, curX, curYCont);

                // Balance curve (emerald green - 2px thick)
                SDL_SetRenderDrawColor(renderer, cGreen.r, cGreen.g, cGreen.b, 255);
                SDL_RenderDrawLine(renderer, prevX, prevYBal, curX, curYBal);
                SDL_RenderDrawLine(renderer, prevX, prevYBal - 1, curX, curYBal - 1);

                prevX = curX;
                prevYCont = curYCont;
                prevYBal = curYBal;
            }

            // Plot border
            SDL_SetRenderDrawColor(renderer, cBorder.r, cBorder.g, cBorder.b, 255);
            SDL_RenderDrawRect(renderer, &rPlot);

            // Interactive Crosshair & Tooltip
            if (mouseX >= padL && mouseX <= padL + plotW &&
                mouseY >= padT && mouseY <= padT + plotH) {
                int hMonth = (int)llround((double)(mouseX - padL) / (double)plotW * termMonths);
                if (hMonth < 0) hMonth = 0;
                if (hMonth > termMonths) hMonth = termMonths;

                const int hX = padL + (int)llround((double)hMonth / (double)termMonths * plotW);
                const int hYCont = padT + plotH - (int)llround(pContributed[hMonth] / maxVal * plotH);
                const int hYBal  = padT + plotH - (int)llround(pBalance[hMonth] / maxVal * plotH);

                // Guide line
                SDL_SetRenderDrawColor(renderer, 203, 213, 225, 140);
                SDL_RenderDrawLine(renderer, hX, padT, hX, padT + plotH);

                // Highlight dots
                SDL_Rect rDotC = { hX - 4, hYCont - 4, 9, 9 };
                SDL_SetRenderDrawColor(renderer, cBlue.r, cBlue.g, cBlue.b, 255);
                SDL_RenderFillRect(renderer, &rDotC);

                SDL_Rect rDotB = { hX - 4, hYBal - 4, 9, 9 };
                SDL_SetRenderDrawColor(renderer, cGreen.r, cGreen.g, cGreen.b, 255);
                SDL_RenderFillRect(renderer, &rDotB);

                // Tooltip box
                const double cVal = pContributed[hMonth];
                const double bVal = pBalance[hMonth];
                const double gVal = bVal - cVal;

                char t1[64], t2[64], t3[64], t4[64];
                snprintf(t1, sizeof(t1), "Time: Year %d, Mo %d", hMonth / 12, hMonth % 12);
                snprintf(t2, sizeof(t2), "Balance:     $%.0lf", bVal);
                snprintf(t3, sizeof(t3), "Contributed: $%.0lf", cVal);
                snprintf(t4, sizeof(t4), "Gain:       +$%.0lf", gVal);

                int tipW = 210;
                int tipH = 75;
                int tipX = hX + 15;
                int tipY = mouseY - 30;
                if (tipX + tipW > winW - padR) {
                    tipX = hX - tipW - 15;
                }
                if (tipY + tipH > winH - padB) {
                    tipY = winH - padB - tipH;
                }
                if (tipY < padT) {
                    tipY = padT + 10;
                }

                SDL_Rect rTip = { tipX, tipY, tipW, tipH };
                SDL_SetRenderDrawColor(renderer, 15, 23, 42, 235);
                SDL_RenderFillRect(renderer, &rTip);
                SDL_SetRenderDrawColor(renderer, 71, 85, 105, 255);
                SDL_RenderDrawRect(renderer, &rTip);

                DrawText(renderer, tipX + 10, tipY +  8, t1, cText, 1);
                DrawText(renderer, tipX + 10, tipY + 24, t2, cGreen, 1);
                DrawText(renderer, tipX + 10, tipY + 40, t3, cBlue, 1);
                DrawText(renderer, tipX + 10, tipY + 56, t4, cSubText, 1);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(pContributed);
    free(pBalance);

    Pause("\nChart window closed. Press enter to continue... ");
    return true;
}