#pragma once
#include "common.h"

#define DATA_PTR_COUNT 13

enum inode_type {
  FREE,DIRECTORY
};

union INode{
  uint8_t data[256];
  struct {
    uint32_t inode_number;            // inode number
    uint16_t mode;                    // file type and mode
    uint32_t links;                   // # of hard links
    uint32_t uid;                     // user id of owner
    uint32_t gid;                     // group id of owner
    uint64_t size;                    // file size (in bytes)
    uint32_t block;                   // # of blocks allocated for file
    uint32_t atime;                   // last access time
    uint32_t ctime;                   // last change time (inode)
    uint32_t mtime;                   // last modify time (file content)
    bid_t p_block[DATA_PTR_COUNT];  // ptr to data blocks
    enum inode_type itype;
    //inode *next_free;                 // ptr to next free inode
  };
};