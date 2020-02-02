#pragma once

#include "common.h"
#include "utils/log_utils.h"
#include "inode/inode_manager.h"
#include "block/block_manager.h"
#include <string>

class FileSystem {
    INodeManager* im;
    BlockManager* bm;
    Storage* storage;

    FileSystem(bid_t nr_blocks,bid_t nr_iblock_blocks);
    void mkfs();
    iid_t path2iid(const std::string& path);
};


/* 
    // TODO (Sherry)
    INode tmp;
    read_inod(id, &tmp);
    read_inode(iid_t id, inode* dst) {
    }
    // external api
    read(str path, buffer, offset,size);
    // internal api
    read(iid_t id, buffer, offset,size);

    // create file
    allocate_inode(path);
    // external api
    write(str path, buffer, offset,size);
    // internal api
    write(iid_t id, buffer, offset,size);


    open(path) {
        path2id();
    }
    //close();
    // TODO(lonhh)
    mkdir();
    readdir();
    rmdir();
    mkfs();
    path2id(str path);
};
*/