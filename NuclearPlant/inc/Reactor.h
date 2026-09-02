//
// Created by jan on 02/09/2026.
//

#ifndef NUCLEARPLANT_REACTOR_H
#define NUCLEARPLANT_REACTOR_H

#include <stdbool.h>

typedef struct tagReactor {
    unsigned int Id;    // core id
    double Temperature; // temp in C
} Reactor, *PReactor;

bool Reactor_Init(PReactor pReactor);
bool Reactor_Cleanup(PReactor pReactor);

#endif //NUCLEARPLANT_REACTOR_H
