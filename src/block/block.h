#include "common.h"

// TODO(lonhh): whether we need to use BLOCK or just uint8_t* to represent a block
//
struct Block{
    union {
        uint8_t data[BLOCK_SIZE];
    };
};