#pragma once

#include "common.h"
#include "inode/inode.h"
#include "storage/storage.h"

// let's say we won't modify the super block
class INodeManager {
    private:
        // s_iblock is the starting block id of inode list
        bid_t s_iblock;
        // s_iblock is the number of blocks of inode list
        bid_t nr_iblock;
        Storage* p_storage;

    public:
        const static uint32_t nr_iblock_PER_BLOCK = BLOCK_SIZE/sizeof(INode);

        INodeManager(Storage* p_storage);
        virtual void mkfs();
        virtual int read_inode(iid_t id, uint8_t *dst);
        virtual int write_inode(iid_t id, const uint8_t *dst);
        virtual iid_t allocate_inode();
        virtual int free_inode(iid_t id);
};