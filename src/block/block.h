#pragma once
#include "common.h"
#include "inode/inode.h"

namespace solid {
    struct Block{
        union {
            uint8_t data[config::block_size];
            struct{
                // used for freelist
                BlockID fl_entry[config::block_size/sizeof(BlockID)];
            };
            struct{
                // used for indexing data blocks by inode
                BlockID bl_entry[config::block_size/sizeof(BlockID)];
            };
            INode inode[config::block_size/sizeof(INode)];
        };
    };
};