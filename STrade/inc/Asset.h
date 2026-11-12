//
// Created by jan on 11/11/26.
//

#ifndef STRADE_ASSET_H
#define STRADE_ASSET_H

#define MAX_ASSETS    1024
#include <stddef.h>

/**
 * @brief Representing a single tradable asset.
 */
typedef struct sAsset {
    unsigned long id;       // Id of the asset
    char* symbol;           // abbreviations, such as MSFT
    char* name;             // Name of the asset, e. g.: Microsoft
    unsigned long value;    // Value in cents: 1 USD = 100 cents
} Asset;

/**
 * Creates a new asset object.
 * @param id Asset id.
 * @param name Asset name.
 * @param symbol Asset symbol, e. g. SMBL.
 * @param value Value of the asset (in cents).
 * @return Pointer to the newly created Asset object. Must be freed with DeleteAsset() later.
 */
Asset* CreateAsset(unsigned long id, const char* name, const char* symbol, unsigned long value);

/**
 * Deletes the given asset object from memory.
 * @param asset Pointer to the asset object.
 * @return true if the asset was freed, otherwise false.
 */
bool DeleteAsset(Asset *asset);

/**
 * Loads assets from a data file.
 * @param filename Path to the data file.
 * @param assets Pointer to the array of asset pointer where the data will be stored.
 * @return true if any data are loaded, otherwise false.
 */
bool LoadAssets(const char* filename, Asset** assets, size_t* count);

/**
 * Unloads the assets inside the assets array from memory.
 * @param assets Pointer to the array of asset pointers.
 * @return true if the assets were freed, otherwise false.
 */
bool UnloadAssets(Asset **assets);

#endif //STRADE_ASSET_H
