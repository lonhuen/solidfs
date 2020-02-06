#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "fs/file_system.h"

Filesystem *fs;

int s_open(const char *path, struct fuse_file_info *info) {
    iid_t id;
    int p_res = fs->path2iid((const std::string)path, &id);
    if (p_res == 0) {  // inode not found
        LOG(ERROR) << "Cannot open file " << path;
        return 0;
    }

    info->fh = id;    // cache inode id
    return 1;
}

int s_read(const char *path, char *buf, size_t size,
           off_t offset, struct fuse_file_info *info){
    int res = s_open(path, info);
    if (res == 0) {
        LOG(ERROR) << "Cannot read file " << path;
        return 0
    }

    iid_t id = (iid_t) info->fh;
    INODE inode;
    fs->im.read_inode(id, inode.data);

    /*
    // check if inode is a directory
    if (inode.data.mode == DIRECTORY) {
        LOG(ERROR) << "File is a directory " << path;
        return 1
    }
    */
    
    return fs->read(id, (uint8_t *)buf, (uint32_t)size,(uint32_t)offset);
}

int s_write(const char *path, const char *buf, size_t size, off_t offset,
            struct fuse_file_info *info) {
    int res = s_open(path, info);
    if (res == 0) {
        LOG(ERROR) << "Cannot read file " << path;
        return 0
    }

    iid_t id = (iid_t) info->fh;
    INODE inode;
    fs->im.read_inode(id, inode.data);

    /*                                                                         
    // check if inode is a directory                                           
    if (inode.data.mode == DIRECTORY) {                                        
        LOG(ERROR) << "File is a directory " << path;                          
        return 1                                                               
    }                                                                          
    */
    
    return fs->write(id, (const uint8_t *)buf,
                     (uint32_t) size, (uint32_t) offset);
}

// list of file system functions
static struct fuse_operations s_oper = {
    .open = s_open,
    .read = s_read,
    .write = .s_write,
    // .lseek = .s_lseek,
    // .unlink = s_unlink,
    // .opendir = s_opendir,
    // .closedir = s_closedir,
    // .mkdir = s_mkdir,
    // .readdir = s_readdir,
    // .rmdir = s_rmdir,
};

int main(int argc, char *argv[]) {
  umask(0);
  return fuse_main(argc, argv, &s_oper, NULL);
} 
