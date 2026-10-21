//
// Created by jan on 10/21/26.
//

#ifndef PLUTUS_CORE_H
#define PLUTUS_CORE_H

#include "../inc/Core.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE 64

/* Reads one line into pszBuffer. Exits if input is closed. */
static void ReadLine(char *pszBuffer, size_t uSize) {
    if (fgets(pszBuffer, (int)uSize, stdin) == NULL) {
        exit(EXIT_FAILURE);
    }
}

/* True if only whitespace follows position p. */
static bool OnlySpaceAfter(const char *p) {
    for (; *p != '\0'; p++) {
        if (!isspace((unsigned char)*p)) return false;
    }
    return true;
}

/* True if the line has a minus sign before the first digit (unsigned input rejects it). */
static bool HasMinus(const char *p) {
    while (isspace((unsigned char)*p)) p++;
    return *p == '-';
}

int ReadInt(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        long lValue = strtol(szLine, &pEnd, 10);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) &&
            errno == 0 && lValue >= INT_MIN && lValue <= INT_MAX) {
            return (int)lValue;
        }
        printf("Invalid input, try again: ");
    }
}

uint32_t ReadUInt32(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        unsigned long long uValue = strtoull(szLine, &pEnd, 10);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) && !HasMinus(szLine) &&
            errno == 0 && uValue <= UINT32_MAX) {
            return (uint32_t)uValue;
        }
        printf("Invalid input, try again: ");
    }
}

uint64_t ReadUInt64(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        unsigned long long uValue = strtoull(szLine, &pEnd, 10);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) && !HasMinus(szLine) &&
            errno == 0) {
            return (uint64_t)uValue;
        }
        printf("Invalid input, try again: ");
    }
}

long long ReadLongLong(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        long long llValue = strtoll(szLine, &pEnd, 10);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) && errno == 0) {
            return llValue;
        }
        printf("Invalid input, try again: ");
    }
}

float ReadFloat(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        float fValue = strtof(szLine, &pEnd);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) && errno == 0) {
            return fValue;
        }
        printf("Invalid input, try again: ");
    }
}

double ReadDouble(void) {
    char szLine[INPUT_SIZE];
    for (;;) {
        ReadLine(szLine, sizeof szLine);
        char *pEnd;
        errno = 0;
        double dValue = strtod(szLine, &pEnd);
        if (pEnd != szLine && OnlySpaceAfter(pEnd) && errno == 0) {
            return dValue;
        }
        printf("Invalid input, try again: ");
    }
}

void WaitForEnter(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

void Pause(const char *msg) {
    printf("%s", msg);
    WaitForEnter();
}

#endif //PLUTUS_CORE_H
