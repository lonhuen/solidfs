#pragma once
#include "common.h"

class INodeManager {
 private:
  bid_t inode_lst_start;
  bid_t inode_lst_end;

 public:
  virtual void mkfs();
  virtual int read_inode(INODE_ID id, uint_t *dst);
  virtual int write_inode(INODE_ID id, uint_t *dst);
  virtual int allocate_inode() = 0;
  virtual int free_inode(INODE_ID id);
}
