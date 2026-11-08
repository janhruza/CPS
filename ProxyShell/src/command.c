//
// Created by jan on 11/8/26.
//

#include "../inc/command.h"

static ErrorInfo errorTable[E_LAST] = {
    (ErrorInfo) {.errorCode = S_OK, .errorMessage = "Operation completed successfully!"},
    (ErrorInfo) {.errorCode = E_FILE_NOT_FOUND, .errorMessage = "File not found."},
    (ErrorInfo) {.errorCode = E_NULLPTR, .errorMessage = "Invalid argument (NULL)."},
    (ErrorInfo) {.errorCode = E_GENERIC, .errorMessage = "Operation failed."},
    (ErrorInfo) {.errorCode = E_OPEN, .errorMessage = "Unable to open file."},
    (ErrorInfo) {.errorCode = E_CLOSE, .errorMessage = "Unable to close file."},
    (ErrorInfo) {.errorCode = E_READ, .errorMessage = "Unable to read from file."},
    (ErrorInfo) {.errorCode = E_WRITE, .errorMessage = "Unable to write to file."}
};

const char* GetErrorMsg(const unsigned int errorCode) {
    if (errorCode >= E_LAST) {
        return nullptr;
    }

    return errorTable[errorCode].errorMessage;
}