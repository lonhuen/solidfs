#include "block/freelist_blockmanager.h"
#include "block/super_block.h"
#include "block/block.h"
#include "utils/log_utils.h"
#include "utils/fs_exception.h"

namespace solid {
    /**
     * @brief write the first data block (i) with [i+512, i+1, i+2,...], then write the i+512 block
     *
    */
    void FreeListBlockManager:: mkfs() {
        p_storage->read_block(0, sblock.data);
        initialized = true;

        auto i = sblock.s_dblock;
        struct Block tmp;
        BlockID* ptr = (BlockID*)tmp.data;
        for(;i + nr_blocks_per_group < sblock.nr_block;i += nr_blocks_per_group) {
            *ptr = i + nr_blocks_per_group;
            for(auto j=1;j<nr_blocks_per_group;j++) {
                *(ptr+j) = i + j; 
            }
            p_storage->write_block(i,tmp.data);
        }
        *ptr = 0;
        for(auto j=1;j<nr_blocks_per_group;j++) {
            if (i + j < sblock.nr_block)
                *(ptr+j) = i + j;
            else
                *(ptr+j) = 0;
        }
        p_storage->write_block(i,tmp.data);

        sblock.h_dblock = sblock.s_dblock;
        p_storage->write_block(0,sblock.data);
    }

    /**
     * @brief read a data block (we can't read a inode or super block)
     * @return return the data block
    */
    Block FreeListBlockManager::read_dblock(BlockID id) {
        if(!initialized) {
            throw fs_exception("read_dblock before mkfs");
        }
        if(id < sblock.s_dblock || id >= sblock.nr_block) {
            throw fs_exception("read_dblock ", id ," not a valid a data block");
        }
        Block bl;
        p_storage->read_block(id, bl.data);
        return bl;
    }

    // TODO(lonhh)
    void FreeListBlockManager::write_dblock(BlockID id, Block& bl) {
        if(!initialized) {
            throw fs_exception("write_dblock before mkfs");
        }
        if(id < sblock.s_dblock || id >= sblock.nr_block) {
            throw fs_exception("write_dblock ", id ," not a valid a data block");
        }
        p_storage->write_block(id, bl.data);
    }

    /**
     * @brief allocate a data block
     * @return the BlockID of the allocated block, 0 for failure
    */
    BlockID FreeListBlockManager::allocate_dblock() {
        //let's assume that the h_dblock will be always the updated
        BlockID head = sblock.h_dblock;
        if (head == 0) {
            throw fs_exception("allocate_dblock: run out of data block");
        }
        Block bl = read_dblock(head);
        uint32_t i=1;
        for(;i<nr_blocks_per_group;i++) {
            if(bl.fl_entry[i] != 0)
                break;
        }
        if(i < nr_blocks_per_group) {
            BlockID ret = bl.fl_entry[i];
            bl.fl_entry[i] = 0;
            write_dblock(head,bl);
            return ret;
        } else {
            BlockID new_head = bl.fl_entry[0];
            bl.fl_entry[0] = 0;
            write_dblock(head,bl);
            sblock.h_dblock = new_head;
            p_storage->write_block(0,sblock.data);
            return head;
        }
    }

    /**
     * @brief free a data block with BlockID id, inserting to the head of the free list
     * @return 1 for success, 0 for failure
    */

    // TODO(lonhh): whether we need to make sure that id is in range?
    // TODO(lonhh): one performance issue----if we free one block, which needs to write the sblock, and then allocate a block and free it again.
    // TODO(lonhh): also do we need to check that the block is avaible or not? double free?
    void FreeListBlockManager::free_dblock(BlockID id) {
        BlockID head = sblock.h_dblock;
        Block bl;
        p_storage->read_block(head,bl.data); 
        uint32_t i=1;
        for(;i<nr_blocks_per_group;i++) {
            if(bl.fl_entry[i] == 0)
                break;
        }
        if(head!=0 && i < nr_blocks_per_group) {
            bl.fl_entry[i] = id;
            write_dblock(head,bl);
        } else {
            // write the block
            Block tmp;
            memset(tmp.fl_entry+1,0,config::block_size);
            tmp.fl_entry[0] = sblock.h_dblock;
            write_dblock(id,tmp);
            // modify the super block
            sblock.h_dblock = id;
            p_storage->write_block(0,sblock.data);
        }
    }
};