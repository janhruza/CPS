//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_COMMANDS_H
#define PROXYSHELL_COMMANDS_H

#include "command.h"

extern Command commands[MAX_COMMANDS];

void CmdCat(com_extra);
void CmdTouch(com_extra);
void CmdClear(com_extra);
void CmdExit(com_extra);
void CmdListDir(com_extra);
void CmdTimeNow(com_extra);
void CmdDate(com_extra);

#endif //PROXYSHELL_COMMANDS_H
