//
// Created by jan on 10/21/26.
//

#ifndef PLUTUS_CORE_H
#define PLUTUS_CORE_H

#include <stdint.h>

/**
 * Progress bar state. Initialize it with InitProgressBar before use or manually initialize cFill, cBlank and bInitialized.
 */
typedef struct tagProgressBar {
   char   *pszLabel;      /* Text shown before the bar. The caller owns the string. May be NULL. */
   double  dMin;          /* Lower bound of the range. */
   double  dMax;          /* Upper bound of the range. Must be greater than dMin. */
   double  dValue;        /* Current value, clamped to [dMin, dMax] when drawn. */
   uint8_t uWidth;        /* Number of cells between the brackets. Must be at least 1. */
   char    cFill;         /* Character drawn for filled cells, e.g. '#'. */
   char    cBlank;        /* Character drawn for empty cells, e.g. '.'. */
   bool    bInitialized;  /* Set by InitProgressBar. DrawProgress rejects the bar if false. */
} ProgressBar;

/**
 * Waits until the user presses Enter. Any text typed before it is discarded.
 */
void WaitForEnter(void);

/**
 * Prints a message and waits until the user presses Enter.
 * @param pszMsg Text printed before waiting. Does not add a newline.
 */
void Pause(const char *pszMsg);

/* Input helpers. Each reads a line, returns the parsed value, and asks
   again on invalid input. Exits the program if stdin is closed. */
int       ReadInt(void);
uint32_t  ReadUInt32(void);
uint64_t  ReadUInt64(void);
long long ReadLongLong(void);
float     ReadFloat(void);
double    ReadDouble(void);

/**
 * Initializes the progress bar struct with default values for the fill and blank characters.
 * @param pBar Pointer to the progress bar struct.
 * @return Operation result: true on success, false otherwise.
 */
bool InitProgressBar(ProgressBar *pBar);

/**
 * @brief Draws a progress bar to stdout.
 * @remarks The progress bar must be initialized before it is drawn. An uninitialized
 *          progress bar causes the function to return false. Manual initialization is possible.
 * @param pBar Pointer to the progress bar struct.
 * @return Operation result: true on success, false otherwise.
 */
bool DrawProgress(const ProgressBar *pBar);

#endif //PLUTUS_CORE_H
