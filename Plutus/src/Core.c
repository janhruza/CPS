//
// Created by jan on 10/21/26.
//

#include "../inc/Core.h"
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

bool InitProgressBar(ProgressBar *pBar) {
    if (pBar == nullptr) return false;
    pBar->cFill = '#';
    pBar->cBlank = '.';
    pBar->bInitialized = true;
    return true;
}

bool DrawProgress(const ProgressBar *pBar) {
    if (pBar == nullptr) return false;
    if (pBar->dMin >= pBar->dMax) return false;
    if (pBar->uWidth == 0) return false;
    if (!pBar->bInitialized) return false;

    if (pBar->pszLabel != nullptr) {
        printf("%s [", pBar->pszLabel);
    } else {
        printf("[");
    }

    // clamp value to [dMin, dMax]
    double dVal = pBar->dValue;
    if (dVal < pBar->dMin) dVal = pBar->dMin;
    if (dVal > pBar->dMax) dVal = pBar->dMax;

    // draw fraction
    // example: min 10, max 20, width 20, value 17.6 => 7.6 / 10 => 76%
    const double dFraction = (dVal - pBar->dMin) / (pBar->dMax - pBar->dMin);
    int iFilled = (int)llround(dFraction * pBar->uWidth);
    if (iFilled < 0) iFilled = 0;
    if (iFilled > pBar->uWidth) iFilled = pBar->uWidth;

    const char cFill = pBar->cFill != '\0' ? pBar->cFill : '#';
    const char cBlank = pBar->cBlank != '\0' ? pBar->cBlank : '.';

    for (int i = 0; i < pBar->uWidth; i++) {
        putchar(i < iFilled ? cFill : cBlank);
    }

    printf("] %.0f%%", dFraction * 100.0);
    return true;
}
