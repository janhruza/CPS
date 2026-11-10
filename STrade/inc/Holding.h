//
// Created by jan on 11/11/26.
//

#ifndef STRADE_HOLDING_H
#define STRADE_HOLDING_H

/**
 * @brief Representing information about a single asset owned by a player.
 */
typedef struct sHolding {
    unsigned int assetId;  // Id of the asset.
    unsigned int quantity;  // Quantity, e. g. number of stocks.
} Holding;

#endif //STRADE_HOLDING_H
