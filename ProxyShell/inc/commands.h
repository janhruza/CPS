//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_COMMANDS_H
#define PROXYSHELL_COMMANDS_H

#include "command.h"

extern Command commands[MAX_COMMANDS];

void CmdCat(com_args);
void CmdTouch(com_args);
void CmdClear(com_args);
void CmdExit(com_args);
void CmdListDir(com_args);
void CmdTimeNow(com_args);
void CmdDate(com_args);
void CmdOpen(com_args);

#endif //PROXYSHELL_COMMANDS_H
