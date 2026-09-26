//
// Created by jan on 23/09/2026.
//

#ifndef NUCLEARPLANT_VERSION_H
#define NUCLEARPLANT_VERSION_H

typedef struct tagVersion {
    int Major;
    int Minor;
} Version;

int SetVersion(Version *version);
int GetVersion(Version *version);

#endif //NUCLEARPLANT_VERSION_H
