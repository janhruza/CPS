//
// Created by jan on 02/09/2026.
//

#ifndef NUCLEARPLANT_REACTOR_H
#define NUCLEARPLANT_REACTOR_H

#include <stdbool.h>

typedef enum tagReactorState {
    STATE_DOWN,
    STATE_UP,
    STATE_INIT,
    STATE_CRITICAL
} REACTOR_STATE;

/**
 * Representing a simple nuclear plant reactor.
 */
typedef struct tagReactor {
    /**
     * Representing the reactor's identifier.
     */
    unsigned int Id;

    /**
     * Representing the reactor state - on/off.
     */
    unsigned int State;

    /**
     * Representing the reactor's uptime in simulation units.
     * The uptime is incremented with each reactor update.
     */
    unsigned int Uptime;

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

/**
 * Updates the reactor state.
 * @param pReactor Pointer to a reactor object.
 * @return Operation result.
 */
bool Reactor_Update(PReactor pReactor);

bool Reactor_Create(PReactor pReactor, unsigned int coreId);

#endif //NUCLEARPLANT_REACTOR_H
