#define FUSE_USE_VERSION 39
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
#include <functional>

#include "fs/file_system.h"
#include "inode/inode.h"
#include "directory/directory.h"
#include "utils/fs_exception.h"
#include "utils/string_utils.h"


using namespace solid;
FileSystem *fs;


inline int unwrap(std::function<int(void)> f) {
    try {
        return f();
    } catch (const fs_exception& e) {
        LOG(INFO) << e.what() << " " << e.code().value();
        return -e.code().value();
    } catch (const fs_error& e) {
        throw;
    }
};

// TODO(lonhh) maybe we need to optimize the functions by using fuse_fiel_info
extern "C" {

    void*s_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
        LOG(INFO) << "#init";
        fs = new FileSystem(10 + 512 + 512 * 512, 9);
        fs->mkfs();
    }
    
    int s_getattr(const char* path, struct stat* st, struct fuse_file_info *fi) {
        LOG(INFO) << "#getattr " << path;

        return unwrap([&](){
            INodeID id = fs->path2iid(path);
            INode inode = fs->im->read_inode(id);
            if (st != nullptr) {
                // https://libfuse.github.io/doxygen/structfuse__operations.html
                // The 'st_ino' field is ignored except if the 'use_ino' mount option is given. 
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
                st->st_blksize = config::block_size;
                // ingore this
                //fi->st_dev     = inode.dev;
                if (inode.itype == INodeType::REGULAR) {
                    st->st_mode = st->st_mode | S_IFREG;
                } else if (inode.itype == INodeType::DIRECTORY) {
                    st->st_mode = st->st_mode | S_IFDIR;
                } else if(inode.itype == INodeType::SYMLINK){
                    st->st_mode = st->st_mode | S_IFLNK;
                }
            }
            return 0;
        });
	}

    // let the kernel to do the permission check
    // If the -o default_permissions mount option is given,
    // this check is already done by the kernel before calling open() and may thus be omitted by the filesystem.
    int s_open(const char* path, struct fuse_file_info* fi) {
        LOG(INFO) << "#open " << path;
        
        return unwrap([&](){
            INodeID id = fs->path2iid(path);
            // TODO(lonhh): we need to handle with the file handler here
            if(fi != nullptr) {
                fi->fh = config::conv_file_handler(id);
                // ? (lonhh): should we do it here?
                if (fi->flags & O_TRUNC)
                    fs->truncate(id,0);
            }
            return 0;
        });
    }

    int s_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        LOG(INFO) << "#read " << path;
        return unwrap([&](){
            int res = s_open(path, fi);

            if(res < 0) {
                return res;
            }
            INodeID id = (fi == nullptr) ? fs->path2iid(path) : config::rest_file_handler(fi->fh);
            return fs->read(id, (uint8_t *)buf, (uint32_t)size,(uint32_t)offset);
        });
    }

    int s_write(const char *path, const char* buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
        LOG(INFO) << "#write " << path;
        return unwrap([&](){
            int res = s_open(path, fi);
            if (res < 0)
                return res;
            INodeID id = (fi == nullptr) ? fs->path2iid(path) : config::rest_file_handler(fi->fh);
            return fs->write(id, (const uint8_t *)buf,
                            (uint32_t) size, (uint32_t) offset);
        });
    }

    int s_truncate(const char *path, off_t offset, struct fuse_file_info *fi) {
        LOG(INFO) << "#truncate " << path << " " << offset;
        
        return unwrap([&](){
            int res = s_open(path, fi);
            if (res < 0) {
                return res;
            }
            INodeID id = (fi == nullptr) ? fs->path2iid(path) : config::rest_file_handler(fi->fh);
            fs->truncate(id, (uint32_t) offset);
            return 0;
        });
    }

    int s_unlink(const char *path) {
        LOG(INFO) << "#unlink " << path;

        return unwrap([&](){
            std::string p(path);
            std::string dir_name = fs->directory_name(p);
            std::string f_name = fs->file_name(p);

            INodeID dir_id;
            dir_id = fs->path2iid(dir_name);

            Directory dr = fs->read_directory(dir_id);

            INodeID f_id = dr.get_entry(f_name);
            dr.remove_entry(f_name);
            fs->write_directory(dir_id,dr);

            fs->unlink(f_id);
            return 0;
        });
    }
    
    int s_readdir(const char * path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi, 
                  enum fuse_readdir_flags flags) {

        LOG(INFO) << "#readdir " << path;
        return unwrap([&](){
            int res = s_open(path, fi);
            if (res < 0)
                return res;

            INodeID id = (fi == nullptr) ? fs->path2iid(path) : config::rest_file_handler(fi->fh);
            Directory dir = fs->read_directory(id);

            for (auto entry : dir.entry_m) {
                // TODO(lonhh): do we need to get the stat?
                // * actually filler is used to check whether buffer is full
                //struct stat st;
                //memset(&st, 0, sizeof(st));
                //res = filler(buf, entry.first.c_str(), &st, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
                //if(filler(buf, entry.first.c_str(), nullptr, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS))
                //    break;
                filler(buf, entry.first.c_str(), nullptr, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
            }
            return 0;
        });
    }

    int s_mknod(const char *path, mode_t mode, dev_t dev) {
        LOG(INFO) << "#mknod " << path;
        return unwrap([&](){
            // TODO(lonhh): check this! make sure that the error is correct
            //if (!S_INSREG(mode)) {
            //    return -ENOTSUP;
            //}
            std::string p(path);
            std::string dir_name = fs->directory_name(p);
            std::string f_name = fs->file_name(p);

            // get dir
            INodeID dir_id = fs->path2iid(dir_name);
            INode dir_inode = fs->im->read_inode(dir_id);

            //check dir will be done in new_inode
            // allocate a new inode for this file
            INodeID f_id = fs->new_inode(f_name,dir_inode);

            // update inode metadata
            INode inode = INode::get_inode(f_id,INodeType::REGULAR,mode);
            inode.mode = mode;
            // TODO(lonhh)
            //inode.dev
            fs->im->write_inode(f_id,inode);
            return 0;
        });
    }

    int s_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi) {
        LOG(INFO) << "#utime " << path;
        
        return unwrap([&](){
            INodeID id = fs->path2iid(path);
            INode inode = fs->im->read_inode(id);
            inode.atime = ts[0].tv_nsec;
            inode.ctime = ts[1].tv_nsec;
            fs->im->write_inode(id,inode);
            return 0;
        });
    }

    int s_mkdir(const char* path, mode_t mode) {
        LOG(INFO) << "#mkdir " << path;
        
        return unwrap([&](){
            std::string p(path);
            std::string dir_name = fs->directory_name(p);
            std::string f_name = fs->file_name(p);

            INodeID dir_id = fs->path2iid(dir_name);
            INode dir_inode = fs->im->read_inode(dir_id);
            //check dir will be done in new_inode
            // allocate a new inode for this dir 
            INodeID f_id = fs->new_inode(f_name,dir_inode);
            // update inode metadata
            INode inode = INode::get_inode(f_id,INodeType::DIRECTORY,mode);
            fs->im->write_inode(f_id,inode);
            //update directory
            Directory dr(f_id,dir_id);
            fs->write_directory(f_id,dr);
            return 0;
        });
    }

    int s_rmdir(const char *path) {
        LOG(INFO) << "#rmdir " << path;

        return unwrap([&](){
            INodeID id = fs->path2iid(path);
            
            Directory dir = fs->read_directory(id);
            if(dir.entry_m.size() > 2) {
                throw fs_exception(std::errc::directory_not_empty,
                "#rmdir: not a empty ",path);
            }
            return s_unlink(path);
        });
    }

    int s_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
        LOG(INFO) << "#create " << path;
        s_mknod(path,mode,0);
        return s_open(path,fi);
    }

    int s_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
        LOG(INFO) << "#chmod " << path;

        return unwrap([&](){
            INodeID id = fs->path2iid(path);
            INode inode = fs->im->read_inode(id);
            
            inode.ctime = time(nullptr);
            inode.mode = mode;
            return 0;
        });       
    }

    int s_chown(const char *path, uid_t uid, gid_t gid, 
                struct fuse_file_info *fi) {
        LOG(INFO) << "#chown " << path;

        return unwrap([&](){
            INodeID id = fs->path2iid(path);    
            INode inode = fs->im->read_inode(id);   
            
            if (uid != -1) {
                inode.uid = uid;
            }
            if (gid != -1) {
                inode.gid = gid;
            }

            inode.ctime = time(nullptr);
            return 0;
        });
    }

    int s_symlink(const char* src_path, const char* dst_path) {
        LOG(INFO) << "#symlink " << src_path << " <- " << dst_path;

        return unwrap([&](){
            std::string p(dst_path);
            std::string dir_name = fs->directory_name(p);
            std::string f_name = fs->file_name(p);

            // get dir
            INodeID dir_id = fs->path2iid(dir_name);
            INode dir_inode = fs->im->read_inode(dir_id);

            //check dir will be done in new_inode
            // allocate a new inode for this file
            INodeID f_id = fs->new_inode(f_name,dir_inode);

            // update inode metadata
            INode inode = INode::get_inode(f_id,INodeType::SYMLINK,0777);
            // TODO(lonhh)
            //inode.dev
            fs->im->write_inode(f_id,inode);
            fs->write(f_id,(const uint8_t*)src_path,strlen(src_path) + 1,0);
            return 0;
        });

    }

    int s_readlink(const char* path, char* dst, size_t size) {
        LOG(INFO) << "#readlink " << path << " " << size;

        return unwrap([&](){
            s_read(path,dst,size,0,nullptr);
            return 0;
        });
    }

    int s_release(const char* path, struct fuse_file_info* fi) {
        LOG(INFO) << "#release " << path;

        // maybe we can evict all the cached things?
        if(fi != nullptr) {
            fi->fh = config::null_file_handler;
        }
        return 0;
    }

    int s_link(const char* src_path, const char* dst_path) {
        LOG(INFO) << "#link " << src_path << " <- " << dst_path;

        return unwrap([&](){
            // first get the inode that we are going to make hard link
            INodeID src_id = fs->path2iid(src_path);
            INode src_inode = fs->im->read_inode(src_id);

            // here we are sure that the inode exists
            // then try to insert this enty to the directory
            std::string p(dst_path);
            std::string dir_name = fs->directory_name(p);
            std::string f_name = fs->file_name(p);
            INodeID dir_id = fs->path2iid(dir_name);
            Directory dr = fs->read_directory(dir_id);
            // we'll judge whether it contains in the insert function
            dr.insert_entry(f_name,src_id);
            fs->write_directory(dir_id,dr);

            // finally let's update the inode
            src_inode.ctime = time(nullptr);
            src_inode.links += 1;
            fs->im->write_inode(src_id,src_inode);
            return 0;
        });


    }



    /*
    // TODO: add a statfs function to filesystem ?
    int s_statfs(const char *path, struct statvfs *stbuf) {
        LOG(INFO) << "#statfs " << path;

        return unwrap([&](){
            stbuf->f_bsize
            stbuf->f_frsize
            stbuf->f_fsid
            stbuf->f_flag
            stbuf->f_flag
            stbuf->f_namemax
            
            return 0; 
       })
    }

    int s_rename(const char *from, const char *to, unsigned int flag) {
        LOG(INFO) << "#rename " << from << " " << to;
    
        return unwrap([&](){
            std::string to_path(to);
            std::string to_dir = fs->directory_name(to_path);
            std::string to_file = fs->file_name(to_path)

            INodeID id = fs->path2iid(from);
            INode inode = fs->im->read_inode(id);            

            return 0           
        });
    }
    */
    
}
  
int main(int argc, char *argv[]) {
    // TODO: change this later to customize size based on argv

    LogUtils::log_level = "0";
    LogUtils::init(argv[0]);
    //fs = new FileSystem(10 + 512 + 512 * 512, 9);
    //fs->mkfs();

    fuse_operations s_oper;
    memset(&s_oper, 0, sizeof(s_oper));

    s_oper.init = s_init;
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
    s_oper.rmdir = s_rmdir;
    s_oper.create= s_create;
    s_oper.chmod = s_chmod;
    s_oper.chown = s_chown;
    // s_oper.statfs = s_statfs;
    //s_oper.rename = s_rename;
    s_oper.symlink = s_symlink;
    s_oper.readlink = s_readlink;
    s_oper.release = s_release;
    s_oper.link = s_link;
 
    // call s_init here?
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
 
