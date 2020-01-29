#pragma once
#include "common.h"

union super_block{
    struct {
        uint32_t magic_number;

        bid_t nr_block;
        
        bid_t s_inode;
        bid_t nr_inode;
        
        bid_t s_dblock;
        bid_t nr_dblock;

        bid_t h_dblock;
    };
    uint8_t data[BLOCK_SIZE];
};