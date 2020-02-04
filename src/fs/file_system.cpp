#include <iostream>
#include <algorithm>
#include "fs/file_system.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "block/block.h"

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

int FileSystem::read(iid_t id,uint8_t* dst,uint32_t size,uint32_t offset) {
    INode inode;
    im->read_inode(id,inode.data);

    // sanity check
    if(offset > inode.size) {
        LOG(ERROR) << "Trying to access offset outside the file";
        return 0;
    } else if (offset + size > inode.size) {
        LOG(WARNING) << "Trying to access outside the file";
        size = inode.size - offset;
    }
    // the number of blocks to read
    uint32_t nr_blocks = IDIV_BLOCK_SIZE(size);
    if(MOD_BLOCK_SIZE(offset)) {
        nr_blocks++;
    }
    uint32_t s_index = IDIV_BLOCK_SIZE(offset);
    uint32_t e_index = nr_blocks + s_index;
    std::vector<bid_t> blockid_arrays = read_dblock_indexs(inode,s_index,e_index);

    // the total number of bytes
    uint32_t s = 0;
    // read [s_addr,s_addr+nr_bytes) in the block
    uint32_t s_addr = MOD_BLOCK_SIZE(offset);
    uint32_t nr_bytes = std::min(size - s,512u);
    for(auto p=blockid_arrays.begin();p!=blockid_arrays.end();p++) {
        Block bl;
        bm->read_dblock(*p,bl.data);
        std::memcpy(dst+s,bl.data+s_addr,nr_bytes);

        //update the s_addr and nr_bytes
        s = s + nr_bytes;
        uint32_t s_addr = MOD_BLOCK_SIZE(offset + s);
        uint32_t nr_bytes = std::min(size - s,512u);
    }
    return s;
}

// read [begin,end) entries
std::vector<bid_t> FileSystem::read_dblock_indexs(INode& inode,uint32_t begin,uint32_t end) {
    std::vector<bid_t> ret;
    ret.reserve(end - begin);

    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);

    auto p = begin;
    while (p < end) {
        // direct look up
        if (p < 10) {
            bid_t id = inode.p_block[p];
            ret.push_back(id);
            p++;
        // indrect (one-level)
        } else if (p < 10 + factor) {
            Block bl;
            bm->read_dblock(10,bl.data);
            for(;(p < 10 + factor) && (p < end);p++){
                ret.push_back(bl.bl_entry[p - 10]);
            }
        } else if (p < 10 + factor + factor * factor) {
            Block bl_1;
            bm->read_dblock(11,bl_1.data);
            for(uint32_t i=0;i<factor && p<end;i++) {
                Block bl_2;
                bm->read_dblock(bl_1.bl_entry[i],bl_2.data);
                for(uint32_t j=0;j<factor && p<end;j++,p++){
                    ret.push_back(bl_2.bl_entry[j]);
                }
            }
        } else if (p < 10 + factor + factor * factor + factor * factor * factor){
            Block bl_1;
            bm->read_dblock(11,bl_1.data);
            for(uint32_t i=0;i<factor && p<end;i++) {
                Block bl_2;
                bm->read_dblock(bl_1.bl_entry[i],bl_2.data);
                for(uint32_t j=0;j<factor && p<end;j++,p++){
                    Block bl_3;
                    bm->read_dblock(bl_2.bl_entry[j],bl_3.data);
                    for(uint32_t k=0;k<factor && p<end;k++,p++) {
                        ret.push_back(bl_3.bl_entry[k]);
                    }
                }
            }

        } else {
            LOG(ERROR) << "Trying to access maxmium file size";
        }

    }
    return ret;
}