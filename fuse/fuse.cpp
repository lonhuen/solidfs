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
#include "inode/inode.h"
#include "directory/directory.h"

FileSystem *fs;

extern "C" {

    //void*s_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    //    return NULL;
    //}

    int s_getattr(const char* path, struct stat* st, struct fuse_file_info *fi) {
        LOG(INFO) << "getattr " << path;
        iid_t id;
        int p_res = fs->path2iid(path, &id);
        if (p_res == 0) {  // inode not found
            LOG(ERROR) << "Cannot getattr file " << path;
            return -ENOENT;
        }
		INode inode;
		fs->im->read_inode(id,inode.data);

		st->st_ino     = id;
		st->st_mode    = inode.mode;
        LOG(INFO) << "mode " << inode.mode;
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

    int s_open(const char* path, struct fuse_file_info* fi) {
        LOG(INFO) << "open " << path;
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
        LOG(INFO) << "read " << path;
        int res = s_open(path, fi);
        if (res == -1) {
            LOG(ERROR) << "Cannot find file to read " << path;
            return -1;
        }

        iid_t id;
        if(fi)
            id = fi->fh;
        else
            fs->path2iid(path,&id);
        /*
        INode inode;
        fs->im->read_inode(id, inode.data);

        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {
            LOG(ERROR) << "File is a directory " << path;
            return 0
        }
        */
    
        LOG(INFO) << "read " << path;
        return fs->read(id, (uint8_t *)buf, (uint32_t)size,(uint32_t)offset);
    }

    int s_write(const char *path, const char* buf, size_t size, off_t offset,struct fuse_file_info *fi) {
        LOG(INFO) << "write " << path;
        int res = s_open(path, fi);
        if (res == -1) {
            LOG(ERROR) << "Cannot find file to read " << path;
            return -1;
        }

        iid_t id;
        if(fi)
            id = fi->fh;
        else
            fs->path2iid(path,&id);
        /*                                                                         
        INode inode;
        fs->im->read_inode(id, inode.data);
        // check if inode is a directory
        if (inode.data.mode == DIRECTORY) {                                        
            LOG(ERROR) << "File is a directory " << path;                          
            return 0                                                               
        }                                                                          
        */
        LOG(INFO) << "write " << buf << " to file " << path; 
        return fs->write(id, (const uint8_t *)buf,
                         (uint32_t) size, (uint32_t) offset);
    }

    int s_truncate(const char *path, off_t offset, struct fuse_file_info *fi) {
        LOG(INFO) << "truncate " << path;
        int res = s_open(path, fi);
        if (res == -1) {
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
        iid_t id;
        if(fi)
            id = fi->fh;
        else
            fs->path2iid(path,&id);

        return fs->truncate(id, (uint32_t) offset)-1; // return 0 if success   
    }

    int s_unlink(const char *path) {
        LOG(INFO) << "unlink " << path;
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
        LOG(INFO) << "readdir " << path;
        int res = s_open(path, fi);
        if (res == -1) {
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
            //struct stat st;
            //memset(&st, 0, sizeof(st));

            //res = filler(buf, entry.first.c_str(), &st, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
            res = filler(buf, entry.first.c_str(), NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
            if (res != 0) {
                return res;
            }
        }
        return 0;
    }

    int s_mknod(const char *path, mode_t mode, dev_t dev) {
        LOG(INFO) << "mknod " << path;
        //if (!S_INSREG(mode)) {
        //    return -ENOTSUP;
        //}

        // parse path
        std::string p(path);
        std::size_t i = p.rfind('/');
        std::string dir_path;
        if(i == 0) {
            dir_path = p.substr(0, 1);
        } else {
            dir_path = p.substr(0, i);
        }
        std::string f_path = p.substr(i+1, p.length()-i-1);

        // get dir
        iid_t dir_id;
        LOG(INFO) << "mknode dir path " << dir_path << " fpath " << f_path;
        int res = fs->path2iid(dir_path, &dir_id);
        if (res == 0) {
            LOG(ERROR) << "Cannot open directory path for mknod " << dir_path;
            return -1;
        }

        INode dir_inode;
        fs->im->read_inode(dir_id,dir_inode.data);

        // allocate a new inode for this file
        iid_t f_id = fs->new_inode(f_path,dir_inode);
        if (f_id == 0) {
            LOG(ERROR) << "Cannot allocate a new inode " << path;
            return -1;
        } 
        // update inode metadata
        INode inode = INode::get_inode(f_id,inode_type::REGULAR);
        inode.mode = mode;
        // dev??
        //inode.dev
        fs->im->write_inode(f_id,inode.data);
        return 0;
    }
    int s_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi) {
        LOG(INFO) << "utime " << path;
        std::string p(path);
        iid_t id;
        int res = fs->path2iid(p, &id);
        if (res == 0) {
            return -1;
        }
        INode inode;
        fs->im->read_inode(id,inode.data);
        inode.atime = ts[0].tv_nsec;
        inode.ctime = ts[1].tv_nsec;
        fs->im->write_inode(id,inode.data);
        return 0;
    }

    int s_mkdir(const char* path, mode_t mode) {
        LOG(INFO) << "mkdir " << path;
        std::string s(path);
        std::size_t i = s.rfind('/');
        std::string dirname;
        if(i == 0) {
            dirname = s.substr(0, 1);
        } else {
            dirname = s.substr(0, i);
        }
        std::string filename(s,i+1,s.length()-i-1);
        LOG(INFO) << "dirname " << dirname << " filename " << filename;

                // get dir
        iid_t dir_id;
        int res = fs->path2iid(dirname, &dir_id);
        if (res == 0) {
            LOG(ERROR) << "Cannot open directory path for mkdir " << dirname;
            return -1;
        }

        INode dir_inode;
        fs->im->read_inode(dir_id,dir_inode.data);

        // allocate a new inode for this dir 
        iid_t f_id = fs->new_inode(filename,dir_inode);
        if (f_id == 0) {
            LOG(ERROR) << "Cannot allocate a new inode " << path;
            return -1;
        } 
        // update inode metadata
        INode inode = INode::get_inode(f_id,inode_type::DIRECTORY);
        inode.mode = mode;
        // dev??
        //inode.dev
        fs->im->write_inode(f_id,inode.data);
        return 0;
    }
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
    s_oper.utimens= s_utimens;
    s_oper.mkdir = s_mkdir;
    s_oper.mknod = s_mknod;

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
 
