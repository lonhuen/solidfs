#include "common.h"
#include "inode/inode_manager.h"
#include "inode/inode.h"
#include "block/super_block.h"
#include "utils/log_utils.h"
#include "block/block.h"

//TODO(lonhh): if single-threaded, then we don't need to maintain a "Allocated" state after allocating

inline iid_t iid2bid(iid_t id, iid_t s_iblock) {
    iid_t nr_iblock_block = BLOCK_SIZE/sizeof(INode);
    return (id / nr_iblock_block) + s_iblock;
}

inline iid_t iid2offset(iid_t id) {
    iid_t nr_iblock_block = BLOCK_SIZE/sizeof(INode);
    return (id % nr_iblock_block);
}

void INodeManager:: mkfs() {
    super_block sb;
    p_storage->read_block(0,sb.data);
    this->s_iblock = sb.s_iblock;
    this->nr_iblock = sb.nr_iblock;
    
    // initialize with root
    Block bl;
    p_storage->read_block(s_iblock,bl.data);
    bl.inode[0].itype = inode_type::DIRECTORY;
    for(auto j=1;j<nr_iblock_PER_BLOCK;j++) {
        bl.inode[j].itype = inode_type::FREE;
    }
    p_storage->write_block(s_iblock,bl.data);


    for(auto i=s_iblock+1;i<s_iblock+nr_iblock;i++) {
        p_storage->read_block(i,bl.data);
        for(auto j=0;j<nr_iblock_PER_BLOCK;j++) {
            bl.inode[j].itype = inode_type::FREE;
        }
        p_storage->write_block(i,bl.data);
    }
}

INodeManager::INodeManager(Storage* p_storage) {
    this->p_storage = p_storage;
}

int INodeManager::read_inode(iid_t id, uint8_t *dst) {
    if(id >= nr_iblock * nr_iblock_PER_BLOCK){
        LOG(WARNING) << "reading out of range in read_inode";
        return 0;
    }
    Block bl;
    p_storage->read_block(iid2bid(id,s_iblock),bl.data);
    memcpy(dst,&bl.inode[iid2offset(id)],sizeof(INode));
    return 1;
}

int INodeManager::write_inode(iid_t id, const uint8_t *dst) {
    if(id >= nr_iblock * nr_iblock_PER_BLOCK){
        LOG(WARNING) << "writing out of range in write_inode";
        return 0;
    }
    Block bl;
    p_storage->read_block(iid2bid(id,s_iblock),bl.data);
    memcpy(&bl.inode[iid2offset(id)],dst,sizeof(INode));
    p_storage->write_block(iid2bid(id,s_iblock),bl.data);
    return 1;
}

iid_t INodeManager::allocate_inode() {
    Block bl;
    for(bid_t i=s_iblock;i < s_iblock + nr_iblock; i++) {
        p_storage->read_block(i,bl.data);
        for(auto j=0;j<nr_iblock_PER_BLOCK;j++) {
            if(bl.inode[j].itype==inode_type::FREE) {
                /* if not multi-threaded
                bl.inode[j].itype = inode_type::ALLOCATED;
                p_storage->write_block(i,bl.data);
                */
                return (i - s_iblock) * nr_iblock_PER_BLOCK + j;
            }
        }
    }
    LOG(WARNING) << "no free inode when allocate_inode";
    return 0;
}

int INodeManager::free_inode(iid_t id) {
    Block bl;
    p_storage->read_block(iid2bid(id,s_iblock),bl.data);
    if(bl.inode[iid2offset(id)].itype != inode_type::FREE) {
        bl.inode[iid2offset(id)].itype = inode_type::FREE;
        p_storage->write_block(iid2bid(id,s_iblock),bl.data);
        return 1;
    }
    LOG(WARNING) << "Trying to free a free inode";
    return 0;
}