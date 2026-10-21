//
// Created by jan on 10/21/26.
//

#ifndef PLUTUS_CORE_H
#define PLUTUS_CORE_H

#include <stdint.h>

void WaitForEnter(void);
void Pause(const char *msg);

/* Input helpers. Each reads a line, returns the parsed value, and asks
   again on invalid input. Exits the program if stdin is closed. */
int      ReadInt(void);
uint32_t ReadUInt32(void);
uint64_t ReadUInt64(void);
long long ReadLongLong(void);
float    ReadFloat(void);
double   ReadDouble(void);

#endif //PLUTUS_CORE_H
