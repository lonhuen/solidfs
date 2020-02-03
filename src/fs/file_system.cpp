#include <iostream>
#include "fs/file_system.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"

FileSystem::FileSystem(bid_t nr_blocks,bid_t nr_iblock_blocks) {
    //TODO(lonhh)
    // this should be actually initilized with a file or disk
    storage = new MemoryStorage(nr_blocks);
    bm = new FreeListBlockManager(storage);
    im = new INodeManager(storage);

    super_block sb;
    sb.nr_block = nr_blocks;
    
    sb.s_iblock = 1;
    sb.nr_iblock = nr_iblock_blocks;
   
    sb.s_dblock = 1 + nr_iblock_blocks;
    sb.nr_dblock = nr_blocks - 1 - nr_iblock_blocks;

    storage->write_block(0,sb.data);
}

void FileSystem::mkfs() {
    //root should be inserted by im->mkfs()
    im->mkfs();
    bm->mkfs();

}

Directory FileSystem::read_directory(iid_t id) {
    // so the functionality could be abstracted to read(iid_t,char* buffer,size,offset)
    // also we'll need write(iid_t,const char* buffer,size,offset)
    // 1. read inode of id
    // 2. read all the blocks of id
    // 3. translate the byte stream to a directory
}

iid_t FileSystem::path2iid(const std::string& path) {
    //suppose that the root inode is 0
    //also suppose that the path would always be "/xxx/xx////xxx"
    iid_t id = 0;
    std::string::size_type p1,p2;
    p2 = path.find('/');
    p1 = 0;
    while(std::string::npos != p2) {
        if((p1 != p2)) {
            Directory dr = read_directory(id);
            // if there not exists
            if(!dr.get_entry(path.substr(p1,p2-p1),&id)) {
                LOG(INFO) << "fail to translate the path " << path;        
            }
        }
        p1 = p2 + 1;
        p2 = path.find('/',p1);
    }
    return id;
}