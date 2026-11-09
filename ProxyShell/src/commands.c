//
// Created by jan on 11/8/26.
//

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>

#include "../inc/types.h"

#include "../inc/commands.h"

Command commands[MAX_COMMANDS] = {
    {"clear", CmdClear},
    {"exit", CmdExit},
    {"cat", CmdCat},
    {"ls", CmdListDir},
    {"time", CmdTimeNow},
    {"date", CmdDate},
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
    printf("\033[H\033[2J");
    com_set_out(0);
    fflush(stdout);
}

void CmdExit(com_extra) {
    UNUSED(pIn);
    exit(0);
    com_set_out(0);
}

void CmdListDir(com_extra) {
    const char *path = (char*)pIn;
    if (path == nullptr) {
        path = "."; // use current directory if none was specified
    }

    com_set_out(S_OK);

    DIR* dir = opendir(path);
    if (dir == nullptr) {
        com_set_out(E_OPEN);
        return;
    }

    struct dirent* entry;
    int count = 1;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        printf("%-3d%s\n", count++, entry->d_name);
    }

    if (closedir(dir) == -1) {
        com_set_out(E_CLOSE);
    }
}

void CmdTimeNow(com_extra) {
    UNUSED(pIn);
    com_set_out(S_OK);

    time_t rawtime;
    time(&rawtime);

    const struct tm* now = localtime(&rawtime);
    printf("%02d:%02d:%02d\n", now->tm_hour, now->tm_min, now->tm_sec);
}

void CmdDate(com_extra) {
    UNUSED(pIn);
    com_set_out(S_OK);

    time_t rawtime;
    time(&rawtime);

    const struct tm* now = localtime(&rawtime);
    printf("%02d/%02d/%02d\n", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
}