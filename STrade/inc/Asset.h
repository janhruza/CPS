//
// Created by jan on 11/11/26.
//

#ifndef STRADE_ASSET_H
#define STRADE_ASSET_H

#define MAX_ASSETS    1024

/**
 * @brief Representing a single tradable asset.
 */
typedef struct sAsset {
    unsigned int id;        // Id of the asset
    char tag[4];            // abbreviations, such as MSFT
    char* name;             // Name of the asset, e. g.: Microsoft
    unsigned long value;    // Value in cents: 1 USD = 100 cents
} Asset;

#endif //STRADE_ASSET_H
