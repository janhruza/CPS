#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/command.h"

#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))
#define prompt "proxsh: "

static void ComClear(command_extra) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

static void ComExit(command_extra) {
    exit(0);
}

void *memory[1024];
static int mem_counter = 0;
static constexpr size_t mem_size = 1024 * 25;

static void CmdAlloc(command_extra) {
    if (mem_counter >= 1024) {
        printf("Memory allocation limit has been reached.\n");
        return;
    }

    void *mem = malloc(mem_size);
    memory[mem_counter] = mem;
    memset(mem, 0, mem_size);
    mem_counter++;
    printf("Alloc: %zu bytes allocated at %p \n", mem_size, mem);
}

void ComFreeAll(command_extra) {
    int count = 0;
    for (int i = 0; i < ARRAY_LEN(memory); i++) {
        if (memory[i] != nullptr) {
            free(memory[i]);
            count++;
        }
    }

    printf("FreeAll: Freed %d/%d allocations (%zu bytes)\n", count, mem_counter, count * mem_size);
}

static Command knownCommands[] = {
    {"clear", ComClear},
    {"exit", ComExit},
    { "alloc", CmdAlloc},
    { "free", ComFreeAll}
};

int main(int argc, char** argv) {
    if (argc != 1) {
        fprintf(stderr, "%s: Arguments are disabled.\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("ProxyShell\n(c) 2026, @jendahruza\n\n");
    fflush(stdout);

    bool bExit = false;
    while (!bExit) {
        printf("%s", prompt);

        // read the command
        char input[256], input_copy[256];
        fgets(input, 256, stdin);
        input[strlen(input) - 1] = '\0';

        // duplicate the string
        strncpy(input_copy, input, ARRAY_LEN(input));

        // handle the command
        const char* ptr = strtok(input_copy, " ");
        if (ptr == nullptr) continue;

        bool handled = false;
        for (size_t i = 0; i < ARRAY_LEN(knownCommands); i++) {
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
