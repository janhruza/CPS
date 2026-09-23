#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/messages.h"
#include "inc/Reactor.h"
#include "inc/Version.h"

#define COMMAND_LEN     255

#define ANSI_CLEAR     "\033[2J\033[H"

/**
 * Representing a custom enum solely for the HandleCommand method.
 */
typedef enum tagHCResult {
    HCOK,       // Command handled successfully.
    HCEMPTY,    // Command is empty.
    HCEXIT,     // Exit command received.
    HCERR,      // A generic error, command not found, etc.
} HCRESULT;

/**
 * Handles a user entered input.
 * @param command Normalized user input.
 * @return Operation result.
 */
static HCRESULT HandleCommand(const char* command) {
    if (strlen(command) == 0) {
        return HCOK;
    }

    if (strcmp(command, "exit") == 0) {
        exit(EXIT_SUCCESS);
        return HCEXIT;
    }

    else if (strcmp(command, "clear") == 0) {
        printf(ANSI_CLEAR);
        return HCOK;
    }

    else if (strcmp(command, "state") == 0) {
        return HCOK;
    }

    // fallback case
    return HCERR;
}

/**
 * Main application method.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Program's exit code.
 */
int main(const int argc, const char* argv[]) {

    // command line args check
    if (argc != 1) {
        fprintf(stderr, MSG_ARGS_DISABLED);
        return EXIT_FAILURE;
    }

    Version version;
    version.Major = 1;
    version.Minor = 0;


    printf(MSG_BANNER);
    printf(FMT_MSG_VERSION, version.Major, version.Minor);
    puts("");

    printf("Initializing 4 reactor cores\n");

    const int total = 4;
    Reactor cores[total];
    int success = 0;
    for (unsigned int x = 0; x < 4; x++) {
        if (Reactor_Init(&cores[x]) == false) {
            fprintf(stderr, MSG_REACTOR_INIT_FAILED);
        }

        else {
            success++;
            cores[x].Id = (unsigned int)x+1;
            printf(FMT_MSG_REACTOR_INIT_OK, cores[x].Id);
        }
    }

    printf("Initialized %d/%d reactor cores\n", success, total);

    printf("\n");

    for (;;) {
        printf(MSG_PROMPT);

        // read the user input
        char command[COMMAND_LEN] = {0};
        fgets(command, COMMAND_LEN, stdin);
        command[strcspn(command, "\n")] = 0;

        // create a copy of the command
        char commandCopy[COMMAND_LEN] = {0};
        strcpy(commandCopy, command);

        // normalize the string to all lowercase
        for (int x = 0; x < strlen(commandCopy); x++) {
            commandCopy[x] = (char)tolower(commandCopy[x]);
        }

        // run the command normalized command
        if (HandleCommand(commandCopy) != HCOK) {
            fprintf(stderr, MSG_COMMAND_NOT_FOUND);
        }
    }

    return EXIT_SUCCESS;
}
