//
// Created by jan on 02/09/2026.
//

#include "../inc/Reactor.h"

#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

bool Reactor_Init(const PReactor pReactor) {
    if (!pReactor) {
        return false;
    }

    pReactor->Id = 0;
    pReactor->State = STATE_INIT;
    pReactor->Uptime = 0;
    pReactor->Temperature = 0;
    pReactor->Power = 0;
    return true;
}

bool Reactor_Cleanup(const PReactor pReactor) {
    if (!pReactor) return false;
    memset(pReactor, 0, sizeof(Reactor));
    return true;
}

bool Reactor_Update(const PReactor pReactor) {
    if (!pReactor) return false;
    pReactor->Uptime++;
    return true;
}

bool Reactor_Create(const PReactor pReactor, const unsigned int coreId) {
    if (pReactor == NULL) {
        return false;
    }

    pReactor->Id = coreId;
    pReactor->State = STATE_DOWN;
    return true;
}

static char* GetStateStr(const REACTOR_STATE state) {
    switch (state) {
        case STATE_INIT: return "INIT";
        case STATE_DOWN: return "DOWN";
        case STATE_UP: return "UP";
        case STATE_CRITICAL: return "CRIT";
        default: return "UNKN";
    }
}

bool Reactor_PrintState(const PReactor pReactor) {
    if (!pReactor) return false;

    printf("Reactor #%02d: [%s] %.2fC %dW\n", pReactor->Id, GetStateStr(pReactor->State), pReactor->Temperature, pReactor->Power);
    return true;
}