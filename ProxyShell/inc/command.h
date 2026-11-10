//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_COMMAND_H
#define PROXYSHELL_COMMAND_H

#include "macros.h"
#include "types.h"

/**
 * @brief Representing the basic command structure.
 */
typedef struct sCommand {
    char* name;                     // Name (alias) of the command.
    void (*action)(com_args);  // Pointer to the command function.
} Command;

/**
 * @brief Gets the error message associated with the given error code.
 * @return Corresponding error message or NULL, if the error code is invalid.
 */
const char* GetErrorMsg(unsigned int errorCode);

#endif //PROXYSHELL_COMMAND_H
