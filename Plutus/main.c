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

static void PrintUsage(const char *progName) {
    printf("Plutus - Personal Finance Terminal & SDL Suite\n");
    printf("Usage:\n");
    printf("  %s [option]\n\n", progName);
    printf("Interactive SDL Charts (pops up parameter window, then displays chart):\n");
    printf("  --investment,     -i,   investment       Investment growth & compound interest\n");
    printf("  --mortgage,       -m,   mortgage         Mortgage amortisation schedule & equity\n");
    printf("  --loan-compare,   -lc,  loan-compare     Side-by-side loan scenarios comparison\n");
    printf("  --expense-report, -er,  expense-report   Monthly categorized expense breakdown\n");
    printf("  --net-worth,      -nw,  net-worth        Long-term net worth & wealth projection\n");
    printf("  --savings-goal,   -sg,  savings-goal     Savings goal accumulator timeline\n");
    printf("  --budget-wheel,   -bw,  budget-wheel     Monthly budget allocation donut wheel\n");
    printf("  --loan-race,      -lr,  loan-race        Accelerated loan payoff extra payment race\n");
    printf("  --inflation,      -inf, inflation        Purchasing power erosion over time\n\n");
    printf("Command Line Utilities (TUI):\n");
    printf("  --mortgage-calc,  -mc                    Console mortgage calculator\n");
    printf("  --investment-calc,-ic                    Console investment calculator\n");
    printf("  --tracker,        -t                     Console expense tracker\n\n");
    printf("General Options:\n");
    printf("  --help,           -h,   help             Show this help message\n");
    printf("  --chart,          -c    <name>           Launch chart by name\n");
    printf("  (no arguments)                           Launch interactive console menu\n");
}

static bool MatchArg(const char *arg, const char *longOpt, const char *shortOpt, const char *subcmd) {
    if (longOpt && strcmp(arg, longOpt) == 0) return true;
    if (shortOpt && strcmp(arg, shortOpt) == 0) return true;
    if (subcmd && strcmp(arg, subcmd) == 0) return true;
    return false;
}

static int RunAppByArg(const char *arg, const char *progName) {
    if (MatchArg(arg, "--help", "-h", "help")) {
        PrintUsage(progName);
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--investment", "-i", "investment") || strcmp(arg, "chart") == 0) {
        MAInvestmentChart();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--mortgage", "-m", "mortgage")) {
        MAMortgageChart();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--loan-compare", "-lc", "loan-compare") || strcmp(arg, "compare") == 0) {
        MALoanComparisonChart();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--expense-report", "-er", "expense-report") || strcmp(arg, "expense") == 0) {
        MAExpenseReport();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--net-worth", "-nw", "net-worth") || strcmp(arg, "wealth") == 0) {
        MANetWorthTimeline();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--savings-goal", "-sg", "savings-goal") || strcmp(arg, "goal") == 0) {
        MASavingsGoalChart();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--budget-wheel", "-bw", "budget-wheel") || strcmp(arg, "budget") == 0) {
        MABudgetWheel();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--loan-race", "-lr", "loan-race") || strcmp(arg, "race") == 0) {
        MALoanPayoffRace();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--inflation", "-inf", "inflation")) {
        MAInflationErosionChart();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--mortgage-calc", "-mc", "mortgage-calc")) {
        MAMortgageCalculator();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--investment-calc", "-ic", "investment-calc")) {
        MAInvestmentCalculator();
        return EXIT_SUCCESS;
    }
    if (MatchArg(arg, "--tracker", "-t", "tracker")) {
        MAExpenseTracker();
        return EXIT_SUCCESS;
    }

    fprintf(stderr, "Unknown argument '%s'. Use --help to list available options.\n", arg);
    return EXIT_FAILURE;
}

int main(int argc, const char** argv, char** envp) {
    (void)envp;

    if (argc > 1) {
        const char *arg = argv[1];
        if ((strcmp(arg, "--chart") == 0 || strcmp(arg, "-c") == 0) && argc > 2) {
            arg = argv[2];
        }
        return RunAppByArg(arg, argv[0]);
    }

    const MenuItem menu[] = {
        (MenuItem) MENU_HEADING_ITEM("Command Line Utilities"),
        (MenuItem) {.id = APP_MORTGAGE,        .label = "Mortgage Calculator",          .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT,      .label = "Investment Calculator",        .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_TRACKER,         .label = "Expense Tracker",              .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) MENU_HEADING_ITEM("SDL Charts"),
        (MenuItem) {.id = APP_INVESTMENT_CHART,.label = "Investment Growth Chart",      .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_MORTGAGE_CHART,  .label = "Mortgage Amortisation Chart",  .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_LOAN_COMPARISON, .label = "Loan Comparison Chart",        .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_EXPENSE_REPORT,  .label = "Expense Report Chart",         .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_NET_WORTH,       .label = "Net Worth Timeline",           .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_SAVINGS_GOAL,    .label = "Savings Goal Accumulator",     .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_BUDGET_WHEEL,    .label = "Monthly Budget Wheel",         .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_LOAN_RACE,       .label = "Loan Accelerated Payoff Race", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INFLATION,       .label = "Inflation Erosion Chart",      .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) {.id = APP_EXIT,            .label = "Exit",                         .selectable = 1, .type = MENU_ITEM}
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
