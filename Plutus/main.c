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
    return MAExpenseTracker() == true ? EXIT_SUCCESS : EXIT_FAILURE;
}
