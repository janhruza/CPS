//
// Created by jan on 10/17/26.
//

#ifndef PLUTUS_CONFIG_H
#define PLUTUS_CONFIG_H

#include <stdint.h>

#define CONFIG_FILE_NAME "config.bin"

/**
 * Representing a simple configuration file structure.
 */
typedef struct tagConfig {
    uint64_t uNextTransactionId; /* Representing the next transaction ID. */
} Config;

bool ConfigRead(const Config *pConfig);
bool ConfigWrite(const Config *pConfig);


#endif //PLUTUS_CONFIG_H
