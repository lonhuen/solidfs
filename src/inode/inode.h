#progma once
#include "common.h"

typedef struct INode{
  // attributes referred from stat system call
  uint32_t device;        // id of device containing file
  uint32_t inode_number;  // inode number
  uint32_t mode;          // file type and node
  uint32_t link_count;    // number of hard links
  uint32_t user_id;       // user id of owner
  uint32_t group_id;      // group id of owner
  uint32_t device_id;     // device id (if special file)
  uint64_t size;          // size of file (in bytes)
  uint64_t block_count;   // number of blocks allocated for file
  uint32_t change_time;   // when inode last changed
  uint32_t modify_time;   // when file last changed
  uint32_t access_time;   // when file last accessed
}
