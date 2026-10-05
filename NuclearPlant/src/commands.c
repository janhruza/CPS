//
// Created by jan on 05/10/2026.
//

#include "../inc/commands.h"
#include "../inc/Version.h"

#include <stdio.h>
#include <stdlib.h>

RESULT CmdQuit(void) {
    fprintf(stdout, "Exiting...\n");
    exit(EXIT_SUCCESS);
    return SUCCESS;
}

RESULT CmdHelp(void) {
    fprintf(stderr, "Help not implemented.\n");
    return SUCCESS;
}

RESULT CmdVersion(void) {
    Version version;
    GetVersion(&version);
    fprintf(stdout, "%d.%d\n", version.Major, version.Minor);
    return SUCCESS;
}

RESULT CmdClear(void) {
    fprintf(stdout, "%s", ANSI_CLEAR);
    return SUCCESS;
}
