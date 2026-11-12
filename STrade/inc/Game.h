//
// Created by jan on 11/11/26.
//

#ifndef STRADE_GAME_H
#define STRADE_GAME_H

#include "Holding.h"
#include "Asset.h"

/**
 * @brief Representing the game state struct.
 */
typedef struct sGame {
    char* playerName;       // Representing the player's name
    Holding *holding;       // Representing the assets owned by a player.
    Asset **assets;         // Representing an array of all tradable assets (basically the market itself).
    unsigned int seed;      // Representing the initial seed used for modeling of the entire run.
    unsigned int day;       // Representing the current iteration of the game loop where each iteration represents a single day on the stock market.
} Game;

/**
 * @brief Creates a new game object.
 * @return Pointer to a newly created game object, otherwise NULL.
 */
Game* GameCreate(void);

/**
 * @brief Loads a saved game from a save file.
 * @param path Path to the game data file.
 * @return Pointer to the new game object on success, otherwise NULL.
 */
Game* GameLoad(const char* path);

/**
 * @brief Destroys the given game object and frees its memory.
 * @param game Pointer to a game object. If the game is NULL, the return value will be false.
 * @return true if the game was destroyed, otherwise false.
 */
bool GameDestroy(Game* game);

/**
 * @brief Initializes a created instance of the game object.
 * @param game Pointer to the game object.
 * @return true if the game object was initialized, otherwise false.
 */
bool GameInit(Game* game);

#endif //STRADE_GAME_H
