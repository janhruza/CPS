#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

#include "inc/MicroApps.h"
#include "inc/Menu.h"
#include "inc/Core.h"
#include "inc/Engine/Investment.h"
#include "inc/Font.h"

enum {
    APP_EXIT,
    APP_MORTGAGE,
    APP_INVESTMENT,
    APP_TRACKER,
    APP_INVESTMENT_CHART
};

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

static void RunInvestmentChartTest(void) {
    puts("\n========================================");
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
        return;
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
        return;
    }

    // Allocate progression data
    double *pContributed = calloc((size_t)termMonths + 1, sizeof(double));
    double *pBalance = calloc((size_t)termMonths + 1, sizeof(double));
    if (!pContributed || !pBalance) {
        fprintf(stderr, "Memory allocation error.\n");
        free(pContributed);
        free(pBalance);
        return;
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
        return;
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
        return;
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
        return;
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
        DrawText(renderer, padL, 20, "PLUTUS - INVESTMENT GROWTH CHART", cText, 2);

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
}

int main(int argc, const char** argv, char** envp) {
    (void)envp;

    if (argc > 1) {
        if (strcmp(argv[1], "--chart") == 0 || strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "chart") == 0) {
            RunInvestmentChartTest();
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Unknown argument '%s'. Use --chart to launch investment chart test directly.\n", argv[1]);
        return EXIT_FAILURE;
    }

    const MenuItem menu[] = {
        (MenuItem) {.id = APP_MORTGAGE, .label = "Mortgage Calculator", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT, .label = "Investment Calculator", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT_CHART, .label = "Investment Chart (SDL Test)", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_TRACKER, .label = "Expense Tracker", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) {.id = APP_EXIT, .label = "Exit", .selectable = 1, .type = MENU_ITEM}
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
                RunInvestmentChartTest();
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
