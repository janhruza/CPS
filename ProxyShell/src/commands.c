//
// Created by jan on 11/8/26.
//

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <string.h>

#include "../inc/types.h"

#include "../inc/commands.h"

Command commands[MAX_COMMANDS] = {
    {.name = "clear", .action = CmdClear},
    {.name = "exit", .action = CmdExit},
    {.name = "cat", .action = CmdCat},
    {.name = "ls", .action = CmdListDir},
    {.name = "time", .action = CmdTimeNow},
    {.name = "date", .action = CmdDate},
    {.name = "open", .action = CmdOpen},
};

/* --- HELPERS --- */

static bool GetLocalTime(struct tm* buf) {
    if (buf == nullptr) {
        return false;
    }

    time_t raw_time = 0;
    time(&raw_time);
    buf = localtime_r(&raw_time, buf);
    return true;
}

/* --------------- */

void CmdCat(com_args) {
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

void CmdTouch(com_args) {
    const char* path = (char*)pIn;
    com_set_out(S_OK);

    FILE *fp = fopen(path, "wb");
    if (fp == nullptr) {
        com_set_out(E_OPEN);
        return;
    }

    if (fclose(fp) != 0) {
        com_set_out(E_CLOSE);
    }
}

void CmdClear(com_args) {
    UNUSED(pIn);

    // clear the visible screen + the history (if supported)
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);

    com_set_out(S_OK);
}

void CmdExit(com_args) {
    UNUSED(pIn);
    exit(0);
    com_set_out(0);
}

void CmdListDir(com_args) {
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

void CmdTimeNow(com_args) {
    UNUSED(pIn);
    com_set_out(S_OK);

    struct tm now = {0};
    GetLocalTime(&now);
    printf("%02d:%02d:%02d\n", now.tm_hour, now.tm_min, now.tm_sec);
}

void CmdDate(com_args) {
    UNUSED(pIn);
    com_set_out(S_OK);

    struct tm now = {0};
    GetLocalTime(&now);
    printf("%02d/%02d/%02d\n", now.tm_mday, now.tm_mon + 1, now.tm_year + 1900);
}

void CmdOpen(com_args)
{
    const size_t len = strlen((char*)pIn);
    com_set_out(S_OK);

    // xdg-open [command]
    const size_t size = len + 10;
    char* mem = malloc(size * sizeof(char));
    snprintf(mem, size, "%s %s", "xdg-open", (char*)pIn);
    system(mem);
    free(mem);

    return;
}
