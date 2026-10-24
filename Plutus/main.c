#include <stdio.h>
#include <stdlib.h>

#include "inc/MicroApps.h"
#include "inc/Menu.h"

enum {
    APP_EXIT,
    APP_MORTGAGE,
    APP_INVESTMENT,
    APP_TRACKER
};

int main(int argc, const char** argv, char** envp) {
    if (argc > 1) {
        fprintf(stderr, "Arguments are disabled.\n");
        return EXIT_FAILURE;
    }

    const MenuItem menu[] = {
        (MenuItem) {.id = APP_MORTGAGE, .label = "Mortgage Calculator", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT, .label = "Investment Calculator", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) {.id = APP_TRACKER, .label = "Expense Tracker", .selectable = 1, .type = MENU_ITEM},
        (MenuItem) MENU_SEPARATOR_ITEM,
        (MenuItem) {.id = APP_EXIT, .label = "Exit", .selectable = 1, .type = MENU_ITEM}
    };

    bool bExit = false;
    int lastResult = 0;
    while (!bExit) {
        const int opt = MenuSelect("Main Menu", menu, sizeof(menu) / sizeof(MenuItem));
        switch (opt) {
            case APP_MORTGAGE:
                lastResult = MAMortgageCalculator();
                break;

            case APP_INVESTMENT:
                lastResult = MAInvestmentCalculator();
                break;

            case APP_TRACKER:
                lastResult = MAExpenseTracker();
                break;

            case 0:
            case -1:
                bExit = true;
                break;

                default: break;
        }
    }

    return (bool)lastResult == true ? EXIT_SUCCESS : EXIT_FAILURE;

    // main goes here
    //return MAExpenseTracker() == true ? EXIT_SUCCESS : EXIT_FAILURE;
}
