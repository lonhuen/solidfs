#pragma once
#include "common.h"
#include "utils/log_utils.h"

class FileSystem {

    // TODO (Sherry)
    /*
    INode tmp;
    read_inod(id, &tmp);
    */
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