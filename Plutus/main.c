#include <stdio.h>
#include <stdlib.h>

#include "inc/Config.h"

int main(int argc, const char** argv, char** envp) {

    Config config = {0};
    config.uNextTransactionId = 12;

    Config newCfg = {0};
    ConfigWrite(&config);
    ConfigRead(&newCfg);

    printf("--- CONFIG ---\n");
    printf("OLD: uNextTransactionId = %lu\n", config.uNextTransactionId);
    printf("NEW: uNextTransactionId = %lu\n", newCfg.uNextTransactionId);
    printf("--- End of CONFIG ---\n");

    return EXIT_SUCCESS;
}
