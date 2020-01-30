#include "common.h"
#include "inode/inode_manager.h"
#include "inode/inode.h"
#include "block/super_block.h"
#include "utils/log_utils.h"
#include "block/block.h"

inline iid_t iid2bid(iid_t id, iid_t s_inode) {
    iid_t nr_inode_block = BLOCK_SIZE/sizeof(INode);
    return (id / nr_inode_block) + s_inode;
}

inline iid_t iid2offset(iid_t id) {
    iid_t nr_inode_block = BLOCK_SIZE/sizeof(INode);
    return (id % nr_inode_block);
}

void INodeManager:: mkfs() {
    super_block sb;
    p_storage->read_block(0,sb.data);
    this->s_inode = sb.s_inode;
    this->nr_inode = sb.nr_inode;

    for(auto i=s_inode;i<s_inode+nr_inode;i++) {
        Block bl;
        p_storage->read_block(i,bl.data);
        for(auto j=0;j<NR_INODE_PER_BLOCK;j++) {
            bl.inode[j].itype = inode_type::FREE;
        }
        p_storage->write_block(i,bl.data);
    }
}

INodeManager::INodeManager(Storage* p_storage) {
    this->p_storage = p_storage;
}

int INodeManager::read_inode(iid_t id, uint8_t *dst) {
    if(id < s_inode || id > s_inode + nr_inode){
        LOG(ERROR) << "reading out of range in read_inode";
        return 0;
    }
    Block bl;
    p_storage->read_block(iid2bid(id,s_inode),bl.data);
    memcpy(dst,&bl.inode[iid2offset(id)],sizeof(INode));
    return 1;
}

int INodeManager::write_inode(iid_t id, const uint8_t *dst) {
    if(id < s_inode || id > s_inode + nr_inode){
        LOG(ERROR) << "reading out of range in write_inode";
        return 0;
    }
    Block bl;
    p_storage->read_block(iid2bid(id,s_inode),bl.data);
    memcpy(&bl.inode[iid2offset(id)],dst,sizeof(INode));
    p_storage->write_block(iid2bid(id,s_inode),bl.data);
    return 1;
}

iid_t INodeManager::allocate_inode() {
    Block bl;
    for(bid_t i=s_inode;i < s_inode + nr_inode; i++) {
        p_storage->read_block(i,bl.data);
        for(auto j=0;j<NR_INODE_PER_BLOCK;j++) {
            if(bl.inode[j].itype==inode_type::FREE) {
                bl.inode[j].itype = inode_type::ALLOCATED;
                p_storage->write_block(i,bl.data);
                return (i - s_inode) * NR_INODE_PER_BLOCK + j;
            }
        }
    }
    LOG(ERROR) << "no free inode when allocate_inode";
    return 0;
}

int INodeManager::free_inode(iid_t id) {
    Block bl;
    p_storage->read_block(iid2bid(id,s_inode),bl.data);
    if(bl.inode[iid2offset(id)].itype != inode_type::FREE) {
        bl.inode[iid2offset(id)].itype = inode_type::FREE;
        p_storage->write_block(iid2bid(id,s_inode),bl.data);
        return 1;
    }
    LOG(ERROR) << "Trying to free a free inode";
    return 0;
}