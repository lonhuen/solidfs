#pragma once

#include <string>
#include <vector>
#include "common.h"
#include "utils/log_utils.h"
#include "inode/inode_manager.h"
#include "block/block_manager.h"
#include "directory/directory.h"

class FileSystem {
    INodeManager* im;
    BlockManager* bm;
    Storage* storage;

public:
    // just used for DEBUG
    FileSystem() {};
    FileSystem(bid_t nr_blocks,bid_t nr_iblock_blocks);
    void mkfs();
    iid_t path2iid(const std::string& path);
    Directory read_directory(iid_t id);
    int read(iid_t id,uint8_t* dst,uint32_t size,uint32_t offset);

private:
    std::vector<bid_t> read_dblock_indexs(INode& inode,uint32_t begin,uint32_t end);
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