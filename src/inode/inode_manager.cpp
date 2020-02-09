#include "common.h"
#include "inode/inode_manager.h"
#include "inode/inode.h"
#include "block/super_block.h"
#include "utils/log_utils.h"
#include "block/block.h"
#include "utils/fs_exception.h"

namespace solid {
    inline INodeID conv_iID_bID(INodeID id, INodeID s_iblock) {
        INodeID nr_iblock_block = config::block_size/sizeof(INode);
        return (id / nr_iblock_block) + s_iblock;
    }

    inline INodeID conv_iID_offset(INodeID id) {
        INodeID nr_iblock_block = config::block_size/sizeof(INode);
        return (id % nr_iblock_block);
    }

    void INodeManager:: mkfs() {
        super_block sb;
        storage->read_block(0,sb.data);
        this->s_iblock = sb.s_iblock;
        this->nr_iblock = sb.nr_iblock;
        
        Block bl;
        storage->read_block(s_iblock,bl.data);

        for(auto j=0;j<nr_inode_per_block;j++) {
            bl.inode[j].itype = INodeType::FREE;
        }
        storage->write_block(s_iblock,bl.data);


        for(auto i=s_iblock+1;i<s_iblock+nr_iblock;i++) {
            storage->read_block(i,bl.data);
            for(auto j=0;j<nr_inode_per_block;j++) {
                bl.inode[j].itype = INodeType::FREE;
            }
            storage->write_block(i,bl.data);
        }
    }

    INodeManager::INodeManager(Storage* p_storage) {
        this->storage = p_storage;
    }

    INode INodeManager::read_inode(INodeID id) {
        LOG(INFO) << "@read_inode " << id;
        if(id >= nr_iblock * nr_inode_per_block){
            throw fs_error("read_inode ",id, " out of range");
        }
        Block bl;
        storage->read_block(conv_iID_bID(id,s_iblock),bl.data);
        INode inode;
        memcpy(inode.data,&bl.inode[conv_iID_offset(id)],sizeof(INode));
        return inode;
    }

    void INodeManager::write_inode(INodeID id, const INode& src) {
        LOG(INFO) << "@write_inode " << id;
        if(id >= nr_iblock * nr_inode_per_block){
            throw fs_error("write_inode ",id, " out of range");
        }
        Block bl;
        storage->read_block(conv_iID_bID(id,s_iblock),bl.data);
        memcpy(&bl.inode[conv_iID_offset(id)],src.data,sizeof(INode));
        storage->write_block(conv_iID_bID(id,s_iblock),bl.data);
    }

    INodeID INodeManager::allocate_inode() {
        LOG(INFO) << "@allocate_inode";
        Block bl;
        for(BlockID i=s_iblock;i < s_iblock + nr_iblock; i++) {
            storage->read_block(i,bl.data);
            for(auto j=0;j<nr_inode_per_block;j++) {
                if(bl.inode[j].itype==INodeType::FREE) {
                    return (i - s_iblock) * nr_inode_per_block+ j;
                }
            }
        }
        throw fs_exception(
            std::errc::no_space_on_device,
            "@allocate_inode: no free inode");
    }

    void INodeManager::free_inode(INodeID id) {
        LOG(INFO) << "@free_inode " << id;
        Block bl;
        storage->read_block(conv_iID_bID(id,s_iblock),bl.data);
        if(bl.inode[conv_iID_offset(id)].itype != INodeType::FREE) {
            bl.inode[conv_iID_offset(id)].itype = INodeType::FREE;
            storage->write_block(conv_iID_bID(id,s_iblock),bl.data);
            return;
        }
        // TODO(lonhh): this might be an error or not
        // throw fs_error("@free_inode: double free inode ",id);
    }
};