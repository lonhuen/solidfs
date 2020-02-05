#include "common.h"
#include "inode/inode.h"

// TODO(lonhh): whether we need to use BLOCK or just uint8_t* to represent a block
//
struct Block{
    union {
        uint8_t data[BLOCK_SIZE];
        struct{
            // used for freelist
            bid_t fl_entry[BLOCK_SIZE/sizeof(bid_t)];
        };
        struct{
            // used for indexing data blocks by inode
            bid_t bl_entry[BLOCK_SIZE/sizeof(bid_t)];
        };
        INode inode[BLOCK_SIZE/sizeof(INode)];
    };
};