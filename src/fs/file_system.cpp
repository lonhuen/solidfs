#include <iostream>
#include <algorithm>
#include "fs/file_system.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "block/block.h"
#include "directory/directory.h"

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
    // TODO(lonhh): whether we need to optimize the one time read_inode here?
    INode inode;
    im->read_inode(id,inode.data);
    uint8_t* buffer = new uint8_t[inode.size];

    read(id,buffer,inode.size,0);

    Directory dr(id,buffer,inode.size);
    delete buffer;
    return dr;
}

int FileSystem::path2iid(const std::string& path,iid_t* id) {
    //suppose that the root inode is 0
    //also suppose that the path would always be "/xxx/xx////xxx"
    //how to deal with errors
    *id = 0;
    std::string::size_type p1,p2;
    p2 = path.find('/');
    p1 = 0;
    while(std::string::npos != p2) {
        if((p1 != p2)) {
            Directory dr = read_directory(*id);
            // if there not exists
            if(!dr.get_entry(path.substr(p1,p2-p1),id)) {
                LOG(INFO) << "fail to translate the path " << path;
                return 0;
            }
        }
        p1 = p2 + 1;
        p2 = path.find('/',p1);
    }
    return 1;
}

int FileSystem::read(iid_t id,uint8_t* dst,uint32_t size,uint32_t offset) {
    INode inode;
    im->read_inode(id,inode.data);

    // sanity check
    if(offset >= inode.size) {
        LOG(ERROR) << "Trying to access offset outside the file";
        return 0;
    } else if (offset + size >= inode.size) {
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
    std::vector<bid_t> blockid_arrays = read_dblock_index(inode,s_index,e_index);

    // the total number of bytes
    uint32_t s = 0;
    // read [s_addr,s_addr+nr_bytes) in the block
    uint32_t s_addr = MOD_BLOCK_SIZE(offset);
    uint32_t nr_bytes = std::min(BLOCK_SIZE - s_addr, size - s);
    for(auto p=blockid_arrays.begin();p!=blockid_arrays.end();p++) {
        Block bl;
        bm->read_dblock(*p,bl.data);
        std::memcpy(dst+s,bl.data+s_addr,nr_bytes);

        //update the s_addr and nr_bytes
        s = s + nr_bytes;
        s_addr = MOD_BLOCK_SIZE(offset + s);
        nr_bytes = std::min(BLOCK_SIZE - s_addr, size - s);
    }
    return s;
}

// read [begin,end) entries
std::vector<bid_t> FileSystem::read_dblock_index(INode& inode,uint32_t begin,uint32_t end) {
    std::vector<bid_t> ret;
    ret.reserve(end - begin);

    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);

    auto p = begin;
    while (p < end) {
        // direct look up
        if (p < 10) {
            p += block_lookup_per_region(inode,p,end,ret,0);
        // indrect (one-level)
        } else if (p < 10 + factor) {
            p += block_lookup_per_region(inode,p - 10,end - 10,ret,1);
        } else if (p < 10 + factor + factor * factor) {
            p += block_lookup_per_region(inode,p - 10 - factor,end - 10 - factor,ret,2);
        } else if (p < 10 + factor + factor * factor + factor * factor * factor){
            p += block_lookup_per_region(inode,p - 10 - factor - factor * factor,
                                            end - 10 - factor - factor*factor,ret,3);
        } else {
            LOG(ERROR) << "Trying to access maxmium file size";
        }
    }
    return ret;
}

// note the "begin" here is in term of the start of the region    
uint32_t FileSystem::block_lookup_per_region(INode& inode,uint32_t begin,uint32_t end,std::vector<bid_t>& vec,int depth) {
    // TODO(lonhh): sanity check
    if(begin >= end) {
        return 0;
    }

    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);
    int ret = 0;
    // direct look up
    // [begin, end) is subset of [0,10)
    if(depth == 0) {
        for(uint32_t i=begin; begin < end && i < 10;i++, begin++) {
            vec.push_back(inode.p_block[i]);
            ret++;
            LOG(INFO) << "reading " << i << " @depth=0 " << inode.p_block[i];
        }
    // note here [begin, end) in [0,512)
    } else if (depth == 1) {
        Block bl;
        bm->read_dblock(inode.p_block[10],bl.data);
        for(uint32_t i=begin; begin < end && i< factor ;i++, begin++){
            vec.push_back(bl.bl_entry[i]);
            ret++;
            LOG(INFO) << "reading " << i << " @depth=1";
        }
    // note here [begin, end) in [0,512 * 512)
    } else if (depth == 2) {
        Block bl_1;
        bm->read_dblock(inode.p_block[11],bl_1.data);
        auto si = begin / factor;
        for(uint32_t i=si; i < factor && begin < end;i++){
            Block bl_2;
            bm->read_dblock(bl_1.bl_entry[i],bl_2.data);
            
            auto sj = begin % factor;
            for(uint32_t j=sj; j < factor && begin < end;j++, begin++){
                vec.push_back(bl_2.bl_entry[j]);
                ret++;
                LOG(INFO) << "reading " << i << " " << j << " @depth=2";
            }
        }
    // note here [begin, end) in [0,512 * 512)
    } else {
        Block bl_1;
        bm->read_dblock(inode.p_block[12],bl_1.data);
        auto si = begin / factor / factor;
        for(uint32_t i=si; i < factor && begin < end;i++){
            Block bl_2;
            bm->read_dblock(bl_1.bl_entry[i],bl_2.data);
            
            auto sj = (begin / factor ) % factor;
            for(uint32_t j=sj; j < factor && begin < end;j++){
                Block bl_3;
                bm->read_dblock(bl_2.bl_entry[j],bl_3.data);
            
            
                auto sk = begin % factor ;
                for(uint32_t k=sk; k < factor && begin < end;k++,begin++){
                    vec.push_back(bl_3.bl_entry[k]);
                    ret++;
                    LOG(INFO) << "reading " << i << " " << j << " " << k << " @depth=3";
                }
            }
        }
    }
    return ret;
}

bid_t FileSystem::new_dblock(INode& inode) {
    //TODO(lonhh) : do we need to check maximum file size or maximum # of blocks
    //TODO(lonhh) : we need to initialize the data block for mapping?
    // the new one should be in the direct entries
    // we should make sure atomic operation
    // maybe we can optimize by providing atomic api
    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);
    if(inode.block < 10) {
        auto ret = bm->allocate_dblock();
        if(!ret) {
            LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
            return 0;
        }
        inode.p_block[inode.block] = ret;
        inode.block++;
        im->write_inode(inode.inode_number,inode.data);
        return ret;
    // 1-level indirect
    } else if (inode.block < 10 + factor) {
        // allocate the mapping block if we have not allocated it before
        if(inode.block == 10 && inode.p_block[10] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            inode.p_block[10] = ret;
        }
        auto ret = bm->allocate_dblock();
        if(!ret) {
            // maybe we need a dirty bit here
            im->write_inode(inode.inode_number,inode.data);
            LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
            return 0;
        }
        Block bl;
        bm->read_dblock(inode.p_block[10],bl.data);
        bl.bl_entry[inode.block - 10] = ret;
        bm->write_dblock(inode.p_block[10],bl.data);

        inode.block++;
        im->write_inode(inode.inode_number,inode.data);
        return ret;
    // 2-level indirect
    } else if (inode.block < 10 + factor + factor * factor) {
        auto level_index = inode.block - 10 - factor;
        // allocate the first level mapping block if level_index = 0
        // allocate the second level mapping block if level_index % 512 == 0
        if(level_index == 0 && inode.p_block[11] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            inode.p_block[11] = ret;
        }

        Block lv1_bl;
        bm->read_dblock(inode.p_block[11],lv1_bl.data); 

        // allocate the second level mapping block
        if(level_index % factor== 0 && lv1_bl.bl_entry[level_index/factor] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                // maybe we need a dirty bit here
                im->write_inode(inode.inode_number,inode.data);
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            lv1_bl.bl_entry[level_index/factor] = ret;
            bm->write_dblock(inode.p_block[11],lv1_bl.data); 
        }
        
        Block lv2_bl;
        bm->read_dblock(lv1_bl.bl_entry[level_index/factor],lv2_bl.data); 
        
        // allocate the real data block
        auto ret = bm->allocate_dblock();
        if(!ret) {
            // maybe we need a dirty bit here
            im->write_inode(inode.inode_number,inode.data);
            LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
            return 0;
        }
        lv2_bl.bl_entry[level_index%factor] = ret;
        bm->write_dblock(lv1_bl.bl_entry[level_index/factor],lv2_bl.data); 

        inode.block++;
        im->write_inode(inode.inode_number,inode.data);
        return ret;
    } else {
        auto level_index = inode.block - 10 - factor - factor * factor * factor;
        // allocate the first level mapping block if level_index = 0
        // allocate the second level mapping block if level_index % (512 * 512) == 0
        // allocate the third level mapping block if level_index % (512) == 0
        if(level_index == 0 && inode.p_block[12] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            inode.p_block[12] = ret;
        }

        Block lv1_bl;
        bm->read_dblock(inode.p_block[12],lv1_bl.data); 

        // allocate the second level mapping block
        if(level_index % (factor * factor) == 0 && lv1_bl.bl_entry[level_index/(factor*factor)] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                // maybe we need a dirty bit here
                im->write_inode(inode.inode_number,inode.data);
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            lv1_bl.bl_entry[level_index/(factor*factor)] = ret;
            bm->write_dblock(inode.p_block[12],lv1_bl.data); 
        }
        
        Block lv2_bl;
        bm->read_dblock(lv1_bl.bl_entry[level_index/(factor*factor)],lv2_bl.data); 
        
        // allocate the third level mapping 
        if(level_index % factor  == 0 && lv2_bl.bl_entry[level_index/factor] == 0) {
            auto ret = bm->allocate_dblock();
            if(!ret) {
                // maybe we need a dirty bit here
                im->write_inode(inode.inode_number,inode.data);
                LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
                return 0;
            }
            lv2_bl.bl_entry[level_index/factor] = ret;
            bm->write_dblock(lv1_bl.bl_entry[level_index/(factor*factor)],lv2_bl.data); 
        }
       
        Block lv3_bl;
        bm->read_dblock(lv2_bl.bl_entry[level_index/factor],lv3_bl.data); 

        // allocate the real data block
        auto ret = bm->allocate_dblock();
        if(!ret) {
            // maybe we need a dirty bit here
            im->write_inode(inode.inode_number,inode.data);
            LOG(ERROR) << "failing allocating a data block for inode " << inode.inode_number;
            return 0;
        }
        lv3_bl.bl_entry[level_index%factor] = ret;
        bm->write_dblock(lv2_bl.bl_entry[level_index/factor],lv3_bl.data); 

        inode.block++;
        im->write_inode(inode.inode_number,inode.data);
        return ret;

    }
}