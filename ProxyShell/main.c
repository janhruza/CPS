#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/command.h"
#define prompt "proxsh: "

static void ComClear(com_extra) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

static void ComExit(com_extra) {
    exit(0);
}

static Command knownCommands[] = {
    {"clear", ComClear},
    {"exit", ComExit}
};

int main(const int argc, const char** argv, const char **envp) {
    if (argc != 1) {
        fprintf(stderr, "%s: Arguments are disabled.\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf(BANNER);
    fflush(stdout);

    for (;;) {
        printf("%s", prompt);

        // read the command
        char input[256], input_copy[256];
        fgets(input, 256, stdin);
        input[strlen(input) - 1] = '\0';

        // duplicate the string
        strncpy(input_copy, input, arr_size(input));

        // handle the command
        const char* ptr = strtok(input_copy, " ");
        if (ptr == nullptr) continue;

        bool handled = false;
        for (size_t i = 0; i < arr_size(knownCommands); i++) {
            if (strcmp(ptr, knownCommands[i].name) == 0) {
                // match found
                ptr = strtok(nullptr, "\0"); // rest of the args
                knownCommands[i].action(nullptr, nullptr);
                handled = true;
                break;
            }
        }

        if (handled) {
            continue;
        }

        // no match, pass it to the real shell
        system(input);
    }

    // program ends here
    return EXIT_SUCCESS;
}
