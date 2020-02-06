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

FileSystem *fs;

extern "C" {

    void *s_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
        return NULL;
    }

    int s_open(const char *path, int info) { //struct fuse_file_info *info) {
        iid_t id;
        int p_res = fs->path2iid((const std::string)path, &id);
        if (p_res == 0) {  // inode not found
            LOG(ERROR) << "Cannot open file " << path;
            return 0;
        }

        info = (int) id;    // cache inode id
        return 1;
    }

    int s_read(const char *path, char *buf, size_t size, off_t offset) {
        // struct fuse_file_info *info){
        int info;
        int res = s_open(path, info);
        if (res == 0) {
            LOG(ERROR) << "Cannot read file " << path;
            return 0;
        }

        iid_t id = (iid_t) info;
        INode inode;
        fs->im->read_inode(id, inode.data);

        /*
        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {
            LOG(ERROR) << "File is a directory " << path;
            return 0
        }
        */
    
        return fs->read(id, (uint8_t *)buf, (uint32_t)size,(uint32_t)offset);
    }

    int s_write(const char *path, const char *buf, size_t size, off_t offset) {
            // struct fuse_file_info *info) {
        int info;
        int res = s_open(path, info);
        if (res == 0) {
            LOG(ERROR) << "Cannot read file " << path;
            return 0;
        }

        iid_t id = (iid_t) info;
        INode inode;
        fs->im->read_inode(id, inode.data);

        /*                                                                         
        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {                                        
            LOG(ERROR) << "File is a directory " << path;                          
            return 0                                                               
        }                                                                          
        */
    
        return fs->write(id, (const uint8_t *)buf,
                         (uint32_t) size, (uint32_t) offset);
    }

/*
// list of file system function
struct fuse_operations s_oper = {
    .init = &s_init,
    .open = &s_open,
    .read = &s_read,
    .write = &s_write,
    // .lseek = .s_lseek,
    // .unlink = s_unlink,
    // .opendir = s_opendir,
    // .closedir = s_closedir,
    // .mkdir = s_mkdir,
    // .readdir = s_readdir,
    // .rmdir = s_rmdir,
};	
*/  
}
  
int main(int argc, char *argv[]) {
    fs = new FileSystem(10 + 512 + 512 * 512, 9);
    fs->mkfs();

    fuse_operations s_oper;
    memset(&s_oper, 0, sizeof(s_oper));

    // s_oper.init = &s_init;
    s_oper.open = &s_open;
    s_oper.read = &s_read;
    s_oper.write = &s_write;
  
    int argcount = 0;
    char *argument[12];

    char s[] = "-s"; // Use a single thread.
    char d[] = "-d"; // Print debuging output.
    char f[] = "-f"; // Run in the foreground.
    char o[] = "-o"; // Other options
    char p[] = "default_permissions"; // Defer permissions checks to kernel
    char r[] = "allow_other"; // Allow all users to access files

    char mount_point[] = "temp/";

    argument[argcount++] = argv[0];
    argument[argcount++] = f;   
    argument[argcount++] = mount_point;
    argument[argcount++] = o;
    argument[argcount++] = p;
    argument[argcount++] = o;
    argument[argcount++] = r;

    return fuse_main(argcount, argument, &s_oper);
}
 
