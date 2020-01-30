#pragma once

#include "common.h"
#include "inode/inode.h"
#include "storage/storage.h"

// let's say we won't modify the super block
class INodeManager {
    private:
        // s_inode is the starting block id of inode list
        bid_t s_inode;
        // s_inode is the number of blocks of inode list
        bid_t nr_inode;
        Storage* p_storage;

    public:
        const static uint32_t NR_INODE_PER_BLOCK = BLOCK_SIZE/sizeof(union INode);

        INodeManager(Storage* p_storage);
        virtual void mkfs();
        virtual int read_inode(iid_t id, uint8_t *dst);
        virtual int write_inode(iid_t id, const uint8_t *dst);
        virtual iid_t allocate_inode();
        virtual int free_inode(iid_t id);
};