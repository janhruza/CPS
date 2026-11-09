//
// Created by jan on 11/8/26.
//

#include <stdlib.h>
#include <stdio.h>

#include "../inc/types.h"

#include "../inc/commands.h"

Command commands[MAX_COMMANDS] = {
    {"clear", CmdClear},
    {"exit", CmdExit},
    {"cat", CmdCat}
};

void CmdCat(com_extra) {
    // pIn: [In] path
    // pOut: [Out] status code
    const char *path = (char *)pIn;
    com_set_out(S_OK);

    if (path == nullptr) {
        com_set_out(E_NULLPTR);
        return;
    }

    FILE *fp = fopen(path, "r");
    if (fp == nullptr) {
        com_set_out(E_OPEN);
        return;
    }

    int line = 1;
    int c;
    bool atLineStart = true;
    while ((c = fgetc(fp)) != EOF) {
        if (atLineStart) {
            fprintf(stdout, "%-4d ", line++);
            atLineStart = false;
        }
        fputc(c, stdout);
        if (c == '\n')
            atLineStart = true;
    }

    if (fclose(fp) != 0) {
        com_set_out(E_CLOSE);
    }
}

void CmdTouch(com_extra) {
    const char* path = (char*)pIn;
    com_set_out(S_OK);

    FILE *fp = fopen(path, "wb");
    if (fp == nullptr) {
        com_set_out(E_OPEN);
        return;
    }

    if (fclose(fp) != 0) {
        com_set_out(E_CLOSE);
        return;
    }

    // successfully created
    printf("touch: File '%s' created successfully!\n", path);
}

void CmdClear(com_extra) {
    UNUSED(pIn);
    UNUSED(pOut);
    printf("\033[H\033[2J");
    com_set_out(0);
    fflush(stdout);
}

void CmdExit(com_extra) {
    UNUSED(pIn);
    UNUSED(pOut);
    exit(0);
    com_set_out(0);
}