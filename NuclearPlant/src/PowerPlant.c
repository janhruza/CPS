//
// Created by jan on 28/09/2026.
//

#include <stdlib.h>
#include <string.h>
#include "../inc/PowerPlant.h"

PPowerPlant PowerPlant_Create() {
    return (PPowerPlant)malloc(sizeof(PowerPlant));
}

bool PowerPlant_Destroy(PPowerPlant pPlant) {
    if (!pPlant) return false;
    free(pPlant);
    return true;
}

bool PowerPlant_Initialize(PPowerPlant* pPlant) {
    if (!pPlant) return false;
    memset(pPlant, 0, sizeof(PowerPlant));
    return true;
}