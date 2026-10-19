#include <stdio.h>
#include <stdlib.h>

#include "inc/Menu.h"

#include "inc/MicroApps.h"

int main(int argc, const char** argv, char** envp) {
    if (argc > 1) {
        fprintf(stderr, "Arguments are disabled.\n");
        return EXIT_FAILURE;
    }

    // main goes here
    //return MAExpenseTracker() == true ? EXIT_SUCCESS : EXIT_FAILURE;

    MenuItem menu[] = {
        { 1, "New game",       1 },
        { 2, "Load game",      1 },
        { 3, "Options (soon)", 0 },
        MENU_SEPARATOR_ITEM,
        { 4, "Credits",        1 },
        MENU_SEPARATOR_ITEM,
        { 5, "Exit",           1 },
    };

    int choice = MENU_SELECT("Main Menu", menu);

    if (choice < 0)
        printf("Quit without selecting.\n");
    else
        printf("You selected item id %d.\n", choice);

    return 0;
}
