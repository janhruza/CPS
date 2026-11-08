//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_TYPES_H
#define PROXYSHELL_TYPES_H

typedef void* PTR;

typedef enum eErrorCodes {
    S_OK = 0,
    E_FILE_NOT_FOUND,
    E_NULLPTR,
    E_GENERIC,
    E_OPEN,
    E_CLOSE,
    E_READ,
    E_WRITE,
    E_LAST              // last value, used for bounds checking
} ErrorCode;

typedef struct sErrorInfo {
    unsigned int errorCode;
    char* errorMessage;
} ErrorInfo;

#endif //PROXYSHELL_TYPES_H
