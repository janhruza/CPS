//
// Created by jan on 02/09/2026.
//

#ifndef NUCLEARPLANT_REACTOR_H
#define NUCLEARPLANT_REACTOR_H

#include <stdbool.h>

/**
 * Representing a simple nuclear plant reactor.
 */
typedef struct tagReactor {
    /**
     * Representing the reactor's identifier.
     */
    unsigned int Id;

    /**
     * Representing the core temperature, in Celsius.
     */
    double Temperature;
} Reactor, *PReactor;

/**
 * Initializes the reactor object.
 * @param pReactor Pointer to a reactor object.
 * @return Operation result.
 */
bool Reactor_Init(PReactor pReactor);

/**
 * Cleans up the reactor object.
 * @param pReactor Pointer to a reactor object.
 * @return Operation result.
 */
bool Reactor_Cleanup(PReactor pReactor);

#endif //NUCLEARPLANT_REACTOR_H
