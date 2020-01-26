#pragma once
#include "common.h"

struct super_block{
    uint32_t magic_number;

    BLOCK_ID nr_blocks;
    
    BLOCK_ID s_inode;
    BLOCK_ID nr_inodes;
    
    BLOCK_ID s_dblocks;
    BLOCK_ID nr_dblocks;
    
    BLOCK_ID id_free_dblocks;
};