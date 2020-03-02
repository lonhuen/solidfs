#pragma once
#include "common.h"
namespace solid {
    union super_block{
        struct {
            uint64_t magic_number;

            BlockID nr_block;

            // all in terms of block 
            BlockID s_iblock;
            BlockID nr_iblock;
            
            // all in terms of block 
            BlockID s_dblock;
            BlockID nr_dblock;

            BlockID h_dblock;
        };
        uint8_t data[config::block_size];
    };
};