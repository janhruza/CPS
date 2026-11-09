#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/command.h"
#include "inc/commands.h"

#define prompt "proxsh: "

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
        for (size_t i = 0; i < MAX_COMMANDS; i++) {
            if (commands[i].name == nullptr) break;
            if (strcmp(ptr, commands[i].name) == 0) {
                // match found
                ptr = strtok(nullptr, "\0"); // rest of the args
                int errorCode = 0;
                commands[i].action((void*)ptr, &errorCode);

                // check the exit code, ignore success (0)
                if (errorCode > 0) {
                    const char* msg = GetErrorMsg(errorCode);
                    printf("%s\n", msg);
                }

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

#undef prompt