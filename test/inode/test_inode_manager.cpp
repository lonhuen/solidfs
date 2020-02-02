
#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "storage/memory_storage.h"
#include "inode/inode_manager.h"
#include "block/super_block.h"
#include "block/block.h"

class INodeTest : public testing::Test {

protected:
    static Storage* p_storage;
    static INodeManager* im;
    static bid_t nr_block;
    static bid_t s_iblock;
    static bid_t nr_iblock;
    static bid_t s_dblock;
    static bid_t nr_dblock;

public:
    static void SetUpTestCase() {
        /* disk layout
         * 0 super_block
         * 1 - 9 inode
         * 10 -1299 data block
         */
        super_block sblock;
        sblock.nr_block = nr_block;
        sblock.s_iblock = s_iblock;
        sblock.nr_iblock = nr_iblock;
        sblock.s_dblock = s_dblock;
        sblock.nr_dblock= nr_dblock;

        p_storage->write_block(0,(uint8_t*)(&sblock));

    }
 
    static void TearDownTestCase() {
        delete p_storage;
    }
};

bid_t INodeTest:: nr_block = 1300;
bid_t INodeTest:: s_iblock = 1;
bid_t INodeTest:: nr_iblock = 9;
bid_t INodeTest:: s_dblock = 10;
bid_t INodeTest:: nr_dblock = 1290;
Storage* INodeTest:: p_storage = (Storage*)new MemoryStorage(nr_block);
INodeManager* INodeTest:: im = new INodeManager(p_storage);

TEST_F(INodeTest,InitTest) {
    super_block sb;
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(sb.nr_block , nr_block);
    EXPECT_EQ(sb.s_iblock, s_iblock);
    EXPECT_EQ(sb.nr_iblock, nr_iblock);
    EXPECT_EQ(sb.s_dblock, s_dblock);
    EXPECT_EQ(sb.nr_dblock, nr_dblock);
}

TEST_F(INodeTest,MkfsTest) {
    im->mkfs();
    bid_t delta = BLOCK_SIZE/sizeof(bid_t);
    for(bid_t i=s_iblock;i<=nr_iblock + s_iblock;i++){
        Block bl;
        p_storage->read_block(i,bl.data);
        for(auto j=0;j<INodeManager::nr_iblock_PER_BLOCK;j++){
            if(i==s_iblock && j==0)
                EXPECT_EQ(bl.inode[j].itype,inode_type::DIRECTORY);
            else
                EXPECT_EQ(bl.inode[j].itype,inode_type::FREE);
        }
    }
}

TEST_F(INodeTest,AllocateINodeTest) {
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->allocate_inode(),i);
        INode inode;
        inode.itype = inode_type::DIRECTORY;
        im->write_inode(i,inode.data);
    }
}

TEST_F(INodeTest,FreeINodeTest) {
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->free_inode(i),1);
    }
}

TEST_F(INodeTest,AllocateAfterFreeTest) {
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->allocate_inode(),i);
        INode inode;
        inode.itype = inode_type::DIRECTORY;
        im->write_inode(i,inode.data);
    }
}


TEST_F(INodeTest,DoubleFreeTest) {
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->free_inode(i),1) << i;
    }
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->free_inode(i),0);
    }
    for(auto i=1;i<nr_iblock*INodeManager::nr_iblock_PER_BLOCK;i++){
        EXPECT_EQ(im->allocate_inode(),i);
        INode inode;
        inode.itype = inode_type::DIRECTORY;
        im->write_inode(i,inode.data);
    }
    im->free_inode(10);
    EXPECT_EQ(im->allocate_inode(),10);
}