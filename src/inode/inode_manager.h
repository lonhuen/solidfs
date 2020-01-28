#pragma once
#include "common.h"

class INodeManager {
 private:
  BLOCK_ID inode_lst_start;
  BLOCK_ID inode_lst_end;

 public:
  virtual void mkfs();
  virtual int read_inode(INODE_ID id, uint_t *dst);
  virtual int write_inode(INODE_ID id, uint_t *dst);
  virtual int allocate_inode() = 0;
  virtual int free_inode(INODE_ID id);
}
