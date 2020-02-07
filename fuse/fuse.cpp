#define FUSE_USE_VERSION 35
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <algorithm>

#include "fs/file_system.h"
#include "directory/directory.h"

FileSystem *fs;

extern "C" {

    //void*s_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    //    return NULL;
    //}

    int s_getattr(const char* path, struct stat* st, struct fuse_file_info *fi) {
        iid_t id;
        int p_res = fs->path2iid((const std::string)path, &id);
        if (p_res == 0) {  // inode not found
            LOG(ERROR) << "Cannot open file " << path;
            return -1;
        }
		INode inode;
		fs->im->read_inode(id,inode.data);

		st->st_ino     = id;
		st->st_mode    = inode.mode;
		st->st_nlink   = inode.links;
		st->st_uid     = inode.uid;
		st->st_gid     = inode.gid;
		st->st_size    = inode.size;
		st->st_blocks  = inode.block;
		st->st_atime   = inode.atime;
		st->st_ctime   = inode.ctime;
		st->st_mtime   = inode.mtime;
		st->st_blksize = BLOCK_SIZE;
		// ingore this
		//fi->st_dev     = inode.dev;
		if (inode.itype != inode_type::DIRECTORY) {
			st->st_mode = st->st_mode | S_IFREG;
		} else if (inode.itype == inode_type::DIRECTORY) {
			st->st_mode = st->st_mode | S_IFDIR;
		} else {
			LOG(ERROR) << "Unkown file type for " << path;
        }
		return 0;
	}

    /*
    // write readdir below
    int s_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
            struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
        iid_t id;
		if(fs->path2iid(path,&id)) {
			LOG(ERROR) << "no such file " << path;
			return 0;
		}
		Directory dr = fs->read_directory(id);
		for_each()
        filler(buf, ".", NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
        filler(buf, "..", NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
        filler(buf, "test", NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
    }
*/

    int s_open(const char* path, struct fuse_file_info* fi) {
        iid_t id;
        int p_res = fs->path2iid((const std::string)path, &id);
        if (p_res == 0) {  // inode not found
            LOG(ERROR) << "Cannot open file " << path;
            return -1;
        }
        fi->fh = id;    // cache inode id
        return 0;
    }

    int s_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        int res = s_open(path, fi);
        if (res == 0) {
            LOG(ERROR) << "Cannot find file to read " << path;
            return -1;
        }

        iid_t id = (iid_t) fi->fh;
        /*
        INode inode;
        fs->im->read_inode(id, inode.data);

        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {
            LOG(ERROR) << "File is a directory " << path;
            return 0
        }
        */
    
        return fs->read(id, (uint8_t *)buf, (uint32_t)size,(uint32_t)offset);
    }

    int s_write(const char *path, const char* buf, size_t size, off_t offset,struct fuse_file_info *fi) {
        int res = s_open(path, fi);
        if (res == 0) {
            LOG(ERROR) << "Cannot find file to read " << path;
            return -1;
        }

        iid_t id = (iid_t) fi->fh;
        /*                                                                         
        INode inode;
        fs->im->read_inode(id, inode.data);
        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {                                        
            LOG(ERROR) << "File is a directory " << path;                          
            return 0                                                               
        }                                                                          
        */
    
        return fs->write(id, (const uint8_t *)buf,
                         (uint32_t) size, (uint32_t) offset);
    }

    int s_truncate(const char *path, off_t offset, struct fuse_file_info *fi) {
        int res = s_open(path, fi);
        if (res == 0) {
            LOG(ERROR) << "Cannot find file to truncate " << path;
            return -1;
        }

        /*                                                                         
        INode inode;
        fs->im->read_inode(id, inode.data);
        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {                                        
            LOG(ERROR) << "File is not a directory " << path;                          
            return 0                                                               
        }                                                                          
        */

        iid_t id = (iid_t) fi->fh;
        return fs->truncate(id, (uint32_t) offset)-1; // return 0 if success   
    }

    int s_unlink(const char *path) {
        iid_t id;
        int p_res = fs->path2iid((const std::string)path, &id);
        if (p_res == 0) {  // inode not found
            LOG(ERROR) << "Cannot find file to unlink " << path;
            return -1;
        }
        return fs->unlink(id);
    }

    int s_readdir(const char * path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi, 
                  enum fuse_readdir_flags flags) {
        (void) flags;
        (void) offset;

        int res = s_open(path, fi);
        if (res == 0) {
            LOG(ERROR) << "Cannot read file " << path;
            return -1;
        }

        iid_t id = (iid_t) fi->fh;
        
        /*                                                                         
        INode inode;
        fs->im->read_inode(id, inode.data);
        // check if inode is a directory
        if (inode.data.mode != DIRECTORY) {                                        
            LOG(ERROR) << "File is not a directory " << path;                          
            return 0                                                               
        }                                                                          
        */

        Directory dir = fs->read_directory(id);
        for (auto entry : dir.entry_m) {
            struct stat st;
            memset(&st, 0, sizeof(st));
            st.st_ino = entry.second;

            res = filler(buf, entry.first.c_str(), &st, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
            if (res != 0) {
                return res;
            }
        }
        return 0;
    }

    int mknod(const char *path, mode_t mode, dev_t dev) {
        if (!S_INSREG(mode)) {
            return -ENOTSUP;
        }

        // parse path
        int idx = ((std::string) path).rfind('/');
        std::string dir_path = ((std::string) path).substr(0, i);
        std::string f_path = ((std::string) path).substr(i+1, s.length()-i-1);

        // get dir
        iid_t dir_id;
        int res = path2iid(dir_path, &dir_id);
        if (res == 0) {
            LOG(ERROR) << "Cannot open directory path for mknod " << dir_path;
            return -1;
        }
        Directory dir = fs->read_directory(dir_id);
        
        // get file
        iid_t f_id;
        res = path2iid(path, &f_id);
        if (res == 0) {
            LOG(ERROR) << "Cannot open file path for mknod " << path;
            return -1;
        } 
        // update inode metadata
        INode inode;
        fs->im->read_inode(id, inode.data);
        inode.data.mode = (uint32_t) mode;
        // also should update device       
            
        return fs->write_directory(dir_id, dir);
    }


/*
    // list of file system function
    struct fuse_operations s_oper = {
        // .init = s_init,
        .getattr = s_getattr,
        .open = s_open,
        .read = s_read,
        .write = s_write,
        // .lseek = s_lseek,
        .truncate = s_truncate,
        .unlink = s_unlink,
        // .opendir = s_opendir,
        // .closedir = s_closedir,
        // .mkdir = s_mkdir,
        .readdir = s_readdir,
        // .rmdir = s_rmdir,
    };	
*/
}
  
int main(int argc, char *argv[]) {
    // TODO: change this later to customize size based on argv
    fs = new FileSystem(10 + 512 + 512 * 512, 9);
    fs->mkfs();

    fuse_operations s_oper;
    memset(&s_oper, 0, sizeof(s_oper));

    // s_oper.init = s_init;
    s_oper.getattr = s_getattr;
    s_oper.open = s_open;
    s_oper.read = s_read;
    s_oper.write = s_write;
    s_oper.truncate = s_truncate;    
    s_oper.unlink = s_unlink;
    s_oper.readdir= s_readdir;

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

    return fuse_main(argcount, argument, &s_oper,0);
}
 
