//
// Created by jan on 02/09/2026.
//

#include "../inc/Reactor.h"

#include <stddef.h>

bool Reactor_Init(const PReactor pReactor) {
    if (!pReactor) {
        return false;
    }

    pReactor->Id = 0;
    pReactor->State = STATE_INIT;
    pReactor->Uptime = 0;
    pReactor->Temperature = 0;
    return true;
}

bool Reactor_Cleanup(PReactor pReactor) {
    return false;
}

bool Reactor_Update(PReactor pReactor) {
    return false;
}

bool Reactor_Create(const PReactor pReactor, unsigned int coreId) {
    if (pReactor == NULL) {
        return false;
    }

    pReactor->Id = coreId;
    pReactor->State = STATE_DOWN;
    return true;
}