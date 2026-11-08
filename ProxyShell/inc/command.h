//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_COMMAND_H
#define PROXYSHELL_COMMAND_H

#include "macros.h"

/**
 * @brief Representing the basic command structure.
 */
typedef struct sCommand {
    char* name;                     // Name (alias) of the command.
    void (*action)(com_extra);  // Pointer to the command function.
} Command;

#endif //PROXYSHELL_COMMAND_H
