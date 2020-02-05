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
    bid_t* ptr = (bid_t*)tmp.data;
    for(;i + NR_BLOCKS_PER_GROUP < sblock.nr_block;i += NR_BLOCKS_PER_GROUP) {
        *ptr = i + NR_BLOCKS_PER_GROUP;
        for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
            *(ptr+j) = i + j; 
        }
        p_storage->write_block(i,tmp.data);
    }
    *ptr = 0;
    for(auto j=1;j<NR_BLOCKS_PER_GROUP;j++) {
        if (i + j < sblock.nr_block)
            *(ptr+j) = i + j;
        else
            *(ptr+j) = 0;
    }
    p_storage->write_block(i,tmp.data);

    // TODO(lonhh): whether this should be initialized by file system or block manager
    sblock.h_dblock = sblock.s_dblock;
    p_storage->write_block(0,sblock.data);
}

/**
 * @brief read a data block (we can't read a inode or super block)
 * @return 1 for success, 0 for fail
*/
// TODO(lonhh) whether we need to do range check? 
// TODO(lonhh) do we need to check that we are reading a valid block?
int FreeListBlockManager::read_dblock(bid_t id, uint8_t* dst) {
    if(!initialized) {
        LOG(WARNING) << "reading data block before mkfs";
        return 0;
    }
    if(id < sblock.s_dblock || id > sblock.nr_block) {
        LOG(WARNING) << "reading out of range in read_dblock";
        return 0;
    }
    return p_storage->read_block(id, dst);
}

// TODO(lonhh)
int FreeListBlockManager::write_dblock(bid_t id, const uint8_t* src) {
    if(!initialized) {
        LOG(WARNING) << "reading data block before mkfs";
        return 0;
    }
    if(id < sblock.s_dblock || id > sblock.nr_block) {
        LOG(WARNING) << "reading out of range in read_dblock";
        return 0;
    }
    return p_storage->write_block(id, src);

}

/**
 * @brief allocate a data block
 * @return the bid_t of the allocated block, 0 for failure
*/
int FreeListBlockManager::allocate_dblock() {
    //let's assume that the h_dblock will be always the updated
    bid_t head = sblock.h_dblock;
    Block bl;
    if (read_dblock(head,bl.data)) {
        uint32_t i=1;
        for(;i<NR_BLOCKS_PER_GROUP;i++) {
            if(bl.fl_entry[i] != 0)
                break;
        }
        if(i < NR_BLOCKS_PER_GROUP) {
            bid_t ret = bl.fl_entry[i];
            bl.fl_entry[i] = 0;
            write_dblock(head,bl.data);
            return ret;
        } else {
            bid_t new_head = bl.fl_entry[0];
            bl.fl_entry[0] = 0;
            write_dblock(head,bl.data);
            sblock.h_dblock = new_head;
            p_storage->write_block(0,sblock.data);
            return head;
        }
    }
    LOG(WARNING) << "failed when allocating a data block";
    return 0;
}

/**
 * @brief free a data block with bid_t id, inserting to the head of the free list
 * @return 1 for success, 0 for failure
*/

// TODO(lonhh): whether we need to make sure that id is in range?
// TODO(lonhh): one performance issue----if we free one block, which needs to write the sblock,
//              and then allocate a block and free it again.
// TODO(lonhh): also do we need to check that the block is avaible or not? double free?
int FreeListBlockManager::free_dblock(bid_t id) {
    bid_t head = sblock.h_dblock;
    Block bl;
    uint32_t i=1;
    if (read_dblock(head,bl.data)) {
        for(;i<NR_BLOCKS_PER_GROUP;i++) {
            if(bl.fl_entry[i] == 0)
                break;
        }
    }
    if(head!=0 && i < NR_BLOCKS_PER_GROUP) {
        bl.fl_entry[i] = id;
        write_dblock(head,bl.data);
        return 1;
    } else {
        // write the block
        Block tmp;
        memset(tmp.fl_entry+1,0,BLOCK_SIZE);
        tmp.fl_entry[0] = sblock.h_dblock;
        write_dblock(id,tmp.data);
        // modify the super block
        sblock.h_dblock = id;
        p_storage->write_block(0,sblock.data);
        return 1;
    }
    return 0;
}