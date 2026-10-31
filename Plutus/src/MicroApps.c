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


/* ============================================================
 *  Shared SDL helpers (used by all chart MicroApps below)
 * ============================================================ */

/* Open a window + renderer.  Returns false on failure (already prints error). */
static bool SDLOpen(const char *title, int w, int h,
                    SDL_Window **ppWin, SDL_Renderer **ppRen)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }
    *ppWin = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              w, h,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!*ppWin) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    *ppRen = SDL_CreateRenderer(*ppWin, -1,
                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*ppRen)
        *ppRen = SDL_CreateRenderer(*ppWin, -1, 0);
    if (!*ppRen) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*ppWin);
        SDL_Quit();
        return false;
    }
    SDL_SetRenderDrawBlendMode(*ppRen, SDL_BLENDMODE_BLEND);
    return true;
}

static void SDLClose(SDL_Renderer *ren, SDL_Window *win)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

/* Draw a filled circle using midpoint algorithm. */
static void DrawFilledCircle(SDL_Renderer *ren, int cx, int cy, int r,
                              SDL_Color col)
{
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)sqrt((double)(r * r - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

/* Draw an arc (pie slice) filled with col, from angle a0 to a1 (radians). */
static void DrawPieSlice(SDL_Renderer *ren, int cx, int cy, int r,
                         double a0, double a1, SDL_Color col)
{
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    const int steps = (int)(fabs(a1 - a0) / (2.0 * M_PI) * 720) + 2;
    for (int s = 0; s < steps; s++) {
        double a = a0 + (a1 - a0) * s / (double)steps;
        int ex = cx + (int)(r * cos(a));
        int ey = cy + (int)(r * sin(a));
        SDL_RenderDrawLine(ren, cx, cy, ex, ey);
    }
}

/* Standard event loop: returns false once ESC/Q/close pressed. */
static bool SDLEvents(int *pMouseX, int *pMouseY)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return false;
        if (e.type == SDL_KEYDOWN &&
            (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))
            return false;
        if (e.type == SDL_MOUSEMOTION) {
            *pMouseX = e.motion.x;
            *pMouseY = e.motion.y;
        }
    }
    return true;
}

/* ============================================================
 *  MAMortgageChart
 *  Stacked-area amortisation schedule + pie chart
 * ============================================================ */

bool MAMortgageChart(void)
{
    puts("\n========================================");
    puts("   MORTGAGE AMORTISATION CHART (SDL)");
    puts("========================================");

    printf("Loan principal ($): ");
    const long long llPrincipal = ReadLongLong();

    printf("Annual interest rate (e.g. 0.045 for 4.5%%): ");
    const double dRate = ReadDouble();

    printf("Term (years): ");
    const int iYears = ReadInt();

    if (llPrincipal <= 0 || dRate < 0.0 || iYears <= 0) {
        fputs("Invalid parameters.\n", stderr);
        Pause("Press enter to continue... ");
        return false;
    }

    Loan loan = {
        .llPrincipal    = llPrincipal,
        .iCurrencyDigits = 2,
        .dRate          = dRate,
        .iTermMonths    = iYears * 12
    };

    if (!LoanCalculate(&loan)) {
        fputs("Loan calculation failed.\n", stderr);
        Pause("Press enter to continue... ");
        return false;
    }

    const int N = loan.iTermMonths;
    double *pBalance   = calloc((size_t)(N + 1), sizeof(double));
    double *pCumPrinc  = calloc((size_t)(N + 1), sizeof(double));
    double *pCumInter  = calloc((size_t)(N + 1), sizeof(double));
    if (!pBalance || !pCumPrinc || !pCumInter) {
        fputs("Memory allocation error.\n", stderr);
        free(pBalance); free(pCumPrinc); free(pCumInter);
        return false;
    }

    /* Build month-by-month schedule */
    const double monthlyRate = dRate / 12.0;
    const long long llScale  = 100LL; /* 2 decimal digits */
    double balance = (double)(llPrincipal * llScale);
    const double payment = (double)(loan.llPayment * llScale);

    pBalance[0]  = (double)llPrincipal;
    pCumPrinc[0] = 0.0;
    pCumInter[0] = 0.0;

    for (int m = 1; m <= N; m++) {
        double interest  = balance * monthlyRate;
        double principal = payment - interest;
        if (m == N || principal > balance) principal = balance;
        balance -= principal;
        pBalance[m]  = balance / (double)llScale;
        pCumPrinc[m] = pCumPrinc[m-1] + principal / (double)llScale;
        pCumInter[m] = pCumInter[m-1] + interest  / (double)llScale;
    }

    printf("\n--- SUMMARY ---\n");
    printf("Principal:       $%lld\n", llPrincipal);
    printf("Monthly payment: $%lld\n", loan.llPayment);
    printf("Total interest:  $%lld\n", loan.llTotalInterest);
    printf("Total cost:      $%lld\n", loan.llTotal);
    printf("Interest share:  %.1f%%\n\n",
           100.0 * (double)loan.llTotalInterest / (double)loan.llTotal);

    puts("Opening SDL Chart window...");
    puts("(Hover to inspect  |  ESC / Q to close)");

    SDL_Window   *win = NULL;
    SDL_Renderer *ren = NULL;
    if (!SDLOpen("Plutus - Mortgage Amortisation", 1080, 640, &win, &ren)) {
        free(pBalance); free(pCumPrinc); free(pCumInter);
        Pause("Press enter to continue... ");
        return false;
    }

    /* Colors */
    const SDL_Color cBg      = {  15,  23,  42, 255 };
    const SDL_Color cPlotBg  = {  30,  41,  59, 255 };
    const SDL_Color cGrid    = {  71,  85, 105, 100 };
    const SDL_Color cText    = { 241, 245, 249, 255 };
    const SDL_Color cSub     = { 148, 163, 184, 255 };
    const SDL_Color cBorder  = {  51,  65,  85, 255 };
    const SDL_Color cBlue    = {  59, 130, 246, 255 };
    const SDL_Color cBlueFill= {  59, 130, 246,  35 };
    const SDL_Color cAmber   = { 251, 191,  36, 255 };
    const SDL_Color cAmberFl = { 251, 191,  36,  35 };
    const SDL_Color cGreen   = {  16, 185, 129, 255 };

    int mouseX = -1, mouseY = -1;
    bool bRun = true;

    while (bRun) {
        bRun = SDLEvents(&mouseX, &mouseY);

        int winW, winH;
        SDL_GetWindowSize(win, &winW, &winH);

        /* Layout: left area = schedule chart, right area = pie */
        const int padL = 90, padR = 20, padT = 80, padB = 55;
        const int pieSize = (winH - padT - padB);
        const int pieW = pieSize + 20;
        const int plotW = winW - padL - padR - pieW - 20;
        const int plotH = winH - padT - padB;
        const int pieX = padL + plotW + 30 + pieSize / 2;
        const int pieY = padT + plotH / 2;
        const int pieR = pieSize / 2 - 10;

        SDL_SetRenderDrawColor(ren, cBg.r, cBg.g, cBg.b, 255);
        SDL_RenderClear(ren);

        /* Title */
        DrawText(ren, padL, 18, "PLUTUS - MORTGAGE AMORTISATION SCHEDULE", cText, 2);

        char hdr[192];
        snprintf(hdr, sizeof(hdr),
                 "Principal: $%lld  |  Rate: %.2f%%  |  Term: %dy  |  Payment: $%lld/mo  |  Total Interest: $%lld",
                 llPrincipal, dRate * 100.0, iYears, loan.llPayment, loan.llTotalInterest);
        DrawText(ren, padL, 50, hdr, cSub, 1);

        if (plotW > 60 && plotH > 60) {
            /* Plot background */
            SDL_Rect rPlot = { padL, padT, plotW, plotH };
            SDL_SetRenderDrawColor(ren, cPlotBg.r, cPlotBg.g, cPlotBg.b, 255);
            SDL_RenderFillRect(ren, &rPlot);

            const double maxVal = (double)llPrincipal * 1.05;

            /* Grid lines */
            for (int i = 0; i <= 5; i++) {
                double v = maxVal * i / 5.0;
                int yg = padT + plotH - (int)llround((double)plotH * i / 5.0);
                SDL_SetRenderDrawColor(ren, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(ren, padL, yg, padL + plotW, yg);
                char lbl[32];
                FormatCurrency(v, lbl, sizeof(lbl));
                int lw = (int)strlen(lbl) * FONT_CHAR_W;
                DrawText(ren, padL - lw - 6, yg - FONT_CHAR_H / 2, lbl, cSub, 1);
            }
            int yearStep = 1;
            if (iYears > 20) yearStep = 5;
            else if (iYears > 10) yearStep = 2;
            for (int y = 0; y <= iYears; y += yearStep) {
                int xg = padL + (int)llround((double)plotW * y / iYears);
                SDL_SetRenderDrawColor(ren, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(ren, xg, padT, xg, padT + plotH);
                char lbl[16]; snprintf(lbl, sizeof(lbl), "Yr%d", y);
                int lw = (int)strlen(lbl) * FONT_CHAR_W;
                DrawText(ren, xg - lw / 2, padT + plotH + 6, lbl, cSub, 1);
            }

            /* Stacked fills and curves */
            int pX = padL;
            int pYBal  = padT + plotH - (int)llround(pBalance[0] / maxVal * plotH);
            int pYPrnc = padT + plotH - (int)llround(pCumPrinc[0] / maxVal * plotH);

            for (int m = 1; m <= N; m++) {
                int cX    = padL + (int)llround((double)m / N * plotW);
                int cYBal = padT + plotH - (int)llround(pBalance[m] / maxVal * plotH);
                int cYPrnc= padT + plotH - (int)llround(pCumPrinc[m] / maxVal * plotH);
                int bottom= padT + plotH;

                /* Cumulative interest fill (amber, above principal) */
                SDL_SetRenderDrawColor(ren, cAmberFl.r, cAmberFl.g, cAmberFl.b, cAmberFl.a);
                SDL_RenderDrawLine(ren, cX, cYPrnc, cX, bottom);

                /* Cumulative principal fill (blue) */
                SDL_SetRenderDrawColor(ren, cBlueFill.r, cBlueFill.g, cBlueFill.b, cBlueFill.a);
                SDL_RenderDrawLine(ren, cX, cYPrnc, cX, cYBal);

                /* Balance curve (green) */
                SDL_SetRenderDrawColor(ren, cGreen.r, cGreen.g, cGreen.b, 255);
                SDL_RenderDrawLine(ren, pX, pYBal, cX, cYBal);
                SDL_RenderDrawLine(ren, pX, pYBal - 1, cX, cYBal - 1);

                /* Principal curve (blue) */
                SDL_SetRenderDrawColor(ren, cBlue.r, cBlue.g, cBlue.b, 255);
                SDL_RenderDrawLine(ren, pX, pYPrnc, cX, cYPrnc);

                pX = cX; pYBal = cYBal; pYPrnc = cYPrnc;
            }

            SDL_SetRenderDrawColor(ren, cBorder.r, cBorder.g, cBorder.b, 255);
            SDL_RenderDrawRect(ren, &rPlot);

            /* Legend */
            int lx = padL + 10, ly = padT + 10;
            SDL_Rect lr1 = {lx, ly + 2, 14, 10};
            SDL_SetRenderDrawColor(ren, cGreen.r, cGreen.g, cGreen.b, 255);
            SDL_RenderFillRect(ren, &lr1);
            DrawText(ren, lx + 18, ly, "Remaining Balance", cSub, 1);

            SDL_Rect lr2 = {lx + 190, ly + 2, 14, 10};
            SDL_SetRenderDrawColor(ren, cBlue.r, cBlue.g, cBlue.b, 255);
            SDL_RenderFillRect(ren, &lr2);
            DrawText(ren, lx + 208, ly, "Cumul. Principal", cSub, 1);

            SDL_Rect lr3 = {lx + 380, ly + 2, 14, 10};
            SDL_SetRenderDrawColor(ren, cAmber.r, cAmber.g, cAmber.b, 255);
            SDL_RenderFillRect(ren, &lr3);
            DrawText(ren, lx + 398, ly, "Cumul. Interest", cSub, 1);

            /* Crosshair tooltip */
            if (mouseX >= padL && mouseX <= padL + plotW &&
                mouseY >= padT && mouseY <= padT + plotH) {
                int hM = (int)llround((double)(mouseX - padL) / plotW * N);
                if (hM < 0) hM = 0;
                if (hM > N) hM = N;
                int hX = padL + (int)llround((double)hM / N * plotW);
                SDL_SetRenderDrawColor(ren, 203, 213, 225, 130);
                SDL_RenderDrawLine(ren, hX, padT, hX, padT + plotH);

                char tt1[64], tt2[64], tt3[64], tt4[64];
                snprintf(tt1, sizeof(tt1), "Month %d  (Yr %d)", hM, hM / 12);
                snprintf(tt2, sizeof(tt2), "Balance:    $%.0f", pBalance[hM]);
                snprintf(tt3, sizeof(tt3), "Princ. paid:$%.0f", pCumPrinc[hM]);
                snprintf(tt4, sizeof(tt4), "Inter. paid:$%.0f", pCumInter[hM]);
                int tW = 205, tH = 75;
                int tX = hX + 12, tY = mouseY - 30;
                if (tX + tW > padL + plotW) tX = hX - tW - 12;
                if (tY < padT) tY = padT + 4;
                if (tY + tH > padT + plotH) tY = padT + plotH - tH - 4;
                SDL_Rect rTip = {tX, tY, tW, tH};
                SDL_SetRenderDrawColor(ren, 15, 23, 42, 230);
                SDL_RenderFillRect(ren, &rTip);
                SDL_SetRenderDrawColor(ren, 71, 85, 105, 255);
                SDL_RenderDrawRect(ren, &rTip);
                DrawText(ren, tX + 8, tY +  6, tt1, cText,  1);
                DrawText(ren, tX + 8, tY + 22, tt2, cGreen, 1);
                DrawText(ren, tX + 8, tY + 38, tt3, cBlue,  1);
                DrawText(ren, tX + 8, tY + 54, tt4, cAmber, 1);
            }
        }

        /* Pie chart */
        if (pieR > 20) {
            DrawText(ren, pieX - 30, padT, "Cost Split", cText, 1);

            double totalPrinc = (double)llPrincipal;
            double totalInter = (double)loan.llTotalInterest;
            double total = totalPrinc + totalInter;
            double a0 = -M_PI / 2.0;
            double a1 = a0 + 2.0 * M_PI * totalPrinc / total;
            double a2 = a0 + 2.0 * M_PI;

            DrawPieSlice(ren, pieX, pieY, pieR, a0, a1, cBlue);
            DrawPieSlice(ren, pieX, pieY, pieR, a1, a2, cAmber);

            /* White ring outline */
            SDL_SetRenderDrawColor(ren, 200, 210, 220, 60);
            for (int ang = 0; ang < 360; ang++) {
                double a = ang * M_PI / 180.0;
                SDL_RenderDrawPoint(ren, pieX + (int)(pieR * cos(a)),
                                         pieY + (int)(pieR * sin(a)));
            }

            /* Donut hole */
            DrawFilledCircle(ren, pieX, pieY, pieR / 3, cPlotBg);

            /* Centre label */
            char pct[16];
            snprintf(pct, sizeof(pct), "%.0f%% int",
                     100.0 * totalInter / total);
            int pw = (int)strlen(pct) * FONT_CHAR_W;
            DrawText(ren, pieX - pw / 2, pieY - FONT_CHAR_H / 2, pct, cAmber, 1);

            /* Pie legend */
            int plx = pieX - pieR, ply = padT + plotH + 10;
            SDL_Rect prl1 = {plx, ply + 1, 12, 9};
            SDL_SetRenderDrawColor(ren, cBlue.r, cBlue.g, cBlue.b, 255);
            SDL_RenderFillRect(ren, &prl1);
            DrawText(ren, plx + 16, ply, "Principal", cSub, 1);

            SDL_Rect prl2 = {plx + 90, ply + 1, 12, 9};
            SDL_SetRenderDrawColor(ren, cAmber.r, cAmber.g, cAmber.b, 255);
            SDL_RenderFillRect(ren, &prl2);
            DrawText(ren, plx + 106, ply, "Interest", cSub, 1);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDLClose(ren, win);
    free(pBalance); free(pCumPrinc); free(pCumInter);
    Pause("\nChart closed. Press enter to continue... ");
    return true;
}

/* ============================================================
 *  MALoanComparisonChart
 *  Bar chart comparing up to 4 loan scenarios
 * ============================================================ */

#define MAX_SCENARIOS 4

bool MALoanComparisonChart(void)
{
    puts("\n========================================");
    puts("   LOAN COMPARISON CHART (SDL)");
    puts("========================================");
    puts("Compare up to 4 loan scenarios (same principal, different rate/term).\n");

    printf("Loan principal ($): ");
    const long long llPrincipal = ReadLongLong();
    if (llPrincipal <= 0) {
        fputs("Invalid principal.\n", stderr);
        Pause("Press enter to continue... ");
        return false;
    }

    printf("Number of scenarios to compare (1-%d): ", MAX_SCENARIOS);
    int nScen = ReadInt();
    if (nScen < 1) nScen = 1;
    if (nScen > MAX_SCENARIOS) nScen = MAX_SCENARIOS;

    Loan loans[MAX_SCENARIOS];
    char labels[MAX_SCENARIOS][32];

    for (int s = 0; s < nScen; s++) {
        printf("\n--- Scenario %d ---\n", s + 1);
        printf("Annual interest rate (e.g. 0.045 for 4.5%%): ");
        const double rate = ReadDouble();
        printf("Term (years): ");
        const int years = ReadInt();

        loans[s] = (Loan){
            .llPrincipal     = llPrincipal,
            .iCurrencyDigits = 2,
            .dRate           = rate,
            .iTermMonths     = years * 12
        };
        snprintf(labels[s], sizeof(labels[s]), "%.2f%%/%dy", rate * 100.0, years);

        if (!LoanCalculate(&loans[s])) {
            fprintf(stderr, "Scenario %d calculation failed.\n", s + 1);
            Pause("Press enter to continue... ");
            return false;
        }
        printf("  Monthly payment: $%lld  |  Total cost: $%lld  |  Interest: $%lld\n",
               loans[s].llPayment, loans[s].llTotal, loans[s].llTotalInterest);
    }

    puts("\nOpening SDL Chart window...");
    puts("(Hover a bar to inspect  |  ESC / Q to close)");

    SDL_Window   *win = NULL;
    SDL_Renderer *ren = NULL;
    if (!SDLOpen("Plutus - Loan Comparison", 960, 600, &win, &ren)) {
        Pause("Press enter to continue... ");
        return false;
    }

    const SDL_Color cBg     = {  15,  23,  42, 255 };
    const SDL_Color cPlotBg = {  30,  41,  59, 255 };
    const SDL_Color cGrid   = {  71,  85, 105, 100 };
    const SDL_Color cText   = { 241, 245, 249, 255 };
    const SDL_Color cSub    = { 148, 163, 184, 255 };
    const SDL_Color cBorder = {  51,  65,  85, 255 };

    /* Four distinct bar colours */
    const SDL_Color barCols[MAX_SCENARIOS] = {
        {  59, 130, 246, 220 },  /* blue   */
        {  16, 185, 129, 220 },  /* green  */
        { 251, 191,  36, 220 },  /* amber  */
        { 239,  68,  68, 220 },  /* red    */
    };
    const SDL_Color interCols[MAX_SCENARIOS] = {
        {  59, 130, 246, 100 },
        {  16, 185, 129, 100 },
        { 251, 191,  36, 100 },
        { 239,  68,  68, 100 },
    };

    int mouseX = -1, mouseY = -1;
    bool bRun = true;

    while (bRun) {
        bRun = SDLEvents(&mouseX, &mouseY);

        int winW, winH;
        SDL_GetWindowSize(win, &winW, &winH);

        const int padL = 95, padR = 30, padT = 80, padB = 80;
        const int plotW = winW - padL - padR;
        const int plotH = winH - padT - padB;

        SDL_SetRenderDrawColor(ren, cBg.r, cBg.g, cBg.b, 255);
        SDL_RenderClear(ren);

        DrawText(ren, padL, 18, "PLUTUS - LOAN SCENARIO COMPARISON", cText, 2);

        char hdr[128];
        snprintf(hdr, sizeof(hdr), "Principal: $%lld  |  %d scenario%s",
                 llPrincipal, nScen, nScen > 1 ? "s" : "");
        DrawText(ren, padL, 50, hdr, cSub, 1);

        /* Find max total to scale bars */
        long long maxTotal = 0;
        for (int s = 0; s < nScen; s++)
            if (loans[s].llTotal > maxTotal) maxTotal = loans[s].llTotal;

        if (plotW > 60 && plotH > 60) {
            SDL_Rect rPlot = {padL, padT, plotW, plotH};
            SDL_SetRenderDrawColor(ren, cPlotBg.r, cPlotBg.g, cPlotBg.b, 255);
            SDL_RenderFillRect(ren, &rPlot);

            /* Horizontal grid */
            for (int i = 0; i <= 5; i++) {
                double v = (double)maxTotal * 1.1 * i / 5.0;
                int yg = padT + plotH - (int)llround((double)plotH * i / 5.0);
                SDL_SetRenderDrawColor(ren, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(ren, padL, yg, padL + plotW, yg);
                char lbl[32]; FormatCurrency(v, lbl, sizeof(lbl));
                int lw = (int)strlen(lbl) * FONT_CHAR_W;
                DrawText(ren, padL - lw - 6, yg - FONT_CHAR_H / 2, lbl, cSub, 1);
            }

            /* Bars */
            const int gap = plotW / (nScen * 3 + 1);
            const int barW = gap * 2;
            int hovered = -1;

            for (int s = 0; s < nScen; s++) {
                int bX = padL + gap + s * (barW + gap);

                /* Total bar (principal + interest, semi-transparent on top) */
                int totalH = (int)llround((double)plotH * (double)loans[s].llTotal
                                          / ((double)maxTotal * 1.1));
                int princH = (int)llround((double)plotH * (double)loans[s].llPrincipal
                                          / ((double)maxTotal * 1.1));

                /* Interest portion */
                SDL_Rect rInter = { bX, padT + plotH - totalH, barW, totalH - princH };
                SDL_SetRenderDrawColor(ren, interCols[s].r, interCols[s].g, interCols[s].b, interCols[s].a);
                SDL_RenderFillRect(ren, &rInter);

                /* Principal portion */
                SDL_Rect rPrinc = { bX, padT + plotH - princH, barW, princH };
                SDL_SetRenderDrawColor(ren, barCols[s].r, barCols[s].g, barCols[s].b, barCols[s].a);
                SDL_RenderFillRect(ren, &rPrinc);

                /* Bar outline */
                SDL_Rect rBar = { bX, padT + plotH - totalH, barW, totalH };
                SDL_SetRenderDrawColor(ren, barCols[s].r, barCols[s].g, barCols[s].b, 255);
                SDL_RenderDrawRect(ren, &rBar);

                /* X label */
                int lw = (int)strlen(labels[s]) * FONT_CHAR_W;
                DrawText(ren, bX + barW / 2 - lw / 2, padT + plotH + 8, labels[s], cSub, 1);

                /* Monthly payment label above bar */
                char payLbl[24];
                snprintf(payLbl, sizeof(payLbl), "$%lld/mo", loans[s].llPayment);
                int pw = (int)strlen(payLbl) * FONT_CHAR_W;
                DrawText(ren, bX + barW / 2 - pw / 2,
                         padT + plotH - totalH - FONT_CHAR_H - 4, payLbl,
                         barCols[s], 1);

                /* Hover detection */
                if (mouseX >= bX && mouseX <= bX + barW &&
                    mouseY >= padT + plotH - totalH && mouseY <= padT + plotH)
                    hovered = s;
            }

            SDL_SetRenderDrawColor(ren, cBorder.r, cBorder.g, cBorder.b, 255);
            SDL_RenderDrawRect(ren, &rPlot);

            /* Tooltip */
            if (hovered >= 0) {
                int s = hovered;
                int bX = padL + gap + s * (barW + gap);
                int totalH = (int)llround((double)plotH * (double)loans[s].llTotal
                                          / ((double)maxTotal * 1.1));
                int tX = bX + barW + 6;
                int tY = padT + plotH - totalH;
                if (tX + 220 > winW) tX = bX - 226;
                if (tY + 80 > padT + plotH) tY = padT + plotH - 80;
                SDL_Rect rTip = {tX, tY, 220, 80};
                SDL_SetRenderDrawColor(ren, 15, 23, 42, 235);
                SDL_RenderFillRect(ren, &rTip);
                SDL_SetRenderDrawColor(ren, 71, 85, 105, 255);
                SDL_RenderDrawRect(ren, &rTip);
                char tt[4][64];
                snprintf(tt[0], 64, "Scenario %d: %s", s + 1, labels[s]);
                snprintf(tt[1], 64, "Monthly:   $%lld", loans[s].llPayment);
                snprintf(tt[2], 64, "Interest:  $%lld", loans[s].llTotalInterest);
                snprintf(tt[3], 64, "Total:     $%lld", loans[s].llTotal);
                DrawText(ren, tX + 8, tY +  6, tt[0], cText,        1);
                DrawText(ren, tX + 8, tY + 22, tt[1], barCols[s],   1);
                DrawText(ren, tX + 8, tY + 38, tt[2], interCols[s], 1);
                DrawText(ren, tX + 8, tY + 54, tt[3], cText,        1);
            }

            /* Legend (bottom right) */
            int lx = padL + plotW - 220, ly = padT + plotH + 35;
            SDL_Rect ll1 = {lx, ly + 1, 12, 9};
            SDL_SetRenderDrawColor(ren, 200, 200, 200, 200);
            SDL_RenderFillRect(ren, &ll1);
            DrawText(ren, lx + 16, ly, "Principal", cSub, 1);
            SDL_Rect ll2 = {lx + 100, ly + 1, 12, 9};
            SDL_SetRenderDrawColor(ren, 150, 150, 150, 120);
            SDL_RenderFillRect(ren, &ll2);
            DrawText(ren, lx + 116, ly, "Interest", cSub, 1);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDLClose(ren, win);
    Pause("\nChart closed. Press enter to continue... ");
    return true;
}

/* ============================================================
 *  MAExpenseReport
 *  Input transactions in-session, then display a SDL bar
 *  chart with monthly totals and an income-vs-expense view.
 * ============================================================ */

bool MAExpenseReport(void)
{
    puts("\n========================================");
    puts("   EXPENSE REPORT CHART (SDL)");
    puts("========================================");
    puts("Enter transactions for the current month. Leave amount as 0 to stop.\n");

    /* Use a fixed-size buffer for simplicity */
    const int MAX_TR = 512;
    Transaction *trs = calloc((size_t)MAX_TR, sizeof(Transaction));
    if (!trs) {
        fputs("Memory error.\n", stderr);
        return false;
    }

    /* Category names (matches eCategory 0..7) */
    static const char *const CATS[] = {
        "Other", "Housing", "Food", "Transport",
        "Health", "Entertainment", "Savings", "Shopping"
    };
    const int NCAT = 8;

    int count = 0;
    uint64_t nextId = 1;

    while (count < MAX_TR) {
        printf("[%d] Type (1=Income, 2=Expense, 0=Done): ", count + 1);
        int ttype = ReadInt();
        if (ttype == 0) break;
        if (ttype != 1 && ttype != 2) { puts("  Invalid type."); continue; }

        printf("     Amount ($, >0): ");
        double val = ReadDouble();
        if (val <= 0.0) break;

        int cat = 0;
        if (ttype == 2) {
            printf("     Category: ");
            for (int i = 0; i < NCAT; i++) printf("%d=%s ", i, CATS[i]);
            printf("\n     Choice: ");
            cat = ReadInt();
            if (cat < 0 || cat >= NCAT) cat = 0;
        }

        trs[count] = (Transaction){
            .uId       = nextId++,
            .dValue    = val,
            .uDate     = 20261031, /* fixed demo date */
            .uType     = (uint8_t)(ttype == 1 ? TRANSACTION_INCOME : TRANSACTION_EXPENSE),
            .eCategory = (uint8_t)cat
        };
        count++;
    }

    if (count == 0) {
        puts("No transactions entered.");
        Pause("Press enter to continue... ");
        free(trs);
        return false;
    }

    /* Aggregate: total income, total expense, per-category expenses */
    double totalIncome = 0.0, totalExpense = 0.0;
    double catTotals[8] = {0};

    for (int i = 0; i < count; i++) {
        if (trs[i].uType == TRANSACTION_INCOME) {
            totalIncome += trs[i].dValue;
        } else {
            totalExpense += trs[i].dValue;
            catTotals[trs[i].eCategory] += trs[i].dValue;
        }
    }

    double balance = totalIncome - totalExpense;
    printf("\n--- SUMMARY (%d transactions) ---\n", count);
    printf("Total Income:   $%.2f\n", totalIncome);
    printf("Total Expenses: $%.2f\n", totalExpense);
    printf("Balance:        %s$%.2f\n\n", balance >= 0 ? "+" : "", balance);

    puts("Opening SDL Chart window...");
    puts("(Hover for details  |  ESC / Q to close)");

    SDL_Window   *win = NULL;
    SDL_Renderer *ren = NULL;
    if (!SDLOpen("Plutus - Expense Report", 1000, 640, &win, &ren)) {
        free(trs);
        Pause("Press enter to continue... ");
        return false;
    }

    const SDL_Color cBg      = {  15,  23,  42, 255 };
    const SDL_Color cPlotBg  = {  30,  41,  59, 255 };
    const SDL_Color cGrid    = {  71,  85, 105, 100 };
    const SDL_Color cText    = { 241, 245, 249, 255 };
    const SDL_Color cSub     = { 148, 163, 184, 255 };
    const SDL_Color cBorder  = {  51,  65,  85, 255 };
    const SDL_Color cGreen   = {  16, 185, 129, 255 };
    const SDL_Color cGreenFl = {  16, 185, 129, 180 };
    const SDL_Color cRed     = { 239,  68,  68, 255 };
    const SDL_Color cRedFl   = { 239,  68,  68, 180 };
    const SDL_Color cAmber   = { 251, 191,  36, 255 };

    /* Eight category colours */
    const SDL_Color catColors[8] = {
        { 148, 163, 184, 220 },  /* Other        */
        {  59, 130, 246, 220 },  /* Housing      */
        {  16, 185, 129, 220 },  /* Food         */
        { 251, 191,  36, 220 },  /* Transport    */
        { 239,  68,  68, 220 },  /* Health       */
        { 167, 139, 250, 220 },  /* Entertain.   */
        {  45, 212, 191, 220 },  /* Savings      */
        { 249, 115,  22, 220 },  /* Shopping     */
    };

    int mouseX = -1, mouseY = -1;
    bool bRun = true;

    while (bRun) {
        bRun = SDLEvents(&mouseX, &mouseY);

        int winW, winH;
        SDL_GetWindowSize(win, &winW, &winH);

        /* Layout: left = income/expense bars, right = category pie */
        const int padT = 80, padB = 55;
        const int padL = 85, padR = 20;
        const int rightW = 260;
        const int plotW  = winW - padL - padR - rightW - 20;
        const int plotH  = winH - padT - padB;

        SDL_SetRenderDrawColor(ren, cBg.r, cBg.g, cBg.b, 255);
        SDL_RenderClear(ren);

        DrawText(ren, padL, 18, "PLUTUS - EXPENSE REPORT", cText, 2);

        char hdr[128];
        snprintf(hdr, sizeof(hdr),
                 "Transactions: %d  |  Income: $%.0f  |  Expenses: $%.0f  |  Balance: %+.0f",
                 count, totalIncome, totalExpense, balance);
        DrawText(ren, padL, 50, hdr, balance >= 0 ? cGreen : cRed, 1);

        if (plotW > 60 && plotH > 60) {
            SDL_Rect rPlot = {padL, padT, plotW, plotH};
            SDL_SetRenderDrawColor(ren, cPlotBg.r, cPlotBg.g, cPlotBg.b, 255);
            SDL_RenderFillRect(ren, &rPlot);

            double maxVal = totalIncome > totalExpense ? totalIncome : totalExpense;
            maxVal *= 1.15;
            if (maxVal < 1.0) maxVal = 1.0;

            /* Grid */
            for (int i = 0; i <= 5; i++) {
                double v = maxVal * i / 5.0;
                int yg = padT + plotH - (int)llround((double)plotH * i / 5.0);
                SDL_SetRenderDrawColor(ren, cGrid.r, cGrid.g, cGrid.b, cGrid.a);
                SDL_RenderDrawLine(ren, padL, yg, padL + plotW, yg);
                char lbl[32]; FormatCurrency(v, lbl, sizeof(lbl));
                int lw = (int)strlen(lbl) * FONT_CHAR_W;
                DrawText(ren, padL - lw - 6, yg - FONT_CHAR_H / 2, lbl, cSub, 1);
            }

            /* Two big bars: Income | Expense */
            int barW = plotW / 5;
            int incX  = padL + plotW / 4 - barW / 2;
            int expX  = padL + 3 * plotW / 4 - barW / 2;

            int incH = (int)llround(totalIncome / maxVal * plotH);
            int expH = (int)llround(totalExpense / maxVal * plotH);

            SDL_Rect rInc = {incX, padT + plotH - incH, barW, incH};
            SDL_SetRenderDrawColor(ren, cGreenFl.r, cGreenFl.g, cGreenFl.b, cGreenFl.a);
            SDL_RenderFillRect(ren, &rInc);
            SDL_SetRenderDrawColor(ren, cGreen.r, cGreen.g, cGreen.b, 255);
            SDL_RenderDrawRect(ren, &rInc);

            SDL_Rect rExp = {expX, padT + plotH - expH, barW, expH};
            SDL_SetRenderDrawColor(ren, cRedFl.r, cRedFl.g, cRedFl.b, cRedFl.a);
            SDL_RenderFillRect(ren, &rExp);
            SDL_SetRenderDrawColor(ren, cRed.r, cRed.g, cRed.b, 255);
            SDL_RenderDrawRect(ren, &rExp);

            /* Bar value labels */
            char incLbl[32], expLbl[32];
            snprintf(incLbl, sizeof(incLbl), "$%.0f", totalIncome);
            snprintf(expLbl, sizeof(expLbl), "$%.0f", totalExpense);
            int ilw = (int)strlen(incLbl) * FONT_CHAR_W;
            int elw = (int)strlen(expLbl) * FONT_CHAR_W;
            DrawText(ren, incX + barW / 2 - ilw / 2,
                     padT + plotH - incH - FONT_CHAR_H - 4, incLbl, cGreen, 1);
            DrawText(ren, expX + barW / 2 - elw / 2,
                     padT + plotH - expH - FONT_CHAR_H - 4, expLbl, cRed, 1);

            /* X labels */
            int ilbl = (int)strlen("Income") * FONT_CHAR_W;
            int elbl = (int)strlen("Expense") * FONT_CHAR_W;
            DrawText(ren, incX + barW / 2 - ilbl / 2, padT + plotH + 8, "Income",  cSub, 1);
            DrawText(ren, expX + barW / 2 - elbl / 2, padT + plotH + 8, "Expense", cSub, 1);

            /* Balance line */
            if (totalIncome > 0.0 && totalExpense > 0.0) {
                int yBal = padT + plotH -
                           (int)llround((totalIncome < totalExpense ? totalIncome : totalExpense)
                                        / maxVal * plotH);
                SDL_SetRenderDrawColor(ren, cAmber.r, cAmber.g, cAmber.b, 180);
                for (int xx = padL; xx < padL + plotW; xx += 8)
                    SDL_RenderDrawLine(ren, xx, yBal, xx + 4, yBal);
                char blbl[32];
                snprintf(blbl, sizeof(blbl), "balance $%+.0f", balance);
                DrawText(ren, padL + plotW / 2 - (int)strlen(blbl) * FONT_CHAR_W / 2,
                         yBal - FONT_CHAR_H - 2, blbl, cAmber, 1);
            }

            SDL_SetRenderDrawColor(ren, cBorder.r, cBorder.g, cBorder.b, 255);
            SDL_RenderDrawRect(ren, &rPlot);
        }

        /* Category pie (right side) */
        {
            int pieX = padL + plotW + 20 + rightW / 2;
            int pieY = padT + (plotH - rightW * 2 / 3) / 2 + rightW / 3;
            int pieR = rightW / 2 - 10;

            DrawText(ren, padL + plotW + 20, padT, "By Category", cText, 1);

            if (totalExpense > 0.0 && pieR > 15) {
                double angle = -M_PI / 2.0;
                for (int c = 0; c < NCAT; c++) {
                    if (catTotals[c] <= 0.0) continue;
                    double span = 2.0 * M_PI * catTotals[c] / totalExpense;
                    DrawPieSlice(ren, pieX, pieY, pieR, angle, angle + span, catColors[c]);
                    angle += span;
                }
                DrawFilledCircle(ren, pieX, pieY, pieR / 3, cPlotBg);
            } else {
                DrawText(ren, padL + plotW + 25, padT + 20, "No expenses", cSub, 1);
            }

            /* Category legend */
            int lx = padL + plotW + 18;
            int ly = pieY + pieR + 15;
            for (int c = 0; c < NCAT; c++) {
                if (catTotals[c] <= 0.0) continue;
                SDL_Rect lr = {lx, ly + 1, 10, 8};
                SDL_SetRenderDrawColor(ren, catColors[c].r, catColors[c].g,
                                       catColors[c].b, catColors[c].a);
                SDL_RenderFillRect(ren, &lr);
                char ll[48];
                snprintf(ll, sizeof(ll), "%s $%.0f", CATS[c], catTotals[c]);
                DrawText(ren, lx + 14, ly, ll, cSub, 1);
                ly += FONT_CHAR_H + 4;
            }

            /* Mouse hover on pie - show tooltip */
            if (totalExpense > 0.0 && pieR > 15) {
                int dx = mouseX - pieX, dy = mouseY - pieY;
                int dist2 = dx * dx + dy * dy;
                if (dist2 <= pieR * pieR && dist2 > (pieR / 3) * (pieR / 3)) {
                    double a = atan2(dy, dx);
                    if (a < -M_PI / 2.0) a += 2.0 * M_PI;
                    double angle2 = -M_PI / 2.0;
                    for (int c = 0; c < NCAT; c++) {
                        if (catTotals[c] <= 0.0) continue;
                        double span = 2.0 * M_PI * catTotals[c] / totalExpense;
                        if (a >= angle2 && a < angle2 + span) {
                            char tt[64];
                            snprintf(tt, sizeof(tt), "%s: $%.0f (%.1f%%)",
                                     CATS[c], catTotals[c],
                                     100.0 * catTotals[c] / totalExpense);
                            int tw = (int)strlen(tt) * FONT_CHAR_W + 16;
                            SDL_Rect rTip = {mouseX + 10, mouseY - 12, tw, 22};
                            SDL_SetRenderDrawColor(ren, 15, 23, 42, 230);
                            SDL_RenderFillRect(ren, &rTip);
                            SDL_SetRenderDrawColor(ren, 71, 85, 105, 255);
                            SDL_RenderDrawRect(ren, &rTip);
                            DrawText(ren, mouseX + 18, mouseY - 6, tt, cText, 1);
                            break;
                        }
                        angle2 += span;
                    }
                }
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDLClose(ren, win);
    free(trs);
    Pause("\nChart closed. Press enter to continue... ");
    return true;
}
