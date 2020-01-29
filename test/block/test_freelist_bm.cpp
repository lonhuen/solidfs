
#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "block/super_block.h"
#include "block/block.h"

class BlockTest : public testing::Test {

protected:
    static Storage* p_storage;
    static FreeListBlockManager* fbm;
    static bid_t nr_block;
    static bid_t s_inode;
    static bid_t nr_inode;
    static bid_t s_dblock;
    static bid_t nr_dblock;

public:
    static void SetUpTestCase() {
        LogUtils::log_level="3";
        LogUtils::init("test");
        /* disk layout
         * 0 super_block
         * 1 - 9 inode
         * 10 -1299 data block
         */
        super_block sblock;
        sblock.nr_block = nr_block;
        sblock.s_inode = s_inode;
        sblock.nr_inode = nr_inode;
        sblock.s_dblock = s_dblock;
        sblock.nr_dblock= nr_dblock;

        p_storage->write_block(0,(uint8_t*)(&sblock));

    }
 
    static void TearDownTestCase() {
        delete p_storage;
    }
};

bid_t BlockTest:: nr_block = 1300;
bid_t BlockTest:: s_inode = 1;
bid_t BlockTest:: nr_inode = 9;
bid_t BlockTest:: s_dblock = 10;
bid_t BlockTest:: nr_dblock = 1290;
Storage* BlockTest:: p_storage = (Storage*)new MemoryStorage(nr_block);
FreeListBlockManager* BlockTest:: fbm = new FreeListBlockManager(p_storage);

TEST_F(BlockTest,InitTest) {
    super_block sb;
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(sb.nr_block , nr_block);
    EXPECT_EQ(sb.s_inode, s_inode);
    EXPECT_EQ(sb.nr_inode, nr_inode);
    EXPECT_EQ(sb.s_dblock, s_dblock);
    EXPECT_EQ(sb.nr_dblock, nr_dblock);
}

TEST_F(BlockTest,MkfsTest) {
    fbm->mkfs();
    Block db;
    bid_t delta = BLOCK_SIZE/sizeof(bid_t);
    for(bid_t i=s_dblock;i<=nr_dblock;i+=BLOCK_SIZE/sizeof(bid_t)){
        p_storage->read_block(i,db.data);
        if(i + delta < nr_block)
            EXPECT_EQ(db.fl_entry[0],i+delta);
        else
            EXPECT_EQ(db.fl_entry[0],0);
        for(int j=1;j<delta;j++){
            if(i + j < nr_block)
                EXPECT_EQ(db.fl_entry[j],i+j);
            else
                EXPECT_EQ(db.fl_entry[0],0);
        }
    }
}

TEST_F(BlockTest,AllocateDBlockTest) {
    Block db;
    bid_t delta = BLOCK_SIZE/sizeof(bid_t);
    for(bid_t i=0;i<delta-1;i++) {
        EXPECT_EQ(i+s_dblock+1,fbm->allocate_dblock());
    }
    EXPECT_EQ(s_dblock,fbm->allocate_dblock());
    
    for(bid_t i=0;i<delta-1;i++) {
        EXPECT_EQ(i+s_dblock+1+delta,fbm->allocate_dblock());
    }
    EXPECT_EQ(s_dblock+delta,fbm->allocate_dblock());

    for(bid_t i=0;i<delta-1;i++) {
        if(i+s_dblock+1+delta+delta > nr_block)
            EXPECT_EQ(0,fbm->allocate_dblock());
        else if(i+s_dblock+1+delta+delta == nr_block)
            EXPECT_EQ(delta*2 + s_dblock,fbm->allocate_dblock());
        else
            EXPECT_EQ(i+s_dblock+1+delta+delta,fbm->allocate_dblock());
    }
    if(s_dblock+3*delta >= nr_block)
        EXPECT_EQ(0,fbm->allocate_dblock());
    super_block sb;
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(0,sb.h_dblock);
}

// let's assume that no double free problems now
TEST_F(BlockTest,FreeDBlockTest) {
    bid_t delta = BLOCK_SIZE/sizeof(bid_t);

    // free the first 512 data blocks
    for(auto i=s_dblock;i<s_dblock+delta;i++) {
        fbm->free_dblock(i);
    }
    super_block sb;
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(s_dblock,sb.h_dblock);
    Block b;
    p_storage->read_block(s_dblock,b.data);
    for(int i=1;i<delta;i++) {
        EXPECT_EQ(i+s_dblock,b.fl_entry[i]);
    }

    // free the second 512 data blocks
    for(auto i=s_dblock+delta;i<s_dblock + delta * 2;i++) {
        fbm->free_dblock(i);
    }
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(s_dblock + delta,sb.h_dblock);
    p_storage->read_block(s_dblock+delta,b.data);
    for(int i=1;i<delta;i++) {
        if(delta+i+s_dblock < nr_block)
            EXPECT_EQ(delta+s_dblock+i,b.fl_entry[i]);
        else
            EXPECT_EQ(0,b.fl_entry[i]);
    }

    // free the third 512(not so many) data blocks
    for(auto i=s_dblock + delta*2;i<nr_block;i++) {
        fbm->free_dblock(i);
    }
    p_storage->read_block(0,sb.data);
    EXPECT_EQ(s_dblock + delta*2,sb.h_dblock);
    p_storage->read_block(s_dblock+delta*2,b.data);
    for(int i=1;i<delta;i++) {
        if(delta*2+i+s_dblock < nr_block)
            EXPECT_EQ(delta*2+s_dblock+i,b.fl_entry[i]);
        else
            EXPECT_EQ(0,b.fl_entry[i]);
    }
}

TEST_F(BlockTest,AllocateAfterFreeTest) {
    for(auto i=0;i<nr_dblock;i++)
        EXPECT_GE(fbm->allocate_dblock(),s_dblock);
    for(auto i=0;i<nr_dblock;i++)
        EXPECT_EQ(fbm->allocate_dblock(),0);
}