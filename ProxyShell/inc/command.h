//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_COMMAND_H
#define PROXYSHELL_COMMAND_H

#define command_extra void* pExtra1, void* pExtra2

/**
 * @brief Representing the basic command structure.
 */
typedef struct sCommand {
    char* name;                     // Name (alias) of the command.
    void (*action)(command_extra);  // Pointer to the command function.
} Command;

#endif //PROXYSHELL_COMMAND_H
