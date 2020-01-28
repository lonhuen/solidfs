#include "block/freelist_blockmanager.h"
#include "block/super_block.h"
#include "block/block.h"
#include "utils/log_utils.h"

/**
 * @brief write the first data block (i) with [i+512, i+1, i+2,...], then write the i+512 block
 *
*/
void FreeListBlockManager:: mkfs() {
    p_storage->read_block(0, (uint8_t*)(&sblock));
    initialized = true;

    auto i = sblock.s_dblock;
    struct Block tmp;
    BLOCK_ID* ptr = (BLOCK_ID*)tmp.data;
    for(;i + NR_BLOCKS_PER_GROUP < sblock.nr_block;i += NR_BLOCKS_PER_GROUP) {
        *ptr = i + NR_BLOCKS_PER_GROUP;
        for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
            *(ptr+j) = i + NR_BLOCKS_PER_GROUP + j; 
        }
        p_storage->write_block(i,tmp.data);
    }
    
    *ptr = 0;
    for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
        if (i + NR_BLOCKS_PER_GROUP + j < sblock.nr_block)
            *(ptr+j) = i + NR_BLOCKS_PER_GROUP + j;
        else
            *(ptr+j) = 0;
    }
    p_storage->write_block(i,tmp.data);
}

/**
 * @brief read a data block (we can't read a inode or super block)
 * @return 1 for success, 0 for fail
*/
// TODO(lonhh) whether we need to do range check? 
// TODO(lonhh) do we need to check that we are reading a valid block?
int FreeListBlockManager::read_dblock(BLOCK_ID id, uint8_t* dst) {
    if(!initialized) {
        LOG(ERROR) << "reading data block before mkfs";
        return 0;
    }
    if(id < sblock.s_dblock || id > sblock.nr_block) {
        LOG(ERROR) << "reading out of range in read_dblock";
        return 0;
    }
    return p_storage->read_block(id, dst);
}

// TODO(lonhh)
int FreeListBlockManager::write_dblock(BLOCK_ID id, const uint8_t* src) {
    if(!initialized) {
        LOG(ERROR) << "reading data block before mkfs";
        return 0;
    }
    if(id < sblock.s_dblock || id > sblock.nr_block) {
        LOG(ERROR) << "reading out of range in read_dblock";
        return 0;
    }
    return p_storage->write_block(id, src);

}

int FreeListBlockManager::allocate_dblock() {

}

int FreeListBlockManager::free_dblock(BLOCK_ID id) {

}