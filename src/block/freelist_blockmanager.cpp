#include "block/freelist_blockmanager.h"
#include "block/super_block.h"
#include "block/block.h"

void FreeListBlockManager:: mkfs() {
    struct super_block sblock;
    p_storage->read_block(0, (uint8_t*)(&sblock));

    auto i = sblock.s_dblocks;
    struct Block tmp;
    BLOCK_ID* ptr = (BLOCK_ID*)tmp.data;
    for(i + NR_BLOCKS_PER_GROUP < sblock.nr_blocks;i += NR_BLOCKS_PER_GROUP) {
        *ptr = i + NR_BLOCKS_PER_GROUP;
        for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
            *(ptr+j) = i + NR_BLOCKS_PER_GROUP + j; 
        }
        write_dblock(i,tmp.data);
    }
    
    *ptr = 0;
    for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
        if (i + NR_BLOCKS_PER_GROUP + j < sblock.nr_blocks)
            *(ptr+j) = i + NR_BLOCKS_PER_GROUP + j;
        else
            *(ptr+j) = 0;
    }
    write_dblock(i,tmp.data);

}

int FreeListBlockManager::read_dblock(BLOCK_ID id, uint8_t* dst) {

}

int FreeListBlockManager::write_dblock(BLOCK_ID id, const uint8_t* dst) {

}

int FreeListBlockManager::allocate_dblock() {

}

int FreeListBlockManager::free_dblock(BLOCK_ID id) {

}