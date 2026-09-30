//
// Created by jan on 28/09/2026.
//

#ifndef NUCLEARPLANT_POWERPLANT_H
#define NUCLEARPLANT_POWERPLANT_H

#include "Reactor.h"

typedef struct tagPowerPlant {
    char* sName;
    int nReactors;
    Reactor* pReactors;
} PowerPlant, *PPowerPlant;

/**
 * Creates a new, global, power plant object.
 * @return Pointer to a newly-created power plant object.
 */
PPowerPlant PowerPlant_Create();

/**
 * Destroys the power plant object and frees the memory the object was using.
 * @param pPlant Pointer to a power plant object.
 * @return Operation result.
 */
bool PowerPlant_Destroy(PPowerPlant pPlant);

/**
 * Sets the power plant object to its default value.
 * @param pPlant Pointer to a power plant object.
 * @return Operation result.
 */
bool PowerPlant_Initialize(PPowerPlant* pPlant);

#endif //NUCLEARPLANT_POWERPLANT_H
