#include <stdio.h>
#include <stdlib.h>

#include "inc/MicroApps.h"
#include "inc/Menu.h"

enum {
    APP_MORTGAGE = 1,
    APP_INVESTMENT,
    APP_TRACKER
};

int main(int argc, const char** argv, char** envp) {
    if (argc > 1) {
        fprintf(stderr, "Arguments are disabled.\n");
        return EXIT_FAILURE;
    }

    MenuItem menu[] = {
        (MenuItem) {.id = APP_MORTGAGE, "Mortgage Calculator", 1, MENU_ITEM},
        (MenuItem) {.id = APP_INVESTMENT, "Investment Calculator", 1, MENU_ITEM},
        (MenuItem) {.id = APP_TRACKER, "Expense Tracker", 1, MENU_ITEM},
        (MenuItem) {.id = 0, "", 1, MENU_SEPARATOR},
        (MenuItem) {.id = 0, "Exit", 1, MENU_ITEM}
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
