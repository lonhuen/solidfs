#pragma once

#include "common.h"
#include "inode/inode.h"
#include "storage/storage.h"
#include "block/super_block.h"

namespace solid {
    class INodeManager {
    private:
        BlockID s_iblock;
        BlockID nr_iblock;
        Storage* storage;

    public:
        const static uint64_t nr_inode_per_block = config::block_size/sizeof(INode);

        INodeManager(Storage* storage,const super_block* p_sb=nullptr);
        virtual void mkfs();
        virtual INode read_inode(INodeID id);
        virtual void write_inode(INodeID id, const INode& src);
        virtual INodeID allocate_inode();
        virtual void free_inode(INodeID id);
    };
};