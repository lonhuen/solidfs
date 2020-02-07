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

    // init inode for the root
    INode inode = INode::get_inode(0,inode_type::DIRECTORY);
    bid_t b = bm->allocate_dblock();
    inode.p_block[0] = b;
    inode.block++;

    // init the data block
    Block bl;
    Directory dr(0,0);
    inode.size = dr.serialize(bl.data,BLOCK_SIZE);
    /*
    std::cout << "mkfs" << std::endl;
    for(auto i=0;i<13;i++) {
        std::cout << std::hex << (uint32_t)bl.data[i] << " ";
    }
    std::cout << std::endl;
    */
    bm->write_dblock(b,bl.data);
    im->write_inode(0,inode.data);
}

std::vector<std::string> FileSystem::parse_path(const std::string& path){
    // taken from https://stackoverflow.com/questions/909289/splitting-a-string
    typedef std::string::const_iterator iter;
    iter beg = path.begin();
    std::vector<std::string> tokens;

    while(beg != path.end()) {
        //cout << ":" << beg._Myptr << ":" << endl;
        iter temp = std::find(beg, path.end(), '/');
        if(beg != path.end())
            tokens.push_back(std::string(beg, temp));
        beg = temp;
        while ((beg != path.end()) && (*beg == '/'))
            beg++;
    }
    return tokens;
}
int FileSystem::path2iid(const std::string& path,iid_t* id) {
    //suppose that the root inode is 0
    //also suppose that the path would always be "/xxx/xx////xxx"
    //how to deal with errors
    *id = 0;
    if(path=="/") {
        return 1;
    }
    std::vector<std::string> v=parse_path(path);
    for(auto p=v.begin()+1;p!=v.end();p++) {
        Directory dr = read_directory(*id);
        if(!dr.get_entry(*p,id)) {
            LOG(INFO) << "fail to translate the path " << path;
            return 0;
        }
    }
    return 1;
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
    uint32_t s_index = IDIV_BLOCK_SIZE(offset);
    uint32_t e_index = (MOD_BLOCK_SIZE(offset+size) == 0) ? IDIV_BLOCK_SIZE(offset+size) : IDIV_BLOCK_SIZE(offset+size) + 1;
    uint32_t nr_blocks = e_index - s_index;
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

int FileSystem::write(iid_t id,const uint8_t* src,uint32_t size,uint32_t offset) {
    // first judge whether we need to allocate more blocks
    LOG(INFO) << "writing " << src;
    // TODO(lonhh) whether we can batch the allocating??
    INode inode;
    im->read_inode(id,inode.data);
    std::vector<bid_t> allocated_blocks;
    if(offset + size > inode.block * BLOCK_SIZE) {
        // the last block index [)
        uint32_t nr_allocate_blocks = (MOD_BLOCK_SIZE(offset+size) == 0) ? IDIV_BLOCK_SIZE(offset+size) : IDIV_BLOCK_SIZE(offset+size) + 1;
        nr_allocate_blocks = nr_allocate_blocks - inode.block;
        allocated_blocks.reserve(nr_allocate_blocks);
        for(auto i=0;i<nr_allocate_blocks;i++){
            allocated_blocks.push_back(new_dblock(inode));
        }
    }

    auto flag = 0;
    std::for_each(allocated_blocks.begin(),allocated_blocks.end(),[&](auto p){
        if(p == 0) flag++;
    });

    // TODO(lonhh) if we don't get enough blocks, just discard?
    if(flag) {
        LOG(ERROR) << "Not getting enough blocks for writing file " << inode.inode_number;
        // free all the allocated ones
        std::for_each(allocated_blocks.begin(),allocated_blocks.end(),[&](auto p){
            if(p != 0) bm->free_dblock(p);
        });
        return 0;
    }

    // just get all the previous allocated blocks (before write function)

    // the number of blocks to write
    uint32_t s_index = IDIV_BLOCK_SIZE(offset);
    uint32_t e_index = (MOD_BLOCK_SIZE(offset+size) == 0) ? IDIV_BLOCK_SIZE(offset+size) : IDIV_BLOCK_SIZE(offset+size) + 1;
    uint32_t nr_blocks = e_index - s_index;
    
    // don't count the just-allocated blocks
    std::vector<bid_t> blockid_arrays = read_dblock_index(inode,s_index,e_index);
    blockid_arrays.reserve(blockid_arrays.size() + std::distance(allocated_blocks.begin(),allocated_blocks.end()));
    blockid_arrays.insert(blockid_arrays.end(),allocated_blocks.begin(),allocated_blocks.end());

    // the total number of bytes
    uint32_t s = 0;
    // read [s_addr,s_addr+nr_bytes) in the block
    uint32_t s_addr = MOD_BLOCK_SIZE(offset);
    uint32_t nr_bytes = std::min(BLOCK_SIZE - s_addr, size - s);
    for(auto p=blockid_arrays.begin();p!=blockid_arrays.end();p++) {
        Block bl;
        bm->read_dblock(*p,bl.data);
        std::memcpy(bl.data+s_addr,src+s,nr_bytes);
        bm->write_dblock(*p,bl.data);

        //update the s_addr and nr_bytes
        s = s + nr_bytes;
        s_addr = MOD_BLOCK_SIZE(offset + s);
        nr_bytes = std::min(BLOCK_SIZE - s_addr, size - s);
    }
    inode.size = std::max(inode.size,(uint64_t)offset+size);
    im->write_inode(id,inode.data);
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
    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);

    bid_t allocate_block_array[4];
    // 4 types, index-1, index-2, index-3
    uint32_t index_array[4];
    uint32_t flag_array[4];

    // let's first do the translation
    if(inode.block < 10) {
        // the first type -- direct lookup
        index_array[0] = 0;
        index_array[1] = inode.block;
    } else if (inode.block < 10 + factor) {
        auto tmp = inode.block - 10;
        // the second type -- 1-indirect lookup
        index_array[0] = 1;
        index_array[1] = tmp % factor;
    } else if (inode.block < 10 + factor + factor * factor) {
        auto tmp = inode.block - 10 - factor;
        // the second type -- 2-indirect lookup
        index_array[0] = 2;
        index_array[1] = tmp / factor;
        index_array[2] = tmp % factor;
    } else {
        auto tmp = inode.block - 10 - factor - factor * factor;
        // the second type -- 3-indirect lookup
        index_array[0] = 3;
        index_array[1] = tmp / factor / factor;
        index_array[2] = (tmp / factor) % factor;
        index_array[3] = tmp  % factor;
    }

    // let's allocate the data block for look up then
    // number of mapping blocks to allocate
    // actually we only need to count how many datablocks we need to allocate

    // TODO(lonhh) might optimize here
    for(auto i=1;i<=index_array[0];i++) {
        flag_array[i] = index_array[i];
        for(auto j=i+1;j<=index_array[0];j++) {
            flag_array[i] += index_array[j];
        }
    }


    uint32_t nr_mblock = 0;
    for(auto i=1;i<=index_array[0];i++) {
        nr_mblock += (flag_array[i] == 0 ? 1 : 0);
    }

    // try to allocate nr_mblock + 1 blocks
    auto flag = 0;
    for(auto i=0;i<nr_mblock+1;i++) {
        allocate_block_array[i] = bm->allocate_dblock();
        flag += (allocate_block_array[i] != 0 ? 1 : 0);
    }

    // we don't get enough blocks. Just free all of them
    if (flag != nr_mblock + 1) {
        for(auto i=0;i<nr_mblock+1;i++) {
            if(allocate_block_array[i]) {
                bm->free_dblock(allocate_block_array[i]);
            }
        }
        return 0;
    }

    // now we have allocated all the datablocks. Let's update
    // 1. the mapping
    // 2. the block field of inode
    inode.block++;
    if(index_array[0] == 0) {
        inode.p_block[index_array[1]] = allocate_block_array[nr_mblock];
    } else if (index_array[0] == 1) {
        // update the inode
        auto i_mblock = 0;
        if(flag_array[1] == 0) {
            inode.p_block[10] = allocate_block_array[i_mblock];
            i_mblock++;
        }
        // update the mapping block
        Block bl;
        bm->read_dblock(inode.p_block[10],bl.data);
        bl.bl_entry[index_array[1]] = allocate_block_array[nr_mblock];
        bm->write_dblock(inode.p_block[10],bl.data);

    } else if (index_array[0] == 2) {
        // try to read inode.p_block[12] as the first level mapping bl1
        auto i_mblock = 0;
        if(flag_array[1] == 0) {
            inode.p_block[11] = allocate_block_array[i_mblock];
            i_mblock++;
        }
        Block bl1;
        bm->read_dblock(inode.p_block[11],bl1.data);

        // try to read bl1 entry[index_array[1]] as the second level mapping bl2
        if(flag_array[2] == 0) {
            bl1.bl_entry[index_array[1]] = allocate_block_array[i_mblock];
            bm->write_dblock(inode.p_block[11],bl1.data);
            i_mblock++;
        }
        Block bl2;
        bm->read_dblock(bl1.bl_entry[index_array[1]],bl2.data);
        // allocate the data block
        bl2.bl_entry[index_array[2]] = allocate_block_array[nr_mblock];
        bm->write_dblock(bl1.bl_entry[index_array[1]],bl2.data);
    } else {
        // try to read inode.p_block[13] as the first level mapping bl1
        auto i_mblock = 0;
        if(flag_array[1] == 0) {
            inode.p_block[12] = allocate_block_array[i_mblock];
            i_mblock++;
        }
        Block bl1;
        bm->read_dblock(inode.p_block[12],bl1.data);

        // try to read bl1 entry[index_array[1]] as the second level mapping bl2
        if(flag_array[2] == 0) {
            bl1.bl_entry[index_array[1]] = allocate_block_array[i_mblock];
            bm->write_dblock(inode.p_block[12],bl1.data);
            i_mblock++;
        }
        Block bl2;
        bm->read_dblock(bl1.bl_entry[index_array[1]],bl2.data);
        
        // try to read bl2 entry[index_array[2]] as the third level mapping bl3
        if(flag_array[3] == 0) {
            bl2.bl_entry[index_array[2]] = allocate_block_array[i_mblock];
            bm->write_dblock(bl1.bl_entry[index_array[1]],bl2.data);
            i_mblock++;
        }
        
        // allocate the data block
        Block bl3;
        bm->read_dblock(bl2.bl_entry[index_array[2]],bl3.data);
        bl3.bl_entry[index_array[3]] = allocate_block_array[nr_mblock];
        bm->write_dblock(bl2.bl_entry[index_array[2]],bl3.data);
    }
    im->write_inode(inode.inode_number,inode.data);
    return allocate_block_array[nr_mblock];
}

// delete the last block
// we will not maintain the size
int FileSystem::delete_dblock(INode& inode) {
    if(inode.block == 0) {
        LOG(WARNING) << "freeing a non-exist data block for inode " << inode.inode_number;
        return 0;
    }

    int ret = 0;

    const bid_t factor = BLOCK_SIZE/sizeof(bid_t);
    bid_t block_id = inode.block - 1;

    bid_t free_block_array[4];
    // 4 types, index-1, index-2, index-3
    uint32_t index_array[4];
    uint32_t flag_array[4];

    // let's first do the translation
    if(block_id < 10) {
        // the first type -- direct lookup
        index_array[0] = 0;
        index_array[1] = block_id;
    } else if (block_id < 10 + factor) {
        auto tmp = block_id - 10;
        // the second type -- 1-indirect lookup
        index_array[0] = 1;
        index_array[1] = tmp % factor;
    } else if (block_id < 10 + factor + factor * factor) {
        auto tmp = block_id - 10 - factor;
        // the second type -- 2-indirect lookup
        index_array[0] = 2;
        index_array[1] = tmp / factor;
        index_array[2] = tmp % factor;
    } else {
        auto tmp = block_id - 10 - factor - factor * factor;
        // the second type -- 3-indirect lookup
        index_array[0] = 3;
        index_array[1] = tmp / factor / factor;
        index_array[2] = (tmp / factor) % factor;
        index_array[3] = tmp  % factor;
    }

    // let's decide how many blocks we are going to free

    // TODO(lonhh) might optimize here
    for(auto i=1;i<=index_array[0];i++) {
        flag_array[i] = index_array[i];
        for(auto j=i+1;j<=index_array[0];j++) {
            flag_array[i] += index_array[j];
        }
    }

    // # of blocks to be freed 
    uint32_t nr_fblock = 0;
    for(auto i=1;i<=index_array[0];i++) {
        nr_fblock += (flag_array[i] == 0 ? 1 : 0);
    }

    // Let's free the blocks
    // 1. the block field of inode and also the size
    // 2. the mapping

    inode.block--;

    if(index_array[0] == 0) {
        //ret = bm->free_dblock(inode.p_block[index_array[1]]);
        free_block_array[0] = inode.p_block[index_array[1]];
    } else if (index_array[0] == 1) {
        // update the inode
        auto i_fblock = 0;
        if(flag_array[1] == 0) {
            free_block_array[i_fblock] = inode.p_block[10];
            i_fblock++;
        }
        // get the mapping block
        Block bl;
        bm->read_dblock(inode.p_block[10],bl.data);
        free_block_array[i_fblock] = bl.bl_entry[index_array[1]];
    } else if (index_array[0] == 2) {
        // try to read inode.p_block[12] as the first level mapping bl1
        auto i_fblock = 0;
        if(flag_array[1] == 0) {
            free_block_array[i_fblock] = inode.p_block[11];
            i_fblock++;
        }
        Block bl1;
        bm->read_dblock(inode.p_block[11],bl1.data);

        // whether to free the 2-level mapping block
        if(flag_array[2] == 0) {
            free_block_array[i_fblock] = bl1.bl_entry[index_array[1]];
            i_fblock++;
        }
        Block bl2;
        bm->read_dblock(bl1.bl_entry[index_array[1]],bl2.data);
        // allocate the data block
        free_block_array[i_fblock] = bl2.bl_entry[index_array[2]];
    } else {
        // try to read inode.p_block[13] as the first level mapping bl1
        auto i_fblock = 0;
        if(flag_array[1] == 0) {
            free_block_array[i_fblock] = inode.p_block[12];
            i_fblock++;
        }
        Block bl1;
        bm->read_dblock(inode.p_block[12],bl1.data);

        // try to read bl1 entry[index_array[1]] as the second level mapping bl2
        if(flag_array[2] == 0) {
            free_block_array[i_fblock] = bl1.bl_entry[index_array[1]];
            i_fblock++;
        }
        Block bl2;
        bm->read_dblock(bl1.bl_entry[index_array[1]],bl2.data);
        
        // try to read bl2 entry[index_array[2]] as the third level mapping bl3
        if(flag_array[3] == 0) {
            free_block_array[i_fblock] =bl2.bl_entry[index_array[2]];
            i_fblock++;
        }
        
        // allocate the data block
        Block bl3;
        bm->read_dblock(bl2.bl_entry[index_array[2]],bl3.data);
        free_block_array[i_fblock] =bl3.bl_entry[index_array[3]];
    }
    im->write_inode(inode.inode_number,inode.data);

    for(auto i=0;i<=nr_fblock;i++) {
        bm->free_dblock(free_block_array[i]);
    }
    return nr_fblock+1;
}
Directory FileSystem::read_directory(iid_t id) {
    INode inode;
    im->read_inode(id,inode.data);
    return std::move(read_directory(inode));
}
int FileSystem::write_directory(iid_t id,Directory& dr) {
    INode inode;
    im->read_inode(id,inode.data);
    return (write_directory(inode,dr));
}

Directory FileSystem::read_directory(INode& inode) {
    if(!inode.size) {
        LOG(ERROR) << "size = 0 for directory inode " << inode.inode_number;
        return std::move(Directory());
    }
    uint8_t* buffer = new uint8_t[inode.size];
    read(inode.inode_number,buffer,inode.size,0);
    Directory dr(inode.inode_number,buffer,inode.size);
    return std::move(dr);
}

int FileSystem::write_directory(INode& inode,Directory& dr) {
    // suppose that one block will be enough, if not allocate one more block 
    // fisrt serialize
    auto i = 1;
    // TODO(lonhh) : let's set up the maximum file size for directory
    // suppose it's 10 now
    while(i < 10) {
        uint8_t* buffer = new uint8_t[BLOCK_SIZE * i];
        auto ret = dr.serialize(buffer,BLOCK_SIZE * i);
        if(ret == 0) {
            i++;
            delete []buffer;
            continue;
        }
        write(inode.inode_number,buffer,ret,0);
        delete []buffer;
        break;
    }
    return 1;
}

iid_t FileSystem::new_inode(const std::string& file_name,INode& inode) {
    // judge whether this is a directory
    if(inode.itype != inode_type::DIRECTORY) {
        LOG(WARNING) << "cannot new a inode under a non-directory inode";
        return 0;
    }
    auto n_inode = im->allocate_inode();
    if(n_inode == 0) {
        LOG(WARNING) << "cannot allocate a new inode";
        return 0;
    }

    Directory dr = read_directory(inode);
    dr.insert_entry(file_name,n_inode);
    write_directory(inode,dr);
    // TODO(lonhh) : we just update the inode, but this should be optimzied
    im->read_inode(inode.inode_number,inode.data);
    return n_inode;
}

int FileSystem::truncate(iid_t id, uint32_t size) {
    INode inode;
    im->read_inode(id,inode.data);

    // extend
    if(size > inode.size) {
        uint8_t* buffer = new uint8_t[size - inode.size];
        std::memset(buffer,0,size-inode.size);
        auto ret = write(id,buffer,size-inode.size,inode.size);
        delete []buffer;
        return ret;
    }
    uint32_t nr_free_blocks = inode.block - IDIV_BLOCK_SIZE(size) - 1;
    if(MOD_BLOCK_SIZE(size) == 0) {
        nr_free_blocks++;
    }
    for(auto i=0;i<nr_free_blocks;i++) {
        delete_dblock(inode);
    }
    inode.size = size;
    im->write_inode(id,inode.data);
    return 1;
}

// return 0 if needs to free
// return 1 ow
int FileSystem::unlink(iid_t id) {
    INode inode;
    im->read_inode(id,inode.data);

    inode.links--;
    if(inode.links == 0) {
        truncate(id,0);
        im->free_inode(id);
        return 0;
    } else {
        im->write_inode(id,inode.data);
        return 1;
    }
}