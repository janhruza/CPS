//
// Created by jan on 11/10/26.
//

#ifndef STRADE_TYPES_H
#define STRADE_TYPES_H

typedef enum eResult {
    S_OK        = 0,    // Operation success.
    E_GENERIC,          // Generic error
    E_NULLPTR,          // Pointer is NULL.
    E_OPEN,             // Error while opening a file.
    E_READ,             // Error while reading a file.
    E_WRITE,            // Error while writing to file.
    E_LAST              // Last value, unused. Must be the last, used for bounds checking.
} Result;

#endif //STRADE_TYPES_H
