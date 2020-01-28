
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
    static BLOCK_ID nr_block;
    static BLOCK_ID s_inode;
    static BLOCK_ID nr_inode;
    static BLOCK_ID s_dblock;
    static BLOCK_ID nr_dblock;

public:
    static void SetUpTestCase() {
        //LogUtils::log_level="3";
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

BLOCK_ID BlockTest:: nr_block = 1300;
BLOCK_ID BlockTest:: s_inode = 1;
BLOCK_ID BlockTest:: nr_inode = 9;
BLOCK_ID BlockTest:: s_dblock = 10;
BLOCK_ID BlockTest:: nr_dblock = 1290;
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
    BLOCK_ID delta = BLOCK_SIZE/sizeof(BLOCK_ID);
    for(BLOCK_ID i=s_dblock;i<=nr_dblock;i+=BLOCK_SIZE/sizeof(BLOCK_ID)){
        p_storage->read_block(i,db.data);
        if(i + delta < nr_block)
            EXPECT_EQ(db.fl_entry[0],i+delta);
        else
            EXPECT_EQ(db.fl_entry[0],0);
    }
}