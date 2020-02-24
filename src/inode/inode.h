#pragma once
#include "common.h"
#include <utility>
#include <fuse.h>

namespace solid {
  enum INodeType {
    FREE,DIRECTORY,REGULAR
  };
  struct INode{
    union {
      uint8_t data[config::inode_size];
      struct {
        uint32_t inode_number;                          // inode number
        uint16_t mode;                                  // file type and mode
        uint32_t links;                                 // # of hard links
        uint32_t uid;                                   // user id of owner
        uint32_t gid;                                   // group id of owner
        uint64_t size;                                  // file size (in bytes)
        uint32_t block;                                 // # of blocks allocated for file
        uint32_t atime;                                 // last access time
        uint32_t ctime;                                 // last change time (inode)
        uint32_t mtime;                                 // last modify time (file content)
        BlockID p_block[config::data_ptr_cnt];          // ptr to data blocks
        enum INodeType itype;
      };
    };
    //TODO(lonhh): whether mode_t matches uint16_t?
    static INode get_inode(INodeID inode_number,enum INodeType itype,mode_t mode) {
      fuse_context* context = fuse_get_context(); 
      INode inode;
      inode.inode_number = inode_number;
      inode.itype = itype;
      inode.block = 0;
      inode.size = 0;
      inode.links = 1;
      inode.uid = context->uid;
      inode.gid = context->gid;
      inode.mode = mode & (~context->umask);
      return inode;
    }
    
    static void init_inode(INode& inode,INodeID inode_number,enum INodeType itype,mode_t mode) {
      fuse_context* context = fuse_get_context(); 
      inode.inode_number = inode_number;
      inode.itype = itype;
      inode.block = 0;
      inode.size = 0;
      inode.links = 1;
      inode.uid = context->uid;
      inode.gid = context->gid;
      inode.mode = mode & (~context->umask);
    }
  };

};