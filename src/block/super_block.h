#pragma once
#include "common.h"

union super_block{
    struct {
        uint32_t magic_number;

        BLOCK_ID nr_block;
        
        BLOCK_ID s_inode;
        BLOCK_ID nr_inode;
        
        BLOCK_ID s_dblock;
        BLOCK_ID nr_dblock;

        BLOCK_ID h_dblock;
    };
    uint8_t data[BLOCK_SIZE];
};